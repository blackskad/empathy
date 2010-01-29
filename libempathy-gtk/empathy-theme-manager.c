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

#include <string.h>

#include <glib/gi18n-lib.h>
#include <telepathy-glib/dbus.h>
#include <gtk/gtk.h>

#include <telepathy-glib/util.h>
#include <libempathy/empathy-utils.h>

#include "empathy-theme-manager.h"
#include "empathy-chat-view.h"
#include "empathy-conf.h"
#include "empathy-chat-theme.h"
#include "empathy-chat-text-view.h"
#include "empathy-classic-chat-theme.h"
#include "empathy-theme-boxes.h"
#include "empathy-theme-irc.h"

#ifdef HAVE_WEBKIT
#include "empathy-theme-adium.h"
#endif

#define DEBUG_FLAG EMPATHY_DEBUG_OTHER
#include <libempathy/empathy-debug.h>

#define GET_PRIV(obj) EMPATHY_GET_PRIV (obj, EmpathyThemeManager)

typedef struct {
  EmpathyChatTheme *selected;
} EmpathyThemeManagerPriv;

enum {
	THEME_CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

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

EmpathyChatView *
empathy_theme_manager_create_view (EmpathyThemeManager *manager)
{
	EmpathyThemeManagerPriv *priv = GET_PRIV (manager);

	g_return_val_if_fail (EMPATHY_IS_THEME_MANAGER (manager), NULL);

  if (!priv->selected)
    {
      /* FIXME: use the first theme as selected theme */
      return NULL;
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
  GList *i;
  GList *themes;
  EmpathyThemeManagerPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
      EMPATHY_TYPE_THEME_MANAGER, EmpathyThemeManagerPriv);

  /* setup the columns */
  GType types[] = {
    G_TYPE_STRING,            /* Display name */
    //GDK_TYPE_PIXBUF,          /* Preview */
    EMPATHY_TYPE_CHAT_THEME   /* The chat theme */
  };

  self->priv = priv;

	gtk_list_store_set_column_types (GTK_LIST_STORE (self),
	    EMPATHY_THEME_MANAGER_COUNT, types);

  /* discover all themes */
  themes = empathy_classic_chat_theme_discover ();
  for (i = themes; i; i=i->next)
    {
      empathy_theme_manager_add_theme (self, i->data);
    }
}


#if 0

/* OLD STUFF, LET's MOVE IT! */

static void
theme_manager_gdk_color_to_hex (GdkColor *gdk_color, gchar *str_color)
{
	g_snprintf (str_color, 10,
		    "#%02x%02x%02x",
		    gdk_color->red >> 8,
		    gdk_color->green >> 8,
		    gdk_color->blue >> 8);
}

static void
theme_manager_boxes_weak_notify_cb (gpointer data,
				    GObject *where_the_object_was)
{
	EmpathyThemeManagerPriv *priv = GET_PRIV (data);

	priv->boxes_views = g_list_remove (priv->boxes_views, where_the_object_was);
}

static EmpathyThemeBoxes *
theme_manager_create_boxes_view (EmpathyThemeManager *manager)
{
	EmpathyThemeManagerPriv *priv = GET_PRIV (manager);
	EmpathyThemeBoxes       *theme;

	theme = empathy_theme_boxes_new ();
	priv->boxes_views = g_list_prepend (priv->boxes_views, theme);
	g_object_weak_ref (G_OBJECT (theme),
			   theme_manager_boxes_weak_notify_cb,
			   manager);

	return theme;
}

static void
theme_manager_update_boxes_tags (EmpathyThemeBoxes *theme,
				 const gchar       *header_foreground,
				 const gchar       *header_background,
				 const gchar       *header_line_background,
				 const gchar       *action_foreground,
				 const gchar       *time_foreground,
				 const gchar       *event_foreground,
				 const gchar       *link_foreground,
				 const gchar       *text_foreground,
				 const gchar       *text_background,
				 const gchar       *highlight_foreground)

{
	EmpathyChatTextView *view = EMPATHY_CHAT_TEXT_VIEW (theme);
	GtkTextTag          *tag;

	DEBUG ("Update view with new colors:\n"
		"header_foreground = %s\n"
		"header_background = %s\n"
		"header_line_background = %s\n"
		"action_foreground = %s\n"
		"time_foreground = %s\n"
		"event_foreground = %s\n"
		"link_foreground = %s\n"
		"text_foreground = %s\n"
		"text_background = %s\n"
		"highlight_foreground = %s\n",
		header_foreground, header_background, header_line_background,
		action_foreground, time_foreground, event_foreground,
		link_foreground, text_foreground, text_background,
		highlight_foreground);


	/* FIXME: GtkTextTag don't support to set color properties to NULL.
	 * See bug #542523 */

	#define TAG_SET(prop, prop_set, value) \
		if (value != NULL) { \
			g_object_set (tag, prop, value, NULL); \
		} else { \
			g_object_set (tag, prop_set, FALSE, NULL); \
		}

	/* Define base tags */
	tag = empathy_chat_text_view_tag_set (view, EMPATHY_CHAT_TEXT_VIEW_TAG_HIGHLIGHT,
					      "weight", PANGO_WEIGHT_BOLD,
					      "pixels-above-lines", 4,
					      NULL);
	TAG_SET ("paragraph-background", "paragraph-background-set", text_background);
	TAG_SET ("foreground", "foreground-set", highlight_foreground);

	empathy_chat_text_view_tag_set (view, EMPATHY_CHAT_TEXT_VIEW_TAG_SPACING,
					"size", 3000,
					"pixels-above-lines", 8,
					NULL);
	tag = empathy_chat_text_view_tag_set (view, EMPATHY_CHAT_TEXT_VIEW_TAG_TIME,
					      "justification", GTK_JUSTIFY_CENTER,
					      NULL);
	TAG_SET ("foreground", "foreground-set", time_foreground);
	tag = empathy_chat_text_view_tag_set (view, EMPATHY_CHAT_TEXT_VIEW_TAG_ACTION,
					      "style", PANGO_STYLE_ITALIC,
					      "pixels-above-lines", 4,
					      NULL);
	TAG_SET ("paragraph-background", "paragraph-background-set", text_background);
	TAG_SET ("foreground", "foreground-set", action_foreground);
	tag = empathy_chat_text_view_tag_set (view, EMPATHY_CHAT_TEXT_VIEW_TAG_BODY,
					      "pixels-above-lines", 4,
					      NULL);
	TAG_SET ("paragraph-background", "paragraph-background-set", text_background);
	TAG_SET ("foreground", "foreground-set", text_foreground);
	tag = empathy_chat_text_view_tag_set (view, EMPATHY_CHAT_TEXT_VIEW_TAG_EVENT,
					      "justification", GTK_JUSTIFY_LEFT,
					      NULL);
	TAG_SET ("foreground", "foreground-set", event_foreground);
	tag = empathy_chat_text_view_tag_set (view, EMPATHY_CHAT_TEXT_VIEW_TAG_LINK,
					      "underline", PANGO_UNDERLINE_SINGLE,
					      NULL);
	TAG_SET ("foreground", "foreground-set", link_foreground);

	/* Define BOXES tags */
	tag = empathy_chat_text_view_tag_set (view, EMPATHY_THEME_BOXES_TAG_HEADER,
					      "weight", PANGO_WEIGHT_BOLD,
					      NULL);
	TAG_SET ("foreground", "foreground-set", header_foreground);
	TAG_SET ("paragraph-background", "paragraph-background-set", header_background);
	tag = empathy_chat_text_view_tag_set (view, EMPATHY_THEME_BOXES_TAG_HEADER_LINE,
					      "size", 1,
					      NULL);
	TAG_SET ("paragraph-background", "paragraph-background-set", header_line_background);

	#undef TAG_SET
}

static void
on_style_set_cb (GtkWidget *widget, GtkStyle *previous_style, gpointer data)
{
	GtkStyle *style;
	gchar     color1[10];
	gchar     color2[10];
	gchar     color3[10];
	gchar     color4[10];

	style = gtk_widget_get_style (GTK_WIDGET (widget));

	theme_manager_gdk_color_to_hex (&style->base[GTK_STATE_SELECTED], color1);
	theme_manager_gdk_color_to_hex (&style->bg[GTK_STATE_SELECTED], color2);
	theme_manager_gdk_color_to_hex (&style->dark[GTK_STATE_SELECTED], color3);
	theme_manager_gdk_color_to_hex (&style->fg[GTK_STATE_SELECTED], color4);

	theme_manager_update_boxes_tags (EMPATHY_THEME_BOXES (widget),
					 color4,     /* header_foreground */
					 color2,     /* header_background */
					 color3,     /* header_line_background */
					 color1,     /* action_foreground */
					 "darkgrey", /* time_foreground */
					 "darkgrey", /* event_foreground */
					 color1,     /* link_foreground */
					 NULL,       /* text_foreground */
					 NULL,       /* text_background */
					 NULL);      /* highlight_foreground */
}

static void
theme_manager_update_boxes_theme (EmpathyThemeManager *manager,
				  EmpathyThemeBoxes   *theme)
{
	EmpathyThemeManagerPriv *priv = GET_PRIV (manager);

	if (strcmp (priv->name, "simple") == 0) {
		g_signal_connect (G_OBJECT (theme), "style-set",
				  G_CALLBACK (on_style_set_cb), theme);
	}
	else if (strcmp (priv->name, "clean") == 0) {
		theme_manager_update_boxes_tags (theme,
						 "black",    /* header_foreground */
						 "#efefdf",  /* header_background */
						 "#e3e3d3",  /* header_line_background */
						 "brown4",   /* action_foreground */
						 "darkgrey", /* time_foreground */
						 "darkgrey", /* event_foreground */
						 "#49789e",  /* link_foreground */
						 NULL,       /* text_foreground */
						 NULL,       /* text_background */
						 NULL);      /* highlight_foreground */
	}
	else if (strcmp (priv->name, "blue") == 0) {
		theme_manager_update_boxes_tags (theme,
						 "black",    /* header_foreground */
						 "#88a2b4",  /* header_background */
						 "#7f96a4",  /* header_line_background */
						 "brown4",   /* action_foreground */
						 "darkgrey", /* time_foreground */
						 "#7f96a4",  /* event_foreground */
						 "#49789e",  /* link_foreground */
						 "black",    /* text_foreground */
						 "#adbdc8",  /* text_background */
						 "black");   /* highlight_foreground */
	}
}

const gchar **
empathy_theme_manager_get_themes (void)
{
	return themes;
}

#ifdef HAVE_WEBKIT
static void
find_themes (GList **list, const gchar *dirpath)
{
	GDir *dir;
	GError *error = NULL;
	const gchar *name = NULL;
	GHashTable *info = NULL;

	dir = g_dir_open (dirpath, 0, &error);
	if (dir != NULL) {
		name = g_dir_read_name (dir);
		while (name != NULL) {
			gchar *path;

			path = g_build_path (G_DIR_SEPARATOR_S, dirpath, name, NULL);
			if (empathy_adium_path_is_valid (path)) {
				info = empathy_adium_info_new (path);
				if (info != NULL) {
					*list = g_list_prepend (*list, info);
				}
			}
			g_free (path);
			name = g_dir_read_name (dir);
		}
		g_dir_close (dir);
	} else {
		DEBUG ("Error opening %s: %s\n", dirpath, error->message);
		g_error_free (error);
	}
}
#endif /* HAVE_WEBKIT */

GList *
empathy_theme_manager_get_adium_themes (void)
{
#ifdef HAVE_WEBKIT
	GList *themes_list = NULL;
	gchar *userpath = NULL;
	const gchar *const *paths = NULL;
	gint i = 0;

	userpath = g_build_path (G_DIR_SEPARATOR_S, g_get_user_data_dir (), "adium/message-styles", NULL);
	find_themes (&themes_list, userpath);
	g_free (userpath);

	paths = g_get_system_data_dirs ();
	for (i = 0; paths[i] != NULL; i++) {
		userpath = g_build_path (G_DIR_SEPARATOR_S, paths[i],
			"adium/message-styles", NULL);
		find_themes (&themes_list, userpath);
		g_free (userpath);
	}

	return themes_list;
#else
	return NULL;
#endif /* HAVE_WEBKIT */
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
