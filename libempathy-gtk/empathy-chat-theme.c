/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * Copyright (C) 2010 Thomas Meire <blackskad@gmail.com>
 *
 * The code contained in this file is free software; you can redistribute
 * it and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either version
 * 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this code; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <glib.h>
#include <glib-object.h>

#include <libempathy/empathy-utils.h>

#include "empathy-chat-theme.h"
#include "empathy-chat-view.h"

#define GET_PRIV(o) EMPATHY_GET_PRIV (o, EmpathyChatTheme)

G_DEFINE_ABSTRACT_TYPE (EmpathyChatTheme, empathy_chat_theme, G_TYPE_OBJECT);

typedef struct _EmpathyChatThemePriv EmpathyChatThemePriv;

struct _EmpathyChatThemePriv
{
  gchar *name;
};

typedef enum {
  PROP_0,
  PROP_THEME_NAME
} EmpathyChatThemeProperty;

EmpathyChatView *
empathy_chat_theme_create_view (EmpathyChatTheme *self)
{
  g_return_val_if_fail (EMPATHY_IS_CHAT_THEME (self), NULL);

  if (EMPATHY_CHAT_THEME_GET_CLASS(self)->create_view)
    {
      return EMPATHY_CHAT_THEME_GET_CLASS(self)->create_view(self);
    }
  return NULL;
}

gchar *
empathy_chat_theme_get_name (EmpathyChatTheme *theme)
{
  EmpathyChatThemePriv *priv = GET_PRIV (theme);
  return g_strdup (priv->name);
}

static void
empathy_chat_theme_get_property(GObject *object,
    guint param_id,
    GValue *value,
    GParamSpec *pspec)
{
  switch (param_id)
    {
      case PROP_THEME_NAME:
        /* FIXME: return the name */
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
        break;
    }
}

static void
empathy_chat_theme_set_property (GObject *object,
    guint param_id,
    const GValue *value,
    GParamSpec *pspec)
{
  EmpathyChatThemePriv *priv = GET_PRIV (object);

  switch (param_id)
    {
      case PROP_THEME_NAME:
        g_assert (priv->name == NULL);
        priv->name = g_strdup (g_value_get_string (value)); 
        break;
    }
}

static void
empathy_chat_theme_init (EmpathyChatTheme *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, EMPATHY_TYPE_CHAT_THEME,
      EmpathyChatThemePriv);
}

static void
empathy_chat_theme_class_init (EmpathyChatThemeClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->set_property = empathy_chat_theme_set_property;
  object_class->get_property = empathy_chat_theme_get_property;

	g_object_class_install_property (object_class, PROP_THEME_NAME,
      g_param_spec_string ("theme-name",
          "The theme name",
          "The name of the theme",
          NULL,
          G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_type_class_add_private (object_class, sizeof (EmpathyChatThemePriv));
}

