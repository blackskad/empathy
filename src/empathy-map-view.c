/*
 * Copyright (C) 2008, 2009 Collabora Ltd.
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
 * Authors: Pierre-Luc Beaudoin <pierre-luc.beaudoin@collabora.co.uk>
 */

#include <config.h>

#include <sys/stat.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>

#include <champlain/champlain.h>
#include <champlain-gtk/champlain-gtk.h>
#include <clutter-gtk/clutter-gtk.h>
#include <telepathy-glib/util.h>

#include <libempathy/empathy-contact.h>
#include <libempathy/empathy-contact-manager.h>
#include <libempathy/empathy-utils.h>
#include <libempathy/empathy-location.h>

#include <libempathy-gtk/empathy-contact-list-store.h>
#include <libempathy-gtk/empathy-contact-list-view.h>
#include <libempathy-gtk/empathy-presence-chooser.h>
#include <libempathy-gtk/empathy-ui-utils.h>

#include "empathy-map-view.h"
#include "ephy-spinner.h"

#define DEBUG_FLAG EMPATHY_DEBUG_LOCATION
#include <libempathy/empathy-debug.h>

typedef struct {
  EmpathyContactList *contact_list;

  GtkWidget *window;
  GtkWidget *zoom_in;
  GtkWidget *zoom_out;
  GtkWidget *throbber;
  ChamplainView *map_view;
  ChamplainLayer *layer;
  guint timeout_id;
  /* reffed (EmpathyContact *) => borrowed (ChamplainMarker *) */
  GHashTable *markers;
  gulong members_changed_id;
} EmpathyMapView;

static void
map_view_state_changed (ChamplainView *view,
    GParamSpec *gobject,
    EmpathyMapView *window)
{
  ChamplainState state;

  g_object_get (G_OBJECT (view), "state", &state, NULL);
  if (state == CHAMPLAIN_STATE_LOADING)
    ephy_spinner_start (EPHY_SPINNER (window->throbber));
  else
    ephy_spinner_stop (EPHY_SPINNER (window->throbber));
}

static gboolean
contact_has_location (EmpathyContact *contact)
{
  GHashTable *location;

  location = empathy_contact_get_location (contact);

  if (location == NULL || g_hash_table_size (location) == 0)
    return FALSE;

  return TRUE;
}

static ChamplainMarker * create_marker (EmpathyMapView *window,
    EmpathyContact *contact);

static void
map_view_update_contact_position (EmpathyMapView *self,
    EmpathyContact *contact)
{
  gdouble lon, lat;
  GValue *value;
  GHashTable *location;
  ChamplainMarker *marker;
  gboolean has_location;

  has_location = contact_has_location (contact);

  marker = g_hash_table_lookup (self->markers, contact);
  if (marker == NULL)
    {
      if (!has_location)
        return;

      marker = create_marker (self, contact);
    }
  else if (!has_location)
    {
      champlain_base_marker_animate_out (CHAMPLAIN_BASE_MARKER (marker));
      return;
    }

  location = empathy_contact_get_location (contact);

  value = g_hash_table_lookup (location, EMPATHY_LOCATION_LAT);
  if (value == NULL)
    {
      clutter_actor_hide (CLUTTER_ACTOR (marker));
      return;
    }
  lat = g_value_get_double (value);

  value = g_hash_table_lookup (location, EMPATHY_LOCATION_LON);
  if (value == NULL)
    {
      clutter_actor_hide (CLUTTER_ACTOR (marker));
      return;
    }
  lon = g_value_get_double (value);

  champlain_base_marker_set_position (CHAMPLAIN_BASE_MARKER (marker), lat, lon);
  champlain_base_marker_animate_in (CHAMPLAIN_BASE_MARKER (marker));
}

static void
map_view_contact_location_notify (EmpathyContact *contact,
    GParamSpec *arg1,
    EmpathyMapView *self)
{
  map_view_update_contact_position (self, contact);
}

static void
map_view_zoom_in_cb (GtkWidget *widget,
    EmpathyMapView *window)
{
  champlain_view_zoom_in (window->map_view);
}

static void
map_view_zoom_out_cb (GtkWidget *widget,
    EmpathyMapView *window)
{
  champlain_view_zoom_out (window->map_view);
}

static void
map_view_zoom_fit_cb (GtkWidget *widget,
    EmpathyMapView *window)
{
  GList *item, *children;
  GPtrArray *markers;

  children = clutter_container_get_children (CLUTTER_CONTAINER (window->layer));
  markers =  g_ptr_array_sized_new (g_list_length (children) + 1);

  for (item = children; item != NULL; item = g_list_next (item))
    g_ptr_array_add (markers, (gpointer) item->data);

  g_ptr_array_add (markers, (gpointer) NULL);
  champlain_view_ensure_markers_visible (window->map_view,
    (ChamplainBaseMarker **) markers->pdata,
    TRUE);

  g_ptr_array_free (markers, TRUE);
  g_list_free (children);
}

static gboolean
marker_clicked_cb (ChamplainMarker *marker,
    ClutterButtonEvent *event,
    EmpathyContact *contact)
{
  GtkWidget *menu;

  if (event->button != 3)
    return FALSE;

  menu = empathy_contact_menu_new (contact,
      EMPATHY_CONTACT_FEATURE_CHAT |
      EMPATHY_CONTACT_FEATURE_CALL |
      EMPATHY_CONTACT_FEATURE_LOG |
      EMPATHY_CONTACT_FEATURE_INFO);

  if (menu == NULL)
    return FALSE;

  gtk_widget_show (menu);
  gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
      event->button, event->time);
  g_object_ref_sink (menu);
  g_object_unref (menu);

  return FALSE;
}

static void
map_view_contacts_update_label (ChamplainMarker *marker)
{
  const gchar *name;
  gchar *date;
  gchar *label;
  GValue *gtime;
  time_t loctime;
  GHashTable *location;
  EmpathyContact *contact;

  contact = g_object_get_data (G_OBJECT (marker), "contact");
  location = empathy_contact_get_location (contact);
  name = empathy_contact_get_name (contact);
  gtime = g_hash_table_lookup (location, EMPATHY_LOCATION_TIMESTAMP);

  if (gtime != NULL)
    {
      time_t now;

      loctime = g_value_get_int64 (gtime);
      date = empathy_time_to_string_relative (loctime);
      label = g_strconcat ("<b>", name, "</b>\n<small>", date, "</small>", NULL);
      g_free (date);

      now = time (NULL);

      /* if location is older than a week */
      if (now - loctime > (60 * 60 * 24 * 7))
        clutter_actor_set_opacity (CLUTTER_ACTOR (marker), 0.75 * 255);
    }
  else
    {
      label = g_strconcat ("<b>", name, "</b>\n", NULL);
    }

  champlain_marker_set_use_markup (CHAMPLAIN_MARKER (marker), TRUE);
  champlain_marker_set_text (CHAMPLAIN_MARKER (marker), label);

  g_free (label);
}

static ChamplainMarker *
create_marker (EmpathyMapView *window,
    EmpathyContact *contact)
{
  ClutterActor *marker;
  ClutterActor *texture;
  GdkPixbuf *avatar;

  marker = champlain_marker_new ();

  avatar = empathy_pixbuf_avatar_from_contact_scaled (contact, 32, 32);
  if (avatar != NULL)
    {
      texture = clutter_texture_new ();
      gtk_clutter_texture_set_from_pixbuf (CLUTTER_TEXTURE (texture), avatar,
          NULL);
      champlain_marker_set_image (CHAMPLAIN_MARKER (marker), texture);
      g_object_unref (avatar);
    }
  else
    champlain_marker_set_image (CHAMPLAIN_MARKER (marker), NULL);

  g_object_set_data_full (G_OBJECT (marker), "contact",
      g_object_ref (contact), g_object_unref);

  g_hash_table_insert (window->markers, g_object_ref (contact), marker);

  map_view_contacts_update_label (CHAMPLAIN_MARKER (marker));

  clutter_actor_set_reactive (CLUTTER_ACTOR (marker), TRUE);
  g_signal_connect (marker, "button-release-event",
      G_CALLBACK (marker_clicked_cb), contact);

  clutter_container_add (CLUTTER_CONTAINER (window->layer), marker, NULL);

  return CHAMPLAIN_MARKER (marker);
}

static void
contact_added (EmpathyMapView *window,
    EmpathyContact *contact)
{
  g_signal_connect (contact, "notify::location",
      G_CALLBACK (map_view_contact_location_notify), window);

  map_view_update_contact_position (window, contact);
}

static void
map_view_destroy_cb (GtkWidget *widget,
    EmpathyMapView *window)
{
  GHashTableIter iter;
  gpointer contact;

  g_source_remove (window->timeout_id);

  g_hash_table_iter_init (&iter, window->markers);
  while (g_hash_table_iter_next (&iter, &contact, NULL))
    g_signal_handlers_disconnect_by_func (contact,
        map_view_contact_location_notify, window);

  g_signal_handler_disconnect (window->contact_list,
      window->members_changed_id);

  g_hash_table_destroy (window->markers);
  g_object_unref (window->contact_list);
  g_object_unref (window->layer);
  g_slice_free (EmpathyMapView, window);
}

static gboolean
map_view_key_press_cb (GtkWidget *widget,
    GdkEventKey *event,
    gpointer user_data)
{
  if ((event->state & GDK_CONTROL_MASK && event->keyval == GDK_w)
      || event->keyval == GDK_Escape)
    {
      gtk_widget_destroy (widget);
      return TRUE;
    }

  return FALSE;
}

static gboolean
map_view_tick (EmpathyMapView *window)
{
  GList *marker;

  marker = clutter_container_get_children (CLUTTER_CONTAINER (window->layer));

  for (; marker; marker = marker->next)
    map_view_contacts_update_label (marker->data);

  return TRUE;
}

static void
contact_removed (EmpathyMapView *self,
    EmpathyContact *contact)
{
  ClutterActor *marker;

  marker = g_hash_table_lookup (self->markers, contact);
  if (marker == NULL)
    return;

  clutter_actor_destroy (marker);
  g_hash_table_remove (self->markers, contact);
}

static void
members_changed_cb (EmpathyContactList *list,
    EmpathyContact *contact,
    EmpathyContact *actor,
    guint reason,
    gchar *message,
    gboolean is_member,
    EmpathyMapView *self)
{
  if (is_member)
    {
      contact_added (self, contact);
    }
  else
    {
      contact_removed (self, contact);
    }
}

GtkWidget *
empathy_map_view_show (void)
{
  static EmpathyMapView *window = NULL;
  GtkBuilder *gui;
  GtkWidget *sw;
  GtkWidget *embed;
  GtkWidget *throbber_holder;
  gchar *filename;
  GList *members, *l;

  if (window)
    {
      empathy_window_present (GTK_WINDOW (window->window));
      return window->window;
    }

  window = g_slice_new0 (EmpathyMapView);

  /* Set up interface */
  filename = empathy_file_lookup ("empathy-map-view.ui", "src");
  gui = empathy_builder_get_file (filename,
     "map_view", &window->window,
     "zoom_in", &window->zoom_in,
     "zoom_out", &window->zoom_out,
     "map_scrolledwindow", &sw,
     "throbber", &throbber_holder,
     NULL);
  g_free (filename);

  empathy_builder_connect (gui, window,
      "map_view", "destroy", map_view_destroy_cb,
      "map_view", "key-press-event", map_view_key_press_cb,
      "zoom_in", "clicked", map_view_zoom_in_cb,
      "zoom_out", "clicked", map_view_zoom_out_cb,
      "zoom_fit", "clicked", map_view_zoom_fit_cb,
      NULL);

  g_object_unref (gui);

  /* Clear the static pointer to window if the dialog is destroyed */
  g_object_add_weak_pointer (G_OBJECT (window->window), (gpointer *) &window);

  window->contact_list = EMPATHY_CONTACT_LIST (
      empathy_contact_manager_dup_singleton ());

  window->members_changed_id = g_signal_connect (window->contact_list,
      "members-changed", G_CALLBACK (members_changed_cb), window);

  window->throbber = ephy_spinner_new ();
  ephy_spinner_set_size (EPHY_SPINNER (window->throbber),
      GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_show (window->throbber);
  gtk_container_add (GTK_CONTAINER (throbber_holder), window->throbber);

  /* Set up map view */
  embed = gtk_champlain_embed_new ();
  window->map_view = gtk_champlain_embed_get_view (GTK_CHAMPLAIN_EMBED (embed));
  g_object_set (G_OBJECT (window->map_view), "zoom-level", 1,
     "scroll-mode", CHAMPLAIN_SCROLL_MODE_KINETIC, NULL);
  champlain_view_center_on (window->map_view, 36, 0);

  gtk_container_add (GTK_CONTAINER (sw), embed);
  gtk_widget_show_all (embed);

  window->layer = g_object_ref (champlain_layer_new ());
  champlain_view_add_layer (window->map_view, window->layer);

  g_signal_connect (window->map_view, "notify::state",
      G_CALLBACK (map_view_state_changed), window);

  /* Set up contact list. */
  window->markers = g_hash_table_new_full (NULL, NULL,
      (GDestroyNotify) g_object_unref, NULL);

  members = empathy_contact_list_get_members (
      window->contact_list);
  for (l = members; l != NULL; l = g_list_next (l))
    {
      contact_added (window, l->data);
      g_object_unref (l->data);
    }
  g_list_free (members);

  empathy_window_present (GTK_WINDOW (window->window));

  /* Set up time updating loop */
  window->timeout_id = g_timeout_add_seconds (5,
      (GSourceFunc) map_view_tick, window);

  return window->window;
}
