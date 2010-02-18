/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * Copyright (C) 2005-2007 Imendio AB
 * Copyright (C) 2008 Collabora Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 *
 * Authors: Xavier Claessens <xclaesse@gmail.com>
 *          Thomas Meire <blackskad@gmail.com>
 */

#include "config.h"

#include <gtk/gtk.h>
#include <telepathy-glib/util.h>
#include <libempathy/empathy-utils.h>
#include <libempathy-gtk/empathy-conf.h>

#include "empathy-theme-manager.h"

#include "empathy-chat-view.h"
#include "empathy-chat-theme.h"
#include "empathy-classic-chat-theme.h"
#include "empathy-boxed-chat-theme.h"

#ifdef HAVE_WEBKIT
#include "empathy-adium-chat-theme.h"
#endif

#define GET_PRIV(obj) EMPATHY_GET_PRIV (obj, EmpathyThemeManager)

typedef struct {
  EmpathyChatTheme *selected;
} EmpathyThemeManagerPriv;

enum {
	SELECTION_CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

typedef GList* (*EmpathyThemeProvider)(void);

EmpathyThemeProvider providers[4] = {
    empathy_classic_chat_theme_discover,
    empathy_boxed_chat_theme_discover,
#ifdef HAVE_WEBKIT
    empathy_adium_chat_theme_discover,
#endif
    NULL
};

G_DEFINE_TYPE (EmpathyThemeManager, empathy_theme_manager, GTK_TYPE_LIST_STORE);

EmpathyThemeManager *
empathy_theme_manager_get (void)
{
	static EmpathyThemeManager *manager = NULL;

  if (!manager)
    {
      manager = g_object_new (EMPATHY_TYPE_THEME_MANAGER, NULL);
    }

  return manager;
}

void
empathy_theme_manager_install (gchar *path)
{
#ifdef HAVE_WEBKIT
  g_message ("Installing theme from %s", path);
  empathy_adium_chat_theme_install (path);
#else
  g_message ("Can't install new themes. Empathy was compiled without webkit support!");
#endif
}

static void
empathy_theme_manager_selected_variant_changed (EmpathyChatTheme *theme,
    gpointer user_data)
{
  gchar *variant = empathy_chat_theme_get_selected_variant (theme);
  g_message ("SAVING VARIANT TO GCONF");
  empathy_conf_set_string (empathy_conf_get (), EMPATHY_PREFS_CHAT_THEME_VARIANT, variant);
  g_free (variant);
}

void
empathy_theme_manager_select (EmpathyThemeManager *self,
    EmpathyChatTheme *theme)
{
  static gulong id = -1;

  gchar *name, *variant;
  EmpathyThemeManagerPriv *priv = GET_PRIV (self);

  g_assert (EMPATHY_IS_CHAT_THEME (theme));

  if (priv->selected)
  {
    g_signal_handler_disconnect (priv->selected, id);
    g_object_unref (G_OBJECT (priv->selected));
  }

  g_object_ref (G_OBJECT (theme));
  priv->selected = theme;

  /* update gconf data */
  name = empathy_chat_theme_get_name (theme);
  variant = empathy_chat_theme_get_selected_variant (theme);
  empathy_conf_set_string (empathy_conf_get (), EMPATHY_PREFS_CHAT_THEME, name);
  empathy_conf_set_string (empathy_conf_get (), EMPATHY_PREFS_CHAT_THEME_VARIANT, variant);
  g_free (name);
  g_free (variant);

  /* Connect to the theme to track changes to the selected variant. */
  id = g_signal_connect (theme, "variant-changed",
      G_CALLBACK (empathy_theme_manager_selected_variant_changed), NULL);
}

EmpathyChatTheme *
empathy_theme_manager_get_selection (EmpathyThemeManager *self)
{
  EmpathyThemeManagerPriv *priv = GET_PRIV (self);
  return priv->selected;
}

EmpathyChatView *
empathy_theme_manager_create_view (EmpathyThemeManager *self)
{
  EmpathyThemeManagerPriv *priv = GET_PRIV (self);

	g_return_val_if_fail (EMPATHY_IS_THEME_MANAGER (self), NULL);

  if (!priv->selected)
    {
      EmpathyChatTheme *theme;
      GtkTreeIter iter;

      if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (self), &iter))
        {
          return NULL;
        }
      gtk_tree_model_get (GTK_TREE_MODEL (self), &iter,
          EMPATHY_THEME_MANAGER_THEME, &theme, -1);
      empathy_theme_manager_select (self, theme);
      g_object_unref (theme);
    }
  return empathy_chat_theme_create_view (priv->selected);
}

static void
empathy_theme_manager_add_theme (EmpathyThemeManager *self,
    EmpathyChatTheme *theme)
{
  GtkTreeIter iter;
  
  /* FIXME: check if theme is already present? */
  gtk_list_store_append (GTK_LIST_STORE (self), &iter);
  gtk_list_store_set (GTK_LIST_STORE (self), &iter,
      EMPATHY_THEME_MANAGER_NAME, empathy_chat_theme_get_name (theme),
      EMPATHY_THEME_MANAGER_THUMBNAIL, empathy_chat_theme_get_thumbnail (theme),
      EMPATHY_THEME_MANAGER_THEME, theme,
      -1);
}

static void
theme_manager_finalize (GObject *object)
{
#if 0
  EmpathyThemeManagerPriv *priv = GET_PRIV (object);

  empathy_conf_notify_remove (empathy_conf_get (), priv->name_notify_id);
  g_free (priv->name);
  empathy_conf_notify_remove (empathy_conf_get (), priv->adium_path_notify_id);
  g_free (priv->adium_path);
#endif

  G_OBJECT_CLASS (empathy_theme_manager_parent_class)->finalize (object);
}

static void
empathy_theme_manager_class_init (EmpathyThemeManagerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

  signals[SELECTION_CHANGED] = g_signal_new ("selection-changed",
      G_OBJECT_CLASS_TYPE (object_class),
      G_SIGNAL_RUN_LAST,
      0,
      NULL, NULL,
      g_cclosure_marshal_VOID__VOID,
      G_TYPE_NONE,
      0);

  g_type_class_add_private (object_class, sizeof (EmpathyThemeManagerPriv));

  object_class->finalize = theme_manager_finalize;
}

static void
empathy_theme_manager_init (EmpathyThemeManager *self)
{
  int j;
  GList *i;
  GList *themes;
  gchar *conf_name, *conf_variant;
  EmpathyThemeManagerPriv *priv;

  /* setup the columns */
  GType types[] = {
    G_TYPE_STRING,            /* Display name */
    GDK_TYPE_PIXBUF,          /* Thumbnail */
    EMPATHY_TYPE_CHAT_THEME   /* The chat theme */
  };

  gtk_list_store_set_column_types (GTK_LIST_STORE (self),
    EMPATHY_THEME_MANAGER_COUNT, types);

  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (self),
    EMPATHY_THEME_MANAGER_NAME, GTK_SORT_ASCENDING);

  /* add priv struct */
  priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
      EMPATHY_TYPE_THEME_MANAGER, EmpathyThemeManagerPriv);
  self->priv = priv;

  /* discover all themes */
  empathy_conf_get_string (empathy_conf_get (), EMPATHY_PREFS_CHAT_THEME, &conf_name);
  empathy_conf_get_string (empathy_conf_get (), EMPATHY_PREFS_CHAT_THEME_VARIANT, &conf_variant);
  for (j = 0; providers[j]; j++)
    {
      themes = providers[j]();
      for (i = themes; i; i=i->next)
        {
          gchar *name = empathy_chat_theme_get_name (i->data);
          empathy_theme_manager_add_theme (self, i->data);
          if (g_strcmp0 (name, conf_name) == 0)
            {
              empathy_chat_theme_set_selected_variant (i->data, conf_variant);
              empathy_theme_manager_select (self, i->data);
            }
          g_free (name);
        }
    }
}

