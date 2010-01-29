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

#include "empathy-classic-chat-theme.h"

#include "empathy-chat-view.h"
#include "empathy-theme-irc.h"

G_DEFINE_TYPE (EmpathyClassicChatTheme, empathy_classic_chat_theme, EMPATHY_TYPE_CHAT_THEME);

static EmpathyChatTheme *
empathy_classic_chat_theme_new ()
{
  return g_object_new (EMPATHY_TYPE_CLASSIC_CHAT_THEME, "theme-name", "classic");
}

GList *
empathy_classic_chat_theme_discover ()
{
  EmpathyChatTheme *theme;
  GList *themes = NULL;

  theme = empathy_classic_chat_theme_new ();
  themes = g_list_append (themes, theme);

  return themes;
}

static EmpathyChatView *
empathy_classic_chat_theme_create_view (EmpathyChatTheme *theme)
{
  EmpathyChatTextView *view;
  EmpathyThemeIrc *irc_view;

  irc_view = empathy_theme_irc_new ();
  view = EMPATHY_CHAT_TEXT_VIEW (irc_view);

  /* Define base tags */
  empathy_chat_text_view_tag_set (view, EMPATHY_CHAT_TEXT_VIEW_TAG_SPACING,
      "size", 2000,
      NULL);
  empathy_chat_text_view_tag_set (view, EMPATHY_CHAT_TEXT_VIEW_TAG_TIME,
      "foreground", "darkgrey",
      "justification", GTK_JUSTIFY_CENTER,
      NULL);
  empathy_chat_text_view_tag_set (view, EMPATHY_CHAT_TEXT_VIEW_TAG_ACTION,
      "foreground", "brown4",
      "style", PANGO_STYLE_ITALIC,
      NULL);
  empathy_chat_text_view_tag_set (view, EMPATHY_CHAT_TEXT_VIEW_TAG_BODY,
      "foreground-set", FALSE,
      NULL);
  empathy_chat_text_view_tag_set (view, EMPATHY_CHAT_TEXT_VIEW_TAG_EVENT,
      "foreground", "PeachPuff4",
      "justification", GTK_JUSTIFY_LEFT,
      NULL);
  empathy_chat_text_view_tag_set (view, EMPATHY_CHAT_TEXT_VIEW_TAG_LINK,
      "foreground", "steelblue",
      "underline", PANGO_UNDERLINE_SINGLE,
      NULL);
  empathy_chat_text_view_tag_set (view, EMPATHY_CHAT_TEXT_VIEW_TAG_HIGHLIGHT,
      "background", "yellow",
      NULL);

  /* Define IRC tags */
  empathy_chat_text_view_tag_set (view, EMPATHY_THEME_IRC_TAG_NICK_SELF,
      "foreground", "sea green",
      NULL);
  empathy_chat_text_view_tag_set (view, EMPATHY_THEME_IRC_TAG_NICK_OTHER,
      "foreground", "skyblue4",
      NULL);
  empathy_chat_text_view_tag_set (view, EMPATHY_THEME_IRC_TAG_NICK_HIGHLIGHT,
      "foreground", "indian red",
      "weight", PANGO_WEIGHT_BOLD,
      NULL);
   
  return EMPATHY_CHAT_VIEW (irc_view);
}

static void
empathy_classic_chat_theme_init (EmpathyClassicChatTheme *theme)
{
}

static void
empathy_classic_chat_theme_class_init (EmpathyClassicChatThemeClass *theme_class)
{
  EmpathyChatThemeClass *chat_theme_class = EMPATHY_CHAT_THEME_GET_CLASS (theme_class);

  chat_theme_class->create_view = empathy_classic_chat_theme_create_view;
}

