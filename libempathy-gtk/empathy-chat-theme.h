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

#ifndef __EMPATHY_CHAT_THEME_H__
#define __EMPATHY_CHAT_THEME_H__

#include <glib.h>
#include <glib-object.h>

#include "empathy-chat-view.h"

G_BEGIN_DECLS

#define EMPATHY_TYPE_CHAT_THEME (empathy_chat_theme_get_type ())
#define EMPATHY_CHAT_THEME(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), \
    EMPATHY_TYPE_CHAT_THEME, EmpathyChatTheme))
#define EMPATHY_CHAT_THEME_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), \
    EMPATHY_TYPE_CHAT_THEME, EmpathyChatThemeClass))
#define EMPATHY_IS_CHAT_THEME(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
    EMPATHY_TYPE_CHAT_THEME))
#define EMPATHY_IS_CHAT_THEME_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), \
    EMPATHY_TYPE_CHAT_THEME))
#define EMPATHY_CHAT_THEME_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o),\
    EMPATHY_TYPE_CHAT_THEME, EmpathyChatThemeClass))

typedef struct _EmpathyChatTheme EmpathyChatTheme;
typedef struct _EmpathyChatThemeClass EmpathyChatThemeClass;

struct _EmpathyChatTheme
{
  GObject parent;

  /*<private>*/
  gpointer priv;
};

struct _EmpathyChatThemeClass
{
  GObjectClass parent_class;
  
  EmpathyChatView *(*create_view) (EmpathyChatTheme *theme);
};

GType empathy_chat_theme_get_type (void) G_GNUC_CONST;

gchar     *empathy_chat_theme_get_name      (EmpathyChatTheme *theme);
GdkPixbuf *empathy_chat_theme_get_thumbnail (EmpathyChatTheme *theme);
GList     *empathy_chat_theme_get_variants  (EmpathyChatTheme *theme);
/* gchar  *empathy_chat_theme_get_selected_variant (EmpathyChatTheme *theme); */

EmpathyChatView  * empathy_chat_theme_create_view (EmpathyChatTheme *theme);

G_END_DECLS

#endif /* __EMPATHY_CHAT_THEME_H__ */

