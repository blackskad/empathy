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

#ifndef __EMPATHY_ADIUM_THEME_MONITOR_H__
#define __EMPATHY_ADIUM_THEME_MONITOR_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define EMPATHY_TYPE_ADIUM_THEME_MONITOR (empathy_adium_theme_monitor_get_type ())
#define EMPATHY_ADIUM_THEME_MONITOR(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), \
    EMPATHY_TYPE_ADIUM_THEME_MONITOR, EmpathyAdiumThemeMonitor))
#define EMPATHY_ADIUM_THEME_MONITOR_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), \
    EMPATHY_TYPE_ADIUM_THEME_MONITOR, EmpathyAdiumThemeMonitorClass))
#define EMPATHY_IS_ADIUM_THEME_MONITOR(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
    EMPATHY_TYPE_ADIUM_THEME_MONITOR))
#define EMPATHY_IS_ADIUM_THEME_MONITOR_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), \
    EMPATHY_TYPE_ADIUM_THEME_MONITOR))
#define EMPATHY_ADIUM_THEME_MONITOR_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o),\
    EMPATHY_TYPE_ADIUM_THEME_MONITOR, EmpathyAdiumThemeMonitorClass))

typedef struct _EmpathyAdiumThemeMonitor EmpathyAdiumThemeMonitor;
typedef struct _EmpathyAdiumThemeMonitorClass EmpathyAdiumThemeMonitorClass;

struct _EmpathyAdiumThemeMonitor
{
  GObject parent;

  /*<private>*/
  gpointer priv;
};

struct _EmpathyAdiumThemeMonitorClass
{
  GObjectClass parent_class;
};

GType empathy_adium_theme_monitor_get_type (void) G_GNUC_CONST;

EmpathyAdiumThemeMonitor* empathy_adium_theme_monitor_new (void);

G_END_DECLS

#endif /* __EMPATHY_ADIUM_THEME_MONITOR_H__ */

