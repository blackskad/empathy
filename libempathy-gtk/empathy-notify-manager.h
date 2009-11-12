/*
 * Copyright (C) 2009 Collabora Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Authors: Guillaume Desmottes <guillaume.desmottes@collabora.co.uk>
 */

#ifndef __EMPATHY_NOTIFY_MANAGER_H__
#define __EMPATHY_NOTIFY_MANAGER_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define EMPATHY_TYPE_NOTIFY_MANAGER         (empathy_notify_manager_get_type ())
#define EMPATHY_NOTIFY_MANAGER(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), EMPATHY_TYPE_NOTIFY_MANAGER, EmpathyNotifyManager))
#define EMPATHY_NOTIFY_MANAGER_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), EMPATHY_TYPE_NOTIFY_MANAGER, EmpathyNotifyManagerClass))
#define EMPATHY_IS_NOTIFY_MANAGER(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), EMPATHY_TYPE_NOTIFY_MANAGER))
#define EMPATHY_IS_NOTIFY_MANAGER_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), EMPATHY_TYPE_NOTIFY_MANAGER))
#define EMPATHY_NOTIFY_MANAGER_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), EMPATHY_TYPE_NOTIFY_MANAGER, EmpathyNotifyManagerClass))

typedef struct _EmpathyNotifyManager      EmpathyNotifyManager;
typedef struct _EmpathyNotifyManagerClass EmpathyNotifyManagerClass;

struct _EmpathyNotifyManager
{
  GObject parent;
  gpointer priv;
};

struct _EmpathyNotifyManagerClass
{
 GObjectClass parent_class;
};

GType empathy_notify_manager_get_type (void) G_GNUC_CONST;

/* Get the notify_manager singleton */
EmpathyNotifyManager * empathy_notify_manager_dup_singleton (void);

G_END_DECLS

#endif /* __EMPATHY_NOTIFY_MANAGER_H__ */