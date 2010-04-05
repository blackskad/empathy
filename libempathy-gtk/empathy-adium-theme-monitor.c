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
#include <gio/gio.h>

#include "empathy-chat-theme.h"
#include "empathy-theme-adium.h"
#include "empathy-adium-chat-theme.h"
#include "empathy-adium-theme-monitor.h"

/* the directory for themes (located within user_data_dir) */
#define THEME_DIRECTORY "adium/message-styles"

#define GET_PRIV(o) EMPATHY_GET_PRIV (o, EmpathyAdiumThemeMonitor)

G_DEFINE_TYPE (EmpathyAdiumThemeMonitor, empathy_adium_theme_monitor, G_TYPE_OBJECT);

typedef struct _EmpathyAdiumThemeMonitorPriv EmpathyAdiumThemeMonitorPriv;

struct _EmpathyAdiumThemeMonitorPriv
{
};

enum {
  THEME_ADDED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

EmpathyAdiumThemeMonitor *
empathy_adium_theme_monitor_new () {
  return EMPATHY_ADIUM_THEME_MONITOR (
      g_object_new (EMPATHY_TYPE_ADIUM_THEME_MONITOR, NULL));
}

static void
empathy_adium_theme_monitor_directory_changed (GFileMonitor *monitor,
    GFile *source,
    GFile *changed,
    GFileMonitorEvent event,
    gpointer user_data)
{
  EmpathyAdiumThemeMonitor *self = user_data;
  if (event == G_FILE_MONITOR_EVENT_CREATED)
    {
      EmpathyChatTheme *theme;
      gchar *path = g_file_get_path (source);
      /* FIXME: we should recurse here to find themes in subdirs too */
      theme = empathy_adium_chat_theme_new_for_path (path);
      if (theme != NULL)
        {
          g_signal_emit (G_OBJECT(self), signals[THEME_ADDED], 0, theme);
        }
      g_free (path);
    }
  /* FIXME: how should we handle the removal of a theme directory
   * Perhaps make the adium theme monitor it's own directory?
  else if (event == G_FILE_MONITOR_EVENT_DELETED)
    {
    }
  */
}

static void
empathy_adium_theme_monitor_follow (EmpathyAdiumThemeMonitor *self,
    gchar *directory)
{
  GFile *file;
  GFileMonitor *monitor;

  file = g_file_new_for_path (directory);
  monitor = g_file_monitor_directory (file, G_FILE_MONITOR_NONE, NULL, NULL);
  if (monitor)
    {
      g_signal_connect (monitor, "changed",
          G_CALLBACK (empathy_adium_theme_monitor_directory_changed), self);
    }
}

static void
empathy_adium_theme_monitor_init (EmpathyAdiumThemeMonitor *self)
{
	gint i = 0;
  gchar *path = NULL;
	const gchar *const *paths = NULL;

  /* Follow the user and system data dirs by default.
   * A gconf key could be added to allow monitoring other dirs, if there's interest */
  path = g_build_path (G_DIR_SEPARATOR_S, g_get_user_data_dir (), THEME_DIRECTORY, NULL);
  empathy_adium_theme_monitor_follow (self, path);
  g_free (path);

  paths = g_get_system_data_dirs ();
  for (i = 0; paths[i]; i++)
    {
	    path = g_build_path (G_DIR_SEPARATOR_S, paths[i], THEME_DIRECTORY, NULL);
      empathy_adium_theme_monitor_follow (self, path);
      g_free (path);
    }
}

static void
empathy_adium_theme_monitor_class_init (EmpathyAdiumThemeMonitorClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  signals[THEME_ADDED] = g_signal_new ("theme-added",
    G_OBJECT_CLASS_TYPE (object_class),
    G_SIGNAL_RUN_LAST, 0,
    NULL, NULL,
    g_cclosure_marshal_VOID__OBJECT,
    G_TYPE_NONE, 1, EMPATHY_TYPE_CHAT_THEME);
}
