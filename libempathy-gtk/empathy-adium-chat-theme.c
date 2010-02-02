/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * Copyright (C) 2010 Thomas Meire <blackskad@gmail.com>
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
 */

#include "empathy-adium-chat-theme.h"

#define DEBUG_FLAG EMPATHY_DEBUG_OTHER
#include <libempathy/empathy-debug.h>
#include <libempathy/empathy-utils.h>

#include "empathy-chat-view.h"
#include "empathy-theme-adium.h"

#define GET_PRIV(obj) EMPATHY_GET_PRIV (obj, EmpathyAdiumChatTheme)

G_DEFINE_TYPE (EmpathyAdiumChatTheme, empathy_adium_chat_theme, EMPATHY_TYPE_CHAT_THEME);

typedef struct _EmpathyAdiumChatThemePriv EmpathyAdiumChatThemePriv;

struct _EmpathyAdiumChatThemePriv
{
  GHashTable *info;
  EmpathyAdiumData *data;
};

static EmpathyChatTheme *
empathy_adium_chat_theme_new (GHashTable *info)
{
  EmpathyChatTheme *theme;
  EmpathyAdiumChatThemePriv *priv;

  theme = EMPATHY_CHAT_THEME (g_object_new (EMPATHY_TYPE_ADIUM_CHAT_THEME,
      "theme-name", tp_asv_get_string (info, "CFBundleName"),
      //"theme-variants", variants,
      NULL));

  priv = GET_PRIV (theme);
  priv->info = info;

  return theme;
}

static void
empathy_adium_chat_theme_discover_in_dir (gchar *dirpath,
    GList *themes)
{
  GDir *dir;
  GError *error = NULL;
  const gchar *name = NULL;
  GHashTable *info = NULL;
  EmpathyChatTheme *theme;

  dir = g_dir_open (dirpath, 0, &error);
  if (dir != NULL)
    {
      name = g_dir_read_name (dir);
      while (name != NULL)
        {
          gchar *path = g_build_path (G_DIR_SEPARATOR_S, dirpath, name, NULL);
          if (empathy_adium_path_is_valid (path))
            {
              info = empathy_adium_info_new (path);
              if (info != NULL)
                {
                  theme = empathy_adium_chat_theme_new (info);
                  themes = g_list_prepend (themes, theme);
                }
            }
          g_free (path);
          name = g_dir_read_name (dir);
        }
        g_dir_close (dir);
    }
  else
    {
      DEBUG ("Error opening %s: %s\n", dirpath, error->message);
      g_error_free (error);
    }
}

GList *
empathy_adium_chat_theme_discover ()
{
  GList *themes = NULL;
  gchar *path = NULL;
	const gchar *const *paths = NULL;
	gint i = 0;

	path = g_build_path (G_DIR_SEPARATOR_S, g_get_user_data_dir (), "adium/message-styles", NULL);
  empathy_adium_chat_theme_discover_in_dir (path, themes);
  g_free (path);

  for (i = 0; paths[i]; i++)
    {
	    path = g_build_path (G_DIR_SEPARATOR_S, paths[i], "adium/message-styles", NULL);
      empathy_adium_chat_theme_discover_in_dir (path, themes);
      g_free (path);
    }

  return themes;
}

static EmpathyChatView *
empathy_adium_chat_theme_create_view (EmpathyChatTheme *theme)
{
  EmpathyAdiumChatThemePriv *priv;
  EmpathyThemeAdium *view;

  g_return_val_if_fail (EMPATHY_IS_ADIUM_CHAT_THEME (theme), NULL);

  priv = GET_PRIV (theme);
  
  view = empathy_theme_adium_new (priv->data);
  return EMPATHY_CHAT_VIEW (view);
}

static void
empathy_adium_chat_theme_constructed (GObject *object)
{
  EmpathyAdiumChatThemePriv *priv = GET_PRIV (object);
  priv->data = empathy_adium_data_new_with_info (
      tp_asv_get_string (priv->info, "path"),
      priv->info);
}

static void
empathy_adium_chat_theme_init (EmpathyAdiumChatTheme *theme)
{
  theme->priv = G_TYPE_INSTANCE_GET_PRIVATE (theme,
		EMPATHY_TYPE_ADIUM_CHAT_THEME, EmpathyAdiumChatThemePriv);
}

static void
empathy_adium_chat_theme_class_init (EmpathyAdiumChatThemeClass *theme_class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (theme_class);
  EmpathyChatThemeClass *chat_theme_class = EMPATHY_CHAT_THEME_CLASS (theme_class);

  object_class->constructed = empathy_adium_chat_theme_constructed;
  chat_theme_class->create_view = empathy_adium_chat_theme_create_view;
}

