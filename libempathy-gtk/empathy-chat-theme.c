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
#include <glib/gstdio.h>
#include <glib-object.h>

#include <libempathy/empathy-utils.h>

#include <libempathy-gtk/empathy-conf.h>

#include "empathy-chat-theme.h"
#include "empathy-chat-view.h"

#define GET_PRIV(o) EMPATHY_GET_PRIV (o, EmpathyChatTheme)

G_DEFINE_ABSTRACT_TYPE (EmpathyChatTheme, empathy_chat_theme, G_TYPE_OBJECT);

typedef struct _EmpathyChatThemePriv EmpathyChatThemePriv;

struct _EmpathyChatThemePriv
{
  gchar *name;
  gchar *variant;
  GList *variants;
  GdkPixbuf *thumbnail;
};

typedef enum {
  PROP_0,
  PROP_THEME_NAME,
  PROP_THEME_THUMBNAIL,
  PROP_THEME_VARIANTS,
  PROP_THEME_VARIANT
} EmpathyChatThemeProperty;

enum {
  VARIANT_CHANGED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

EmpathyChatView *
empathy_chat_theme_create_view (EmpathyChatTheme *self)
{
  g_return_val_if_fail (EMPATHY_IS_CHAT_THEME (self), NULL);

  if (EMPATHY_CHAT_THEME_GET_CLASS (self)->create_view)
    {
      return EMPATHY_CHAT_THEME_GET_CLASS (self)->create_view (self);
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
empathy_chat_theme_create_thumbnail (EmpathyChatTheme *theme,
    gchar *cachefile)
{
  GdkPixmap *full;
  EmpathyChatView *view;
  GtkWidget *window;
  GError *error = NULL;
  int width, height;
  GtkAllocation allocation = {0, 0, 400, 400};

  EmpathyChatThemePriv *priv = GET_PRIV (theme);

  /* Create a new theme view, and replay a dummy conversation. Force GTK+ to
   * handle all events before replaying the conversation. If not, we might
   * see some javascript errors and no messages in the screenshot.
   */
  view = empathy_chat_theme_create_view (theme);
  gtk_widget_size_allocate (GTK_WIDGET (view), &allocation);

  while (gtk_events_pending ())
    {
      gtk_main_iteration_do (FALSE);
    }

  empathy_chat_view_append_event (view, "blackskad has left the room");

  /* Render the view to an offscreen window. After that, force GTK+ to handle
   * all pening events. The resulting pixmap would be just gray if those events
   * aren't handled first. An async version would get rid of the forcing */
  window = gtk_offscreen_window_new ();
  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (view));

  /* FIXME: GtkTextView doesn't like GtkOffscreenWindow, see #609818*/ 
  if (!GTK_IS_TEXT_VIEW (view))
    {
      gtk_widget_show_all (window);

      while (gtk_events_pending ())
        {
          gtk_main_iteration_do (FALSE);
        }

      /* get a snapshot and create the preview, the easy way...
       * FIXME: develop an algorithm to discover the interesting parts of an image
       * */
      full = gtk_offscreen_window_get_pixmap (GTK_OFFSCREEN_WINDOW (window));
      gdk_drawable_get_size (full, &width, &height);
      priv->thumbnail = gdk_pixbuf_get_from_drawable (NULL, full, gdk_colormap_get_system (),
          0, 0, 0, 0, (width < 100) ? width : 100, (height < 100) ? height : 100);

      /* save the preview to the cache */
      gdk_pixbuf_save (priv->thumbnail, cachefile, "png", &error, NULL);
      if (error)
        {
          g_message("Failed to save thumbnail for theme %s: %s\n", priv->name, error->message);
        }
    }
  gtk_widget_destroy(window);
}

static void
empathy_chat_theme_load_thumbnail (EmpathyChatTheme *theme)
{
  gchar *cachefile;
  gchar *cachepath;

  EmpathyChatThemePriv *priv = GET_PRIV (theme);

  /* test if the cache dir exists */
  cachepath = g_build_path (G_DIR_SEPARATOR_S, g_get_user_cache_dir (),
      G_DIR_SEPARATOR_S, "empathy/themes/thumnails/", NULL);
  if (!g_file_test(cachepath, G_FILE_TEST_EXISTS))
    {
      g_mkdir_with_parents (cachepath, 777);
    }

  /* let's try to load the thumbnail from the cache */
  cachefile = g_strdup_printf ("%s/%s.png", cachepath, priv->name);
  if (g_file_test(cachefile, G_FILE_TEST_EXISTS))
    {
      GError *error = NULL;
      priv->thumbnail = gdk_pixbuf_new_from_file (cachefile, &error);
      if (error)
        {
          g_message ("Failed to load thumnail from cache: %s", error->message);
        }
      else { g_message ("yay, loaded from file :D");}
    }

  /* thumbnail is still null, create a new one */
  if (priv->thumbnail == NULL)
    {
      empathy_chat_theme_create_thumbnail (theme, cachefile);
    }

  g_free(cachefile);
  g_free(cachepath);
}

GdkPixbuf *
empathy_chat_theme_get_thumbnail (EmpathyChatTheme *theme)
{
  EmpathyChatThemePriv *priv = GET_PRIV (theme);
  if (!priv->thumbnail)
    {
      /* Taking off screen snapshots seems to be something for the future.
       * So don't do anything if the thumbnail wasn't set on construction.*/
      empathy_chat_theme_load_thumbnail (theme);
    }
  return gdk_pixbuf_copy (priv->thumbnail);
}

GList *
empathy_chat_theme_get_variants (EmpathyChatTheme *theme)
{
  EmpathyChatThemePriv *priv = GET_PRIV (theme);
  return priv->variants;
}

void
empathy_chat_theme_set_selected_variant (EmpathyChatTheme *theme,
    gchar *variant)
{
  EmpathyChatThemePriv *priv = GET_PRIV (theme);
  /* FIXME: check if the proposed variant is really a variant of this theme. */
  if (priv->variant)
    {
      g_free (priv->variant);
    }
  priv->variant = g_strdup (variant);
  g_signal_emit (theme, signals[VARIANT_CHANGED], 0);
}

gchar *
empathy_chat_theme_get_selected_variant (EmpathyChatTheme *theme)
{
  EmpathyChatThemePriv *priv = GET_PRIV (theme);
  return g_strdup (priv->variant);
}

static void
empathy_chat_theme_get_property (GObject *object,
    guint param_id,
    GValue *value,
    GParamSpec *pspec)
{
  switch (param_id)
    {
      case PROP_THEME_NAME:
        /* FIXME: return the name */
        break;
      case PROP_THEME_THUMBNAIL:
        /* FIXME: return the preview */
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
  GList *variants;
  EmpathyChatThemePriv *priv = GET_PRIV (object);

  switch (param_id)
    {
      case PROP_THEME_NAME:
        g_assert (priv->name == NULL);
        priv->name = g_strdup (g_value_get_string (value));
        break;
      case PROP_THEME_THUMBNAIL:
        g_assert (priv->thumbnail == NULL);
        priv->thumbnail = gdk_pixbuf_copy (g_value_get_object (value));
        break;
      case PROP_THEME_VARIANTS:
        g_assert (priv->variants == NULL);
        /* make a deep copy of the list */
        variants = g_value_get_pointer (value);
        for (; variants; variants = variants->next)
        {
          priv->variants = g_list_append (priv->variants, g_strdup (variants->data));
        }
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
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

	g_object_class_install_property (object_class, PROP_THEME_THUMBNAIL,
      g_param_spec_object ("theme-thumbnail",
          "The theme thumbnail",
          "A small thumbnail of the theme",
          GDK_TYPE_PIXBUF,
          G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (object_class, PROP_THEME_VARIANTS,
      g_param_spec_pointer ("theme-variants",
          "The theme variants",
          "A list of strings with the names of the theme variants",
          G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  signals[VARIANT_CHANGED] = g_signal_new ("variant-changed",
      G_OBJECT_CLASS_TYPE (object_class),
      G_SIGNAL_RUN_LAST,
      0,
      NULL, NULL,
      g_cclosure_marshal_VOID__VOID,
      G_TYPE_NONE,
      0);

  g_type_class_add_private (object_class, sizeof (EmpathyChatThemePriv));
}
