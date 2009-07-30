/*
 * empathy-account-settings.c - Source for EmpathyAccountSettings
 * Copyright (C) 2009 Collabora Ltd.
 * @author Sjoerd Simons <sjoerd.simons@collabora.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <stdio.h>
#include <stdlib.h>

#include <telepathy-glib/util.h>

#include "empathy-account-settings.h"
#include "empathy-account-manager.h"
#include "empathy-connection-managers.h"
#include "empathy-utils.h"

#define GET_PRIV(obj) EMPATHY_GET_PRIV (obj, EmpathyAccountSettings)

G_DEFINE_TYPE(EmpathyAccountSettings, empathy_account_settings, G_TYPE_OBJECT)

/* signal enum */
#if 0
enum
{
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};
#endif

enum {
  PROP_ACCOUNT = 1,
  PROP_CM_NAME,
  PROP_PROTOCOL,
  PROP_DISPLAY_NAME,
  PROP_READY
};

/* private structure */
typedef struct _EmpathyAccountSettingsPriv EmpathyAccountSettingsPriv;

struct _EmpathyAccountSettingsPriv
{
  gboolean dispose_has_run;
  EmpathyConnectionManagers *managers;
  EmpathyAccountManager *account_manager;
  gulong account_manager_ready_id;

  TpConnectionManager *manager;
  const TpConnectionManagerProtocol *tp_protocol;

  EmpathyAccount *account;
  gchar *cm_name;
  gchar *protocol;
  gchar *display_name;
  gboolean ready;

  GHashTable *parameters;
  GArray *unset_parameters;

  gulong managers_ready_id;
  gulong account_ready_id;

  GSimpleAsyncResult *apply_result;
};

static void
empathy_account_settings_init (EmpathyAccountSettings *obj)
{
  EmpathyAccountSettingsPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE ((obj),
    EMPATHY_TYPE_ACCOUNT_SETTINGS, EmpathyAccountSettingsPriv);

  obj->priv = priv;

  /* allocate any data required by the object here */
  priv->managers = empathy_connection_managers_dup_singleton ();
  priv->account_manager = empathy_account_manager_dup_singleton ();

  priv->parameters = tp_asv_new (NULL, NULL);
  priv->unset_parameters = g_array_new (TRUE, FALSE, sizeof (gchar *));
}

static void empathy_account_settings_dispose (GObject *object);
static void empathy_account_settings_finalize (GObject *object);
static void empathy_account_settings_ready_cb (GObject *obj,
  GParamSpec *spec, gpointer user_data);
static void empathy_account_settings_check_readyness (
    EmpathyAccountSettings *self);

static void
empathy_account_settings_set_property (GObject *object,
    guint prop_id,
    const GValue *value,
    GParamSpec *pspec)
{
  EmpathyAccountSettings *settings = EMPATHY_ACCOUNT_SETTINGS (object);
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);

  switch (prop_id)
    {
      case PROP_ACCOUNT:
        priv->account = g_value_dup_object (value);
        break;
      case PROP_CM_NAME:
        priv->cm_name = g_value_dup_string (value);
        break;
      case PROP_PROTOCOL:
        priv->protocol = g_value_dup_string (value);
        break;
      case PROP_DISPLAY_NAME:
        priv->display_name = g_value_dup_string (value);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
empathy_account_settings_get_property (GObject *object,
    guint prop_id,
    GValue *value,
    GParamSpec *pspec)
{
  EmpathyAccountSettings *settings = EMPATHY_ACCOUNT_SETTINGS (object);
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);

  switch (prop_id)
    {
      case PROP_ACCOUNT:
        g_value_set_object (value, priv->account);
        break;
      case PROP_CM_NAME:
        g_value_set_string (value, priv->cm_name);
        break;
      case PROP_PROTOCOL:
        g_value_set_string (value, priv->protocol);
        break;
      case PROP_DISPLAY_NAME:
        g_value_set_string (value, priv->display_name);
        break;
      case PROP_READY:
        g_value_set_boolean (value, priv->ready);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
empathy_account_settings_constructed (GObject *object)
{
  EmpathyAccountSettings *self = EMPATHY_ACCOUNT_SETTINGS (object);
  EmpathyAccountSettingsPriv *priv = GET_PRIV (self);

  if (priv->account != NULL)
    {
      g_free (priv->cm_name);
      g_free (priv->protocol);
      g_free (priv->display_name);

      priv->cm_name =
        g_strdup (empathy_account_get_connection_manager (priv->account));
      priv->protocol =
        g_strdup (empathy_account_get_protocol (priv->account));
      priv->display_name =
        g_strdup (empathy_account_get_display_name (priv->account));
    }

  g_assert (priv->cm_name != NULL && priv->protocol != NULL
    && priv->display_name != NULL);

  empathy_account_settings_check_readyness (self);

  if (!priv->ready)
    {
      g_signal_connect (priv->account, "notify::ready",
        G_CALLBACK (empathy_account_settings_ready_cb), self);
      g_signal_connect (priv->managers, "notify::ready",
        G_CALLBACK (empathy_account_settings_ready_cb), self);
    }

  if (G_OBJECT_CLASS (
        empathy_account_settings_parent_class)->constructed != NULL)
    G_OBJECT_CLASS (
        empathy_account_settings_parent_class)->constructed (object);
}


static void
empathy_account_settings_class_init (
    EmpathyAccountSettingsClass *empathy_account_settings_class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (empathy_account_settings_class);

  g_type_class_add_private (empathy_account_settings_class, sizeof
      (EmpathyAccountSettingsPriv));

  object_class->dispose = empathy_account_settings_dispose;
  object_class->finalize = empathy_account_settings_finalize;
  object_class->set_property = empathy_account_settings_set_property;
  object_class->get_property = empathy_account_settings_get_property;
  object_class->constructed = empathy_account_settings_constructed;

  g_object_class_install_property (object_class, PROP_ACCOUNT,
    g_param_spec_object ("account",
      "Account",
      "The EmpathyAccount backing these settings",
      EMPATHY_TYPE_ACCOUNT,
      G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_CM_NAME,
    g_param_spec_string ("connection-manager",
      "connection-manager",
      "The name of the connection manager this account uses",
      NULL,
      G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_PROTOCOL,
    g_param_spec_string ("protocol",
      "Protocol",
      "The name of the protocol this account uses",
      NULL,
      G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_DISPLAY_NAME,
    g_param_spec_string ("display-name",
      "display-name",
      "The display name account these settings belong to",
      NULL,
      G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_READY,
    g_param_spec_boolean ("ready",
      "Ready",
      "Whether this account is ready to be used",
      FALSE,
      G_PARAM_STATIC_STRINGS | G_PARAM_READABLE));
}

static void
empathy_account_settings_dispose (GObject *object)
{
  EmpathyAccountSettings *self = EMPATHY_ACCOUNT_SETTINGS (object);
  EmpathyAccountSettingsPriv *priv = GET_PRIV (self);

  if (priv->dispose_has_run)
    return;

  priv->dispose_has_run = TRUE;

  if (priv->managers_ready_id != 0)
    g_signal_handler_disconnect (priv->managers, priv->managers_ready_id);
  priv->managers_ready_id = 0;

  if (priv->managers != NULL)
    g_object_unref (priv->managers);
  priv->managers = NULL;

  if (priv->manager != NULL)
    g_object_unref (priv->manager);
  priv->manager = NULL;

  if (priv->account_manager_ready_id != 0)
    g_signal_handler_disconnect (priv->account_manager,
        priv->account_manager_ready_id);
  priv->account_manager_ready_id = 0;

  if (priv->account_manager != NULL)
    g_object_unref (priv->account_manager);
  priv->account_manager = NULL;

  if (priv->account_ready_id != 0)
    g_signal_handler_disconnect (priv->account, priv->account_ready_id);
  priv->account_ready_id = 0;

  if (priv->account != NULL)
    g_object_unref (priv->account);
  priv->account = NULL;

  /* release any references held by the object here */
  if (G_OBJECT_CLASS (empathy_account_settings_parent_class)->dispose)
    G_OBJECT_CLASS (empathy_account_settings_parent_class)->dispose (object);
}

static void
empathy_account_settings_free_unset_parameters (
    EmpathyAccountSettings *settings)
{
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);
  int i;

  for (i = 0 ; i < priv->unset_parameters->len; i++)
    g_free (g_array_index (priv->unset_parameters, gchar *, i));

  g_array_set_size (priv->unset_parameters, 0);
}

static void
empathy_account_settings_finalize (GObject *object)
{
  EmpathyAccountSettings *self = EMPATHY_ACCOUNT_SETTINGS (object);
  EmpathyAccountSettingsPriv *priv = GET_PRIV (self);

  /* free any data held directly by the object here */
  g_free (priv->cm_name);
  g_free (priv->protocol);
  g_free (priv->display_name);

  g_hash_table_destroy (priv->parameters);

  empathy_account_settings_free_unset_parameters (self);
  g_array_free (priv->unset_parameters, TRUE);

  G_OBJECT_CLASS (empathy_account_settings_parent_class)->finalize (object);
}

static void
empathy_account_settings_check_readyness (EmpathyAccountSettings *self)
{
  EmpathyAccountSettingsPriv *priv = GET_PRIV (self);

  if (priv->ready)
    return;

  if (priv->account != NULL && !empathy_account_is_ready (priv->account))
      return;

  if (!empathy_connection_managers_is_ready (priv->managers))
    return;

  priv->manager = empathy_connection_managers_get_cm (
    priv->managers, priv->cm_name);

  if (priv->manager == NULL)
    return;

  priv->tp_protocol = tp_connection_manager_get_protocol (priv->manager,
    priv->protocol);

  if (priv->tp_protocol == NULL)
    {
      priv->manager = NULL;
      return;
    }

  g_object_ref (priv->manager);

  priv->ready = TRUE;
  g_object_notify (G_OBJECT (self), "ready");
}

static void
empathy_account_settings_ready_cb (GObject *obj,
    GParamSpec *spec,
    gpointer user_data)
{
  EmpathyAccountSettings *settings = EMPATHY_ACCOUNT_SETTINGS (user_data);

  empathy_account_settings_check_readyness (settings);
}

EmpathyAccountSettings *
empathy_account_settings_new (const gchar *connection_manager,
    const gchar *protocol,
    const char *display_name)
{
  return g_object_new (EMPATHY_TYPE_ACCOUNT_SETTINGS,
      "connection-manager", connection_manager,
      "protocol", protocol,
      "display-name", display_name,
      NULL);
}

EmpathyAccountSettings *
empathy_account_settings_new_for_account (EmpathyAccount *account)
{
  return g_object_new (EMPATHY_TYPE_ACCOUNT_SETTINGS,
      "account", account,
      NULL);
}

TpConnectionManagerParam *
empathy_account_settings_get_tp_params (EmpathyAccountSettings *settings)
{
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);

  g_return_val_if_fail (priv->tp_protocol != NULL, NULL);

  return priv->tp_protocol->params;
}

gboolean
empathy_account_settings_is_ready (EmpathyAccountSettings *settings)
{
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);

  return priv->ready;
}

const gchar *
empathy_account_settings_get_cm (EmpathyAccountSettings *settings)
{
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);

  return priv->cm_name;
}

const gchar *
empathy_account_settings_get_protocol (EmpathyAccountSettings *settings)
{
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);

  return priv->protocol;
}

const gchar *
empathy_account_settings_get_icon_name (EmpathyAccountSettings *settings)
{
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);

  if (priv->account != NULL)
    return empathy_account_get_icon_name (priv->account);

  return NULL;
}

const gchar *
empathy_account_settings_get_display_name (EmpathyAccountSettings *settings)
{
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);

  return priv->display_name;
}

EmpathyAccount *
empathy_account_settings_get_account (EmpathyAccountSettings *settings)
{
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);

  return priv->account;
}

static gboolean
empathy_account_settings_is_unset (EmpathyAccountSettings *settings,
    const gchar *param)
{
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);
  GArray *a;
  int i;

  a = priv->unset_parameters;

  for (i = 0; i < a->len; i++)
    {
      if (!tp_strdiff (g_array_index (a, gchar *, i), param))
        return TRUE;
    }

  return FALSE;
}

static TpConnectionManagerParam *
empathy_account_settings_get_tp_param (EmpathyAccountSettings *settings,
    const gchar *param)
{
  TpConnectionManagerParam *tp_params =
      empathy_account_settings_get_tp_params (settings);
  TpConnectionManagerParam *p;

  for (p = tp_params; p != NULL && p->name != NULL; p++)
    {
      if (tp_strdiff (p->name, param))
        continue;

      return p;
    }

  return NULL;
}

const GValue *
empathy_account_settings_get_default (EmpathyAccountSettings *settings,
    const gchar *param)
{
  TpConnectionManagerParam *p;

  p = empathy_account_settings_get_tp_param (settings, param);

  if (p == NULL || !(p->flags & TP_CONN_MGR_PARAM_FLAG_HAS_DEFAULT))
    return NULL;

  return &(p->default_value);
}

const gchar *
empathy_settings_get_dbus_signature (EmpathyAccountSettings *settings,
    const gchar *param)
{
  TpConnectionManagerParam *p;

  p = empathy_account_settings_get_tp_param (settings, param);

  if (p == NULL)
    return NULL;

  return p->dbus_signature;
}

const GValue *
empathy_account_settings_get (EmpathyAccountSettings *settings,
    const gchar *param)
{
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);
  const GValue *result = NULL;

  /* Lookup the update parameters we set */
  result = tp_asv_lookup (priv->parameters, param);
  if (result != NULL)
    return result;

  /* If the parameters isn't unset use the accounts setting if any */
  if (priv->account != NULL
      && !empathy_account_settings_is_unset (settings, param))
    {
      const GHashTable *parameters;

      parameters = empathy_account_get_parameters (priv->account);
      result = tp_asv_lookup (parameters, param);

      if (result != NULL)
        return result;
    }

  /* fallback to the default */
  return empathy_account_settings_get_default (settings, param);
}


void
empathy_account_settings_unset (EmpathyAccountSettings *settings,
    const gchar *param)
{
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);
  gchar *v;
  if (empathy_account_settings_is_unset (settings, param))
    return;

  v = g_strdup (param);

  g_array_append_val (priv->unset_parameters, v);
  g_hash_table_remove (priv->parameters, param);
}

const gchar *
empathy_account_settings_get_string (EmpathyAccountSettings *settings,
    const gchar *param)
{
  const GValue *v;

  v = empathy_account_settings_get (settings, param);

  if (v == NULL || !G_VALUE_HOLDS_STRING (v))
    return NULL;

  return g_value_get_string (v);
}

gint32
empathy_account_settings_get_int32 (EmpathyAccountSettings *settings,
    const gchar *param)
{
  const GValue *v;
  gint32 ret = 0;

  v = empathy_account_settings_get (settings, param);

  if (v == NULL)
    return 0;

  switch G_VALUE_TYPE (v)
    {
      case G_TYPE_UCHAR:
        ret = g_value_get_uchar (v);
        break;
      case G_TYPE_INT:
        ret = g_value_get_int (v);
        break;
      case G_TYPE_UINT:
        ret = CLAMP (G_MININT32, g_value_get_uint (v), G_MAXINT32);
        break;
      case G_TYPE_INT64:
        ret = CLAMP (G_MININT32, g_value_get_int64 (v), G_MAXINT32);
        break;
      case G_TYPE_UINT64:
        ret = CLAMP (G_MININT32, g_value_get_uint64 (v), G_MAXINT32);
        break;
      default:
        ret = 0;
        break;
    }

  return ret;
}

gint64
empathy_account_settings_get_int64 (EmpathyAccountSettings *settings,
    const gchar *param)
{
  const GValue *v;
  gint64 ret = 0;

  v = empathy_account_settings_get (settings, param);
  if (v == NULL)
    return 0;

  switch G_VALUE_TYPE (v)
    {
      case G_TYPE_UCHAR:
        ret = g_value_get_uchar (v);
        break;
      case G_TYPE_INT:
        ret = g_value_get_int (v);
        break;
      case G_TYPE_UINT:
        ret = g_value_get_uint (v);
        break;
      case G_TYPE_INT64:
        ret = g_value_get_int64 (v);
        break;
      case G_TYPE_UINT64:
        ret = CLAMP (G_MININT64, g_value_get_uint64 (v), G_MAXINT64);
        break;
      default:
        ret = 0;
        break;
    }

  return ret;
}

guint32
empathy_account_settings_get_uint32 (EmpathyAccountSettings *settings,
    const gchar *param)
{
  const GValue *v;
  guint32 ret;

  v = empathy_account_settings_get (settings, param);

  switch G_VALUE_TYPE (v)
    {
      case G_TYPE_UCHAR:
        ret = g_value_get_uchar (v);
        break;
      case G_TYPE_INT:
        ret = MAX (0, g_value_get_int (v));
        break;
      case G_TYPE_UINT:
        ret = g_value_get_uint (v);
        break;
      case G_TYPE_INT64:
        ret = CLAMP (0, g_value_get_int64 (v), G_MAXUINT32);
        break;
      case G_TYPE_UINT64:
        ret = CLAMP (0, g_value_get_uint64 (v), G_MAXUINT32);
        break;
      default:
        ret = 0;
        break;
    }

  return ret;
}

guint64
empathy_account_settings_get_uint64 (EmpathyAccountSettings *settings,
    const gchar *param)
{
  const GValue *v;
  guint64 ret = 0;

  v = empathy_account_settings_get (settings, param);

  if (v == NULL || !G_VALUE_HOLDS_INT (v))
    return 0;

  switch G_VALUE_TYPE (v)
    {
      case G_TYPE_UCHAR:
        ret = g_value_get_uchar (v);
        break;
      case G_TYPE_INT:
        ret = MAX (0, g_value_get_int (v));
        break;
      case G_TYPE_UINT:
        ret = g_value_get_uint (v);
        break;
      case G_TYPE_INT64:
        ret = MAX (0, g_value_get_int64 (v));
        break;
      case G_TYPE_UINT64:
        ret = CLAMP (0, g_value_get_uint64 (v), G_MAXUINT64);
        break;
      default:
        ret = 0;
        break;
    }

  return ret;
}

gboolean
empathy_account_settings_get_boolean (EmpathyAccountSettings *settings,
    const gchar *param)
{
  const GValue *v;

  v = empathy_account_settings_get (settings, param);

  if (v == NULL || !G_VALUE_HOLDS_BOOLEAN (v))
    return FALSE;

  return g_value_get_boolean (v);
}

void
empathy_account_settings_set_string (EmpathyAccountSettings *settings,
    const gchar *param,
    const gchar *value)
{
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);

  tp_asv_set_string (priv->parameters, param, value);
}

void
empathy_account_settings_set_int32 (EmpathyAccountSettings *settings,
    const gchar *param,
    gint32 value)
{
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);

  tp_asv_set_int32 (priv->parameters, param, value);
}

void
empathy_account_settings_set_int64 (EmpathyAccountSettings *settings,
    const gchar *param,
    gint64 value)
{
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);

  tp_asv_set_int64 (priv->parameters, param, value);
}

void
empathy_account_settings_set_uint32 (EmpathyAccountSettings *settings,
    const gchar *param,
    guint32 value)
{
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);

  tp_asv_set_uint32 (priv->parameters, param, value);
}

void
empathy_account_settings_set_uint64 (EmpathyAccountSettings *settings,
    const gchar *param,
    guint64 value)
{
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);

  tp_asv_set_uint64 (priv->parameters, param, value);
}

void
empathy_account_settings_set_boolean (EmpathyAccountSettings *settings,
    const gchar *param,
    gboolean value)
{
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);

  tp_asv_set_boolean (priv->parameters, param, value);
}


void
empathy_account_settings_set_display_name_async (
  EmpathyAccountSettings *settings,
  const gchar *name,
  GAsyncReadyCallback callback,
  gpointer user_data)
{
}

gboolean
empathy_account_settings_set_display_name_finish (
  EmpathyAccountSettings *settings,
  GAsyncResult *result,
  GError **error)
{

  return TRUE;
}

static void
empathy_account_settings_account_updated (GObject *source,
    GAsyncResult *result,
    gpointer user_data)
{
  EmpathyAccountSettings *settings = EMPATHY_ACCOUNT_SETTINGS (user_data);
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);
  GSimpleAsyncResult *r;
  GError *error = NULL;

  if (!empathy_account_update_settings_finish (EMPATHY_ACCOUNT (source),
    result, &error))
    {
      g_simple_async_result_set_from_error (priv->apply_result, error);
      g_error_free (error);
    }

  r = priv->apply_result;
  priv->apply_result = NULL;

  g_simple_async_result_complete (r);
  g_object_unref (r);
}

static void
empathy_account_settings_created_cb (GObject *source,
    GAsyncResult *result,
    gpointer user_data)
{
  EmpathyAccountSettings *settings = EMPATHY_ACCOUNT_SETTINGS (user_data);
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);
  EmpathyAccount *account;
  GError *error = NULL;
  GSimpleAsyncResult *r;

  account = empathy_account_manager_create_account_finish (
    EMPATHY_ACCOUNT_MANAGER (source), result, &error);

  if (account == NULL)
    {
      g_simple_async_result_set_from_error (priv->apply_result, error);
    }
  else
    {
      priv->account = g_object_ref (account);
    }

  r = priv->apply_result;
  priv->apply_result = NULL;

  g_simple_async_result_complete (r);
  g_object_unref (r);
}


static void
empathy_account_settings_do_create_account (EmpathyAccountSettings *settings)
{
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);
  GHashTable *properties;

  properties = g_hash_table_new (NULL, NULL);

  empathy_account_manager_create_account_async (priv->account_manager,
    priv->cm_name, priv->protocol, priv->display_name,
    priv->parameters, properties,
    empathy_account_settings_created_cb,
    settings);

  g_hash_table_unref (properties);
}

static void
empathy_account_settings_manager_ready_cb (EmpathyAccountManager *manager,
    GParamSpec *spec,
    gpointer user_data)
{
  EmpathyAccountSettings *settings = EMPATHY_ACCOUNT_SETTINGS (user_data);
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);

  if (empathy_account_manager_is_ready (manager))
    {
      g_assert (priv->apply_result != NULL && priv->account == NULL);
      g_signal_handler_disconnect (priv->account_manager,
        priv->account_manager_ready_id);
      priv->account_manager_ready_id = 0;

      empathy_account_settings_do_create_account (settings);
    }
}

void
empathy_account_settings_apply_async (EmpathyAccountSettings *settings,
    GAsyncReadyCallback callback,
    gpointer user_data)
{
  EmpathyAccountSettingsPriv *priv = GET_PRIV (settings);

  if (priv->apply_result != NULL)
    {
      g_simple_async_report_error_in_idle (G_OBJECT (settings),
        callback, user_data,
        G_IO_ERROR, G_IO_ERROR_PENDING, "Applying already in progress");
      return;
    }

  priv->apply_result = g_simple_async_result_new (G_OBJECT (settings),
      callback, user_data, empathy_account_settings_apply_finish);

  if (priv->account == NULL)
    {
      if (empathy_account_manager_is_ready (priv->account_manager))
        empathy_account_settings_do_create_account (settings);
      else
        priv->account_manager_ready_id = g_signal_connect (
            priv->account_manager,
            "notify::ready",
            G_CALLBACK (empathy_account_settings_manager_ready_cb),
            settings);
    }
  else
    {
      empathy_account_update_settings_async (priv->account,
        priv->parameters, (const gchar **)priv->unset_parameters->data,
        empathy_account_settings_account_updated, settings);

      g_hash_table_remove_all (priv->parameters);
      empathy_account_settings_free_unset_parameters (settings);
    }
}

gboolean
empathy_account_settings_apply_finish (EmpathyAccountSettings *settings,
    GAsyncResult *result,
    GError **error)
{
  if (g_simple_async_result_propagate_error (G_SIMPLE_ASYNC_RESULT (result),
      error))
    return FALSE;

  g_return_val_if_fail (g_simple_async_result_is_valid (result,
    G_OBJECT (settings), empathy_account_settings_apply_finish), FALSE);

  return TRUE;
}
