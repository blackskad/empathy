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

#include <libempathy/empathy-utils.h>

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
	THEME_CHANGED,
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
empathy_theme_manager_select (EmpathyThemeManager *self,
    EmpathyChatTheme *theme)
{
  EmpathyThemeManagerPriv *priv = GET_PRIV (self);

  g_assert (EMPATHY_IS_CHAT_THEME (theme));

  if (priv->selected)
  {
    g_object_unref (G_OBJECT (priv->selected));
  }

  g_object_ref (G_OBJECT (theme));
  priv->selected = theme;

  /* FIXME: update GConf data */
  /* FIXME: send theme-changed signal */
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

  signals[THEME_CHANGED] = g_signal_new ("theme-changed",
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
  EmpathyThemeManagerPriv *priv;

  /* setup the columns */
  GType types[] = {
    G_TYPE_STRING,            /* Display name */
    GDK_TYPE_PIXBUF,          /* Thumbnail */
    EMPATHY_TYPE_CHAT_THEME   /* The chat theme */
  };

	gtk_list_store_set_column_types (GTK_LIST_STORE (self),
	    EMPATHY_THEME_MANAGER_COUNT, types);

  /* add priv struct */
  priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
      EMPATHY_TYPE_THEME_MANAGER, EmpathyThemeManagerPriv);
  self->priv = priv;

  /* discover all themes */
  for (j = 0; providers[j]; j++)
    {
      themes = providers[j]();
      for (i = themes; i; i=i->next)
        {
          empathy_theme_manager_add_theme (self, i->data);
        }
    }
}


#if 0

/* OLD STUFF, LET's MOVE IT! */

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
    }
  g_free (filename);
}

gboolean
empathy_theme_manager_install_theme (gchar *path)
{
  GFile *file;
  gchar *filename;

  g_return_val_if_fail (!EMP_STR_EMPTY (path), FALSE);
  g_return_val_if_fail (g_strcmp0(path, "adiumxtra://") != 0, FALSE);

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
  g_free (filename);

  g_file_load_contents_async (file, NULL,
      empathy_theme_manager_theme_file_loaded, NULL);

  return TRUE;
}
#endif
