/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * empathy-status-preset-dialog.c
 *
 * EmpathyStatusPresetDialog - a dialog for adding and removing preset status
 * messages.
 *
 * Copyright (C) 2009 Collabora Ltd.
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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors: Davyd Madeley <davyd.madeley@collabora.co.uk>
 */

#include "config.h"

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include <libmissioncontrol/mc-enum-types.h>

#include <libempathy/empathy-utils.h>
#include <libempathy/empathy-status-presets.h>

#define DEBUG_FLAG EMPATHY_DEBUG_OTHER
#include <libempathy/empathy-debug.h>

#include "empathy-ui-utils.h"
#include "empathy-status-preset-dialog.h"

#define GET_PRIV(obj) EMPATHY_GET_PRIV (obj, EmpathyStatusPresetDialog)

G_DEFINE_TYPE (EmpathyStatusPresetDialog, empathy_status_preset_dialog, GTK_TYPE_DIALOG);

static McPresence states[] = {
	MC_PRESENCE_AVAILABLE,
	MC_PRESENCE_DO_NOT_DISTURB,
	MC_PRESENCE_AWAY
};

typedef struct _EmpathyStatusPresetDialogPriv EmpathyStatusPresetDialogPriv;
struct _EmpathyStatusPresetDialogPriv
{
	GtkWidget *presets_treeview;
	GtkWidget *add_combobox;
	GtkWidget *add_button;

	McPresence selected_state;
	gboolean add_combo_changed;
};

enum
{
	PRESETS_STORE_STATE,
	PRESETS_STORE_ICON_NAME,
	PRESETS_STORE_STATUS,
	PRESETS_STORE_N_COLS
};

enum
{
	ADD_COMBO_STATE,
	ADD_COMBO_ICON_NAME,
	ADD_COMBO_STATUS,
	ADD_COMBO_DEFAULT_TEXT,
	ADD_COMBO_N_COLS
};

static void
empathy_status_preset_dialog_class_init (EmpathyStatusPresetDialogClass *class)
{
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS (class);

	g_type_class_add_private (gobject_class,
			sizeof (EmpathyStatusPresetDialogPriv));
}

static void
status_preset_dialog_setup_presets_update (EmpathyStatusPresetDialog *self)
{
	EmpathyStatusPresetDialogPriv *priv = GET_PRIV (self);
	GtkListStore *store;
	int i;

	store = GTK_LIST_STORE (gtk_tree_view_get_model (
				GTK_TREE_VIEW (priv->presets_treeview)));

	gtk_list_store_clear (store);

	for (i = 0; i < G_N_ELEMENTS (states); i++) {
		GList *presets, *l;
		const char *icon_name;

		presets = empathy_status_presets_get (states[i], -1);
		icon_name = empathy_icon_name_for_presence (states[i]);

		for (l = presets; l; l = l->next) {
			char *preset = (char *) l->data;

			gtk_list_store_insert_with_values (store,
					NULL, -1,
					PRESETS_STORE_STATE, states[i],
					PRESETS_STORE_ICON_NAME, icon_name,
					PRESETS_STORE_STATUS, preset,
					-1);
		}

		g_list_free (presets);
	}
}

static void
status_preset_add_combo_reset (EmpathyStatusPresetDialog *self)
{
	EmpathyStatusPresetDialogPriv *priv = GET_PRIV (self);

	gtk_combo_box_set_active (GTK_COMBO_BOX (priv->add_combobox), 0);
}

static void
status_preset_dialog_setup_add_combobox (EmpathyStatusPresetDialog *self)
{
	EmpathyStatusPresetDialogPriv *priv = GET_PRIV (self);
	GtkWidget *combobox = priv->add_combobox;
	GtkListStore *store;
	GtkCellRenderer *renderer;
	int i;

	store = gtk_list_store_new (ADD_COMBO_N_COLS,
			MC_TYPE_PRESENCE,	/* ADD_COMBO_STATE */
			G_TYPE_STRING,		/* ADD_COMBO_ICON_NAME */
			G_TYPE_STRING,		/* ADD_COMBO_STATUS */
			G_TYPE_STRING);		/* ADD_COMBO_DEFAULT_TEXT */

	gtk_combo_box_set_model (GTK_COMBO_BOX (combobox),
				 GTK_TREE_MODEL (store));
	g_object_unref (store);

	gtk_combo_box_entry_set_text_column (GTK_COMBO_BOX_ENTRY (combobox),
			ADD_COMBO_DEFAULT_TEXT);

	for (i = 0; i < G_N_ELEMENTS (states); i++) {
		gtk_list_store_insert_with_values (store, NULL, -1,
				ADD_COMBO_STATE, states[i],
				ADD_COMBO_ICON_NAME, empathy_icon_name_for_presence (states[i]),
				ADD_COMBO_STATUS, empathy_presence_get_default_message (states[i]),
				ADD_COMBO_DEFAULT_TEXT, _("Enter Custom Message"),
				-1);
	}

	gtk_cell_layout_clear (GTK_CELL_LAYOUT (combobox));

	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, FALSE);
	gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combobox), renderer,
			"icon-name", ADD_COMBO_ICON_NAME);

	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combobox), renderer,
			"text", ADD_COMBO_STATUS);
	g_object_set (renderer,
			"style", PANGO_STYLE_ITALIC,
			"foreground", "Gray", /* FIXME - theme */
			NULL);

	status_preset_add_combo_reset (self);
}

static void
status_preset_dialog_setup_presets_treeview (EmpathyStatusPresetDialog *self)
{
	EmpathyStatusPresetDialogPriv *priv = GET_PRIV (self);
	GtkWidget *treeview = priv->presets_treeview;
	GtkListStore *store;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;

	store = gtk_list_store_new (PRESETS_STORE_N_COLS,
			MC_TYPE_PRESENCE,	/* PRESETS_STORE_STATE */
			G_TYPE_STRING,		/* PRESETS_STORE_ICON_NAME */
			G_TYPE_STRING);		/* PRESETS_STORE_STATUS */

	gtk_tree_view_set_model (GTK_TREE_VIEW (treeview),
				 GTK_TREE_MODEL (store));
	g_object_unref (store);

	status_preset_dialog_setup_presets_update (self);

	column = gtk_tree_view_column_new ();
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_add_attribute (column, renderer,
			"icon-name", PRESETS_STORE_ICON_NAME);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, renderer,
			"text", PRESETS_STORE_STATUS);
}

static void
status_preset_dialog_preset_selection_changed (GtkTreeSelection *selection,
					       GtkWidget *remove_button)
{
	/* update the sensitivity of the Remove button */
	gtk_widget_set_sensitive (remove_button,
			gtk_tree_selection_get_selected (selection, NULL, NULL));
}

static void
status_preset_dialog_preset_remove (GtkButton *button,
				    EmpathyStatusPresetDialog *self)
{
	EmpathyStatusPresetDialogPriv *priv = GET_PRIV (self);
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	McPresence state;
	char *status;

	selection = gtk_tree_view_get_selection (
			GTK_TREE_VIEW (priv->presets_treeview));

	g_return_if_fail (gtk_tree_selection_get_selected (selection,
				&model, &iter));

	gtk_tree_model_get (model, &iter,
			PRESETS_STORE_STATE, &state,
			PRESETS_STORE_STATUS, &status,
			-1);

	DEBUG ("REMOVE PRESET (%i, %s)\n", state, status);
	empathy_status_presets_remove (state, status);

	g_free (status);

	status_preset_dialog_setup_presets_update (self);
}

static void
status_preset_dialog_add_combo_changed (GtkComboBox *combo,
					EmpathyStatusPresetDialog *self)
{
	EmpathyStatusPresetDialogPriv *priv = GET_PRIV (self);
	GtkWidget *entry;
	GtkTreeModel *model;
	GtkTreeIter iter;

	model = gtk_combo_box_get_model (combo);
	entry = gtk_bin_get_child (GTK_BIN (combo));

	if (gtk_combo_box_get_active_iter (combo, &iter)) {
		char *icon_name;
		GdkColor colour;

		gtk_tree_model_get (model, &iter,
				PRESETS_STORE_STATE, &priv->selected_state,
				PRESETS_STORE_ICON_NAME, &icon_name,
				-1);

		gtk_entry_set_icon_from_icon_name (GTK_ENTRY (entry),
				GTK_ENTRY_ICON_PRIMARY,
				icon_name);

		g_free (icon_name);

		gtk_widget_grab_focus (entry);
		gtk_editable_select_region (GTK_EDITABLE (entry), 0, -1);

		priv->add_combo_changed = FALSE;
		gtk_widget_set_sensitive (priv->add_button, FALSE);

		gdk_color_parse ("Gray", &colour); /* FIXME - theme */
		gtk_widget_modify_text (entry, GTK_STATE_NORMAL, &colour);
	} else {
		priv->add_combo_changed = TRUE;
		gtk_widget_set_sensitive (priv->add_button, TRUE);
		gtk_widget_modify_text (entry, GTK_STATE_NORMAL, NULL);
	}
}

static void
status_preset_dialog_add_preset (GtkWidget *widget,
				 EmpathyStatusPresetDialog *self)
{
	EmpathyStatusPresetDialogPriv *priv = GET_PRIV (self);
	GtkWidget *entry;
	const char *status;

	g_return_if_fail (priv->add_combo_changed);

	entry = gtk_bin_get_child (GTK_BIN (priv->add_combobox));
	status = gtk_entry_get_text (GTK_ENTRY (entry));

	DEBUG ("ADD PRESET (%i, %s)\n", priv->selected_state, status);
	empathy_status_presets_set_last (priv->selected_state, status);

	status_preset_dialog_setup_presets_update (self);
	status_preset_add_combo_reset (self);
}

static void
empathy_status_preset_dialog_init (EmpathyStatusPresetDialog *self)
{
	EmpathyStatusPresetDialogPriv *priv = self->priv =
		G_TYPE_INSTANCE_GET_PRIVATE (self,
			EMPATHY_TYPE_STATUS_PRESET_DIALOG,
			EmpathyStatusPresetDialogPriv);
	GtkBuilder *gui;
	GtkWidget *toplevel_vbox, *remove_button;
	char *filename;

	gtk_dialog_set_has_separator (GTK_DIALOG (self), FALSE);
	gtk_dialog_add_button (GTK_DIALOG (self),
			GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);

	filename = empathy_file_lookup ("empathy-status-preset-dialog.ui",
			"libempathy-gtk");
	gui = empathy_builder_get_file (filename,
			"toplevel-vbox", &toplevel_vbox,
			"presets-treeview", &priv->presets_treeview,
			"remove-button", &remove_button,
			"add-combobox", &priv->add_combobox,
			"add-button", &priv->add_button,
			NULL);
	g_free (filename);

	g_signal_connect (gtk_tree_view_get_selection (
				GTK_TREE_VIEW (priv->presets_treeview)),
			"changed",
			G_CALLBACK (status_preset_dialog_preset_selection_changed),
			remove_button);

	g_signal_connect (gtk_bin_get_child (GTK_BIN (priv->add_combobox)),
			"activate",
			G_CALLBACK (status_preset_dialog_add_preset),
			self);

	empathy_builder_connect (gui, self,
			"remove-button", "clicked", status_preset_dialog_preset_remove,
			"add-combobox", "changed", status_preset_dialog_add_combo_changed,
			"add-button", "clicked", status_preset_dialog_add_preset,
			NULL);

	status_preset_dialog_setup_presets_treeview (self);
	status_preset_dialog_setup_add_combobox (self);

	gtk_box_pack_start(GTK_BOX (GTK_DIALOG (self)->vbox), toplevel_vbox,
			TRUE, TRUE, 0);

	g_object_unref (gui);
}

GtkWidget *
empathy_status_preset_dialog_new (GtkWindow *parent)
{
	GtkWidget *self = g_object_new (EMPATHY_TYPE_STATUS_PRESET_DIALOG,
			NULL);

	if (parent) {
		gtk_window_set_transient_for (GTK_WINDOW (self), parent);
	}

	return self;
}
