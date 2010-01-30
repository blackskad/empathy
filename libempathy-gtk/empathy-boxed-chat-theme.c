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

EmpathyThemeBoxesColors clean_colors = {
}

EmpathyThemeBoxesColors blue_colors = {
}
#endif

static EmpathyChatTheme *
empathy_boxed_chat_theme_new ()
{
  EmpathyChatTheme *theme;
  GList *variants = NULL;

  /* Add the three boxes variants */
  variants = g_list_append (variants, "simple");
  variants = g_list_append (variants, "clean");
  variants = g_list_append (variants, "blue");

  theme = EMPATHY_CHAT_THEME (g_object_new (EMPATHY_TYPE_BOXED_CHAT_THEME,
      "theme-name", "boxed",
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

  theme = empathy_boxed_chat_theme_new ();
  themes = g_list_append (themes, theme);

  return themes;
}

static EmpathyChatView *
empathy_boxed_chat_theme_create_view (EmpathyChatTheme *theme)
{
  EmpathyThemeBoxes *view;

  g_return_val_if_fail (EMPATHY_IS_BOXED_CHAT_THEME (theme), NULL);

  view = empathy_theme_boxes_new ();

  /* FIXME: Apply the correct colors */

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

