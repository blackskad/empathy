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

/*
 * FIXME: add a variant based on theme colors
 */

#include "empathy-boxed-chat-theme.h"

#include "empathy-chat-view.h"
#include "empathy-theme-boxes.h"

G_DEFINE_TYPE (EmpathyBoxedChatTheme, empathy_boxed_chat_theme, EMPATHY_TYPE_CHAT_THEME);

#if 0
EmpathyThemeBoxesColors simple_colors = {
}
#endif

EmpathyThemeBoxesColors clean_colors = {
    "black",    /* header_foreground */
    "#efefdf",  /* header_background */
    "#e3e3d3",  /* header_line_background */
    "brown4",   /* action_foreground */
    "darkgrey", /* time_foreground */
    "darkgrey", /* event_foreground */
    "#49789e",  /* link_foreground */
    NULL,       /* text_foreground */
    NULL,       /* text_background */
    NULL        /* highlight_foreground */
};

EmpathyThemeBoxesColors blue_colors = {
    "black",    /* header_foreground */
    "#88a2b4",  /* header_background */
    "#7f96a4",  /* header_line_background */
    "brown4",   /* action_foreground */
    "darkgrey", /* time_foreground */
    "#7f96a4",  /* event_foreground */
    "#49789e",  /* link_foreground */
    "black",    /* text_foreground */
    "#adbdc8",  /* text_background */
    "black"     /* highlight_foreground */
};

static EmpathyChatTheme *
empathy_boxed_chat_theme_new (gchar *name)
{
  EmpathyChatTheme *theme;
  GList *variants = NULL;

  /* Add the three boxes variants */
  variants = g_list_append (variants, "simple");
  variants = g_list_append (variants, "clean");
  variants = g_list_append (variants, "blue");

  theme = EMPATHY_CHAT_THEME (g_object_new (EMPATHY_TYPE_BOXED_CHAT_THEME,
      "theme-name", name,
      "theme-variants" , variants,
      NULL));

  g_list_free (variants);

  return theme;
}

GList *
empathy_boxed_chat_theme_discover ()
{
  EmpathyChatTheme *theme;
  GList *themes = NULL;

  /* FIXME: use theme variants instead of new theme instances */
  theme = empathy_boxed_chat_theme_new ("clean");
  themes = g_list_append (themes, theme);

  theme = empathy_boxed_chat_theme_new ("blue");
  themes = g_list_append (themes, theme);

  theme = empathy_boxed_chat_theme_new ("simple");
  themes = g_list_append (themes, theme);

  return themes;
}

static EmpathyChatView *
empathy_boxed_chat_theme_create_view (EmpathyChatTheme *theme)
{
  EmpathyThemeBoxes *view;
  gchar *name;

  g_return_val_if_fail (EMPATHY_IS_BOXED_CHAT_THEME (theme), NULL);

  view = empathy_theme_boxes_new ();

  /* FIXME: Apply the correct colors for the selected variant */
  name = empathy_chat_theme_get_name (theme);  
  if (g_strcmp0 (name, "clean") == 0)
    {
      empathy_theme_boxes_set_colors (view, &clean_colors);
    }
  else if (g_strcmp0 (name, "blue") == 0)
    {
      empathy_theme_boxes_set_colors (view, &blue_colors);
    }
  else
    {
      empathy_theme_boxes_use_system_colors (view, TRUE);
    }

  return EMPATHY_CHAT_VIEW (view);
}

static void
empathy_boxed_chat_theme_init (EmpathyBoxedChatTheme *theme)
{
}

static void
empathy_boxed_chat_theme_class_init (EmpathyBoxedChatThemeClass *theme_class)
{
  EmpathyChatThemeClass *chat_theme_class = EMPATHY_CHAT_THEME_CLASS (theme_class);

  chat_theme_class->create_view = empathy_boxed_chat_theme_create_view;
}

