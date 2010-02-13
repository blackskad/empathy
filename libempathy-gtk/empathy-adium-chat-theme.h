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

#ifndef __EMPATHY_ADIUM_CHAT_THEME_H__
#define __EMPATHY_ADIUM_CHAT_THEME_H__

#include <glib.h>

#include "empathy-chat-theme.h"

G_BEGIN_DECLS

#define EMPATHY_TYPE_ADIUM_CHAT_THEME (empathy_adium_chat_theme_get_type ())
#define EMPATHY_ADIUM_CHAT_THEME(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), \
    EMPATHY_TYPE_ADIUM_CHAT_THEME, EmpathyAdiumChatTheme))
#define EMPATHY_ADIUM_CHAT_THEME_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), \
    EMPATHY_TYPE_ADIUM_CHAT_THEME, EmpathyAdiumChatThemeClass))
#define EMPATHY_IS_ADIUM_CHAT_THEME(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
    EMPATHY_TYPE_ADIUM_CHAT_THEME))
#define EMPATHY_IS_ADIUM_CHAT_THEME_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), \
    EMPATHY_TYPE_ADIUM_CHAT_THEME))
#define EMPATHY_ADIUM_CHAT_THEME_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o),\
    EMPATHY_TYPE_ADIUM_CHAT_THEME, EmpathyAdiumChatThemeClass))

typedef struct _EmpathyAdiumChatTheme EmpathyAdiumChatTheme;
typedef struct _EmpathyAdiumChatThemeClass EmpathyAdiumChatThemeClass;

struct _EmpathyAdiumChatTheme
{
  EmpathyChatTheme parent;

  gpointer priv;
};

struct _EmpathyAdiumChatThemeClass
{
  EmpathyChatThemeClass parent_class;
};

GType  empathy_adium_chat_theme_get_type (void);
void   empathy_adium_chat_theme_install  (gchar* path);
GList *empathy_adium_chat_theme_discover (void);

G_END_DECLS

#endif /* __EMPATHY_ADIUM_CHAT_THEME_H__ */
