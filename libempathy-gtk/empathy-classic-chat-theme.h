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

#ifndef __EMPATHY_CLASSIC_CHAT_THEME_H__
#define __EMPATHY_CLASSIC_CHAT_THEME_H__

#include <glib.h>

#include "empathy-chat-theme.h"

G_BEGIN_DECLS

#define EMPATHY_TYPE_CLASSIC_CHAT_THEME (empathy_classic_chat_theme_get_type ())
#define EMPATHY_CLASSIC_CHAT_THEME(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), \
    EMPATHY_TYPE_CLASSIC_CHAT_THEME, EmpathyClassicChatTheme))
#define EMPATHY_CLASSIC_CHAT_THEME_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), \
    EMPATHY_TYPE_CLASSIC_CHAT_THEME, EmpathyClassicChatThemeClass))
#define EMPATHY_IS_CLASSIC_CHAT_THEME(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
    EMPATHY_TYPE_CLASSIC_CHAT_THEME))
#define EMPATHY_IS_CLASSIC_CHAT_THEME_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), \
    EMPATHY_TYPE_CLASSIC_CHAT_THEME))
#define EMPATHY_CLASSIC_CHAT_THEME_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o),\
    EMPATHY_TYPE_CLASSIC_CHAT_THEME, EmpathyClassicChatThemeClass))

typedef struct _EmpathyClassicChatTheme EmpathyClassicChatTheme;
typedef struct _EmpathyClassicChatThemeClass EmpathyClassicChatThemeClass;

struct _EmpathyClassicChatTheme
{
  EmpathyChatTheme parent;
};

struct _EmpathyClassicChatThemeClass
{
  EmpathyChatThemeClass parent_class;
};

GType  empathy_classic_chat_theme_get_type (void);
GList *empathy_classic_chat_theme_discover (void);

G_END_DECLS

#endif /* __EMPATHY_CLASSIC_CHAT_THEME_H__ */
