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

/* the directory for themes (located within user_data_dir) */
#define THEME_DIRECTORY "adium/message-styles"

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

static GList*
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
  return themes;
}

GList *
empathy_adium_chat_theme_discover ()
{
  GList *themes = NULL;
  gchar *path = NULL;
	const gchar *const *paths = NULL;
	gint i = 0;

	path = g_build_path (G_DIR_SEPARATOR_S, g_get_user_data_dir (), THEME_DIRECTORY, NULL);
  themes = empathy_adium_chat_theme_discover_in_dir (path, themes);
  g_free (path);

  paths = g_get_system_data_dirs ();
  for (i = 0; paths[i]; i++)
    {
	    path = g_build_path (G_DIR_SEPARATOR_S, paths[i], THEME_DIRECTORY, NULL);
      themes = empathy_adium_chat_theme_discover_in_dir (path, themes);
      g_free (path);
    }
  return themes;
}

static void
empathy_theme_manager_theme_file_loaded (GObject *source_object,
    GAsyncResult *result,
    gpointer user_data)
{
  GError *error = NULL;
  gchar *contents;
  gsize length;

  GFile *file = G_FILE (source_object);
  gchar *filename = g_file_get_uri (file);

  if (!g_file_load_contents_finish (file, result, &contents, &length, NULL, &error))
    {
      /* FIXME: show a nice error dialog */
      g_warning ("Failed to load '%s': %s", filename, error->message);
      g_error_free (error);
      return;
    }
  else
    {
      g_message ("Loaded the contents of %s", filename);

      /* extract the content to THEME_DIRECTORY */
      /* */
    }
  g_free (filename);
}

void
empathy_adium_chat_theme_install (gchar *path) {
  GFile *file;
  gchar *filename;

  g_return_if_fail (!EMP_STR_EMPTY (path));
  g_return_if_fail (g_strcmp0(path, "adiumxtra://") != 0);

  /* Assume we can use http to fetch the theme when the path is prefixed with
   * adiumxtra://
   * */
  if (g_str_has_prefix (path, "adiumxtra://"))
    {
      filename = g_strdup_printf ("http://%s", path + strlen ("adiumxtra://"));
    }
  else
    {
      filename = g_strdup (path);
    }

  /* Read the archive file & load it async */
  file = g_file_new_for_commandline_arg (filename);
  g_file_load_contents_async (file, NULL,
      empathy_adium_chat_theme_file_loaded, NULL);

  g_free (filename);
}

static EmpathyChatView *
empathy_adium_chat_theme_create_view (EmpathyChatTheme *theme)
{
  EmpathyAdiumChatThemePriv *priv;
  EmpathyThemeAdium *view;

  g_return_val_if_fail (EMPATHY_IS_ADIUM_CHAT_THEME (theme), NULL);

  priv = GET_PRIV (theme);
  
  if (!priv->data)
    {
      priv->data = empathy_adium_data_new_with_info (
          tp_asv_get_string (priv->info, "path"),
          priv->info);
    }

  view = empathy_theme_adium_new (priv->data);
  return EMPATHY_CHAT_VIEW (view);
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

  chat_theme_class->create_view = empathy_adium_chat_theme_create_view;

  g_type_class_add_private (object_class, sizeof (EmpathyAdiumChatThemePriv));
}

