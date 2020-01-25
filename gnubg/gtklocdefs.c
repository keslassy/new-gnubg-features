/*
 * Copyright (C) 2011-2014 Michael Petch <mpetch@capp-sysware.com>
 * Copyright (C) 2011-2016 the AUTHORS
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * $Id$
 */


#include "config.h"
#include "gtkgame.h"
#include "gtklocdefs.h"

#if (USE_GTK)
#include <gtk/gtk.h>

#if ! GTK_CHECK_VERSION(2,22,0)
gint
gdk_visual_get_depth(GdkVisual * visual)
{
    return visual->depth;
}
#endif

#if ! GTK_CHECK_VERSION(2,18,0)
void
gtk_widget_get_allocation(GtkWidget * widget, GtkAllocation * allocation)
{
    *allocation = widget->allocation;
}

void
gtk_widget_set_allocation(GtkWidget * widget, const GtkAllocation * allocation)
{
    widget->allocation = *allocation;
}

void
gtk_cell_renderer_get_alignment(GtkCellRenderer * cell, gfloat * xalign, gfloat * yalign)
{
    *xalign = cell->xalign;
    *yalign = cell->yalign;
}

void
gtk_cell_renderer_set_padding(GtkCellRenderer * cell, gint xpad, gint ypad)
{
    cell->xpad = xpad;
    cell->ypad = ypad;
}
#endif

#if ! GTK_CHECK_VERSION(2,14,0)

GtkWidget *
gtk_dialog_get_action_area(GtkDialog * dialog)
{
    return (dialog->action_area);
}

GtkWidget *
gtk_dialog_get_content_area(GtkDialog * dialog)
{
    return (dialog->vbox);
}

GdkWindow *
gtk_widget_get_window(GtkWidget * widget)
{
    return (widget->window);
}

gdouble
gtk_adjustment_get_upper(GtkAdjustment * adjustment)
{
    return adjustment->upper;
}

void
gtk_adjustment_set_upper(GtkAdjustment * adjustment, gdouble upper)
{
    adjustment->upper = upper;
}

guchar *
gtk_selection_data_get_data(GtkSelectionData * data)
{
    return data->data;
}

#endif

#if ! GTK_CHECK_VERSION(2,12,0)
GtkTooltips *ptt;
#endif

extern GtkWidget *
get_statusbar_label(GtkStatusbar * statusbar)
{
#if GTK_CHECK_VERSION(2,20,0)
    return
        GTK_WIDGET(gtk_container_get_children
                   (GTK_CONTAINER(gtk_statusbar_get_message_area(GTK_STATUSBAR(statusbar))))->data);
#else
    return GTK_WIDGET(statusbar->label);
#endif
}


extern void
toolbar_set_orientation(GtkToolbar * toolbar, GtkOrientation orientation)
{
#if GTK_CHECK_VERSION(2,16,0)
    gtk_orientable_set_orientation(GTK_ORIENTABLE(toolbar), orientation);
#else
    gtk_toolbar_set_orientation(toolbar, orientation);
#endif
}

#ifndef USE_GRESOURCE
#include <string.h>
#include "gnubg-stock-pixbufs.h"

static const struct {
    const char *resource_path;
    gconstpointer inline_data;
} resource_to_inline[] = {
    {"/org/gnubg/16x16/actions/ok_16.png", ok_16},
    {"/org/gnubg/24x24/actions/ok_24.png", ok_24},
    {"/org/gnubg/24x24/actions/anti_clockwise_24.png", anti_clockwise_24},
    {"/org/gnubg/24x24/actions/clockwise_24.png", clockwise_24},
    {"/org/gnubg/16x16/actions/double_16.png", double_16},
    {"/org/gnubg/24x24/actions/double_24.png", double_24},
    {"/org/gnubg/16x16/actions/runit_16.png", runit_16},
    {"/org/gnubg/24x24/actions/runit_24.png", runit_24},
    {"/org/gnubg/16x16/actions/go_next_cmarked_16.png", go_next_cmarked_16},
    {"/org/gnubg/24x24/actions/go_next_cmarked_24.png", go_next_cmarked_24},
    {"/org/gnubg/16x16/actions/go_next_game_16.png", go_next_game_16},
    {"/org/gnubg/24x24/actions/go_next_game_24.png", go_next_game_24},
    {"/org/gnubg/16x16/actions/go_next_16.png", go_next_16},
    {"/org/gnubg/24x24/actions/go_next_24.png", go_next_24},
    {"/org/gnubg/16x16/actions/go_next_marked_16.png", go_next_marked_16},
    {"/org/gnubg/24x24/actions/go_next_marked_24.png", go_next_marked_24},
    {"/org/gnubg/16x16/actions/go_prev_cmarked_16.png", go_prev_cmarked_16},
    {"/org/gnubg/24x24/actions/go_prev_cmarked_24.png", go_prev_cmarked_24},
    {"/org/gnubg/16x16/actions/go_prev_game_16.png", go_prev_game_16},
    {"/org/gnubg/24x24/actions/go_prev_game_24.png", go_prev_game_24},
    {"/org/gnubg/16x16/actions/go_prev_16.png", go_prev_16},
    {"/org/gnubg/24x24/actions/go_prev_24.png", go_prev_24},
    {"/org/gnubg/16x16/actions/go_prev_marked_16.png", go_prev_marked_16},
    {"/org/gnubg/24x24/actions/go_prev_marked_24.png", go_prev_marked_24},
    {"/org/gnubg/16x16/actions/hint_16.png", hint_16},
    {"/org/gnubg/24x24/actions/hint_24.png", hint_24},
    {"/org/gnubg/24x24/actions/new0_24.png", new0_24},
    {"/org/gnubg/24x24/actions/new11_24.png", new11_24},
    {"/org/gnubg/24x24/actions/new13_24.png", new13_24},
    {"/org/gnubg/24x24/actions/new15_24.png", new15_24},
    {"/org/gnubg/24x24/actions/new17_24.png", new17_24},
    {"/org/gnubg/24x24/actions/new1_24.png", new1_24},
    {"/org/gnubg/24x24/actions/new3_24.png", new3_24},
    {"/org/gnubg/24x24/actions/new5_24.png", new5_24},
    {"/org/gnubg/24x24/actions/new7_24.png", new7_24},
    {"/org/gnubg/24x24/actions/new9_24.png", new9_24},
    {"/org/gnubg/16x16/actions/cancel_16.png", cancel_16},
    {"/org/gnubg/24x24/actions/cancel_24.png", cancel_24},
    {"/org/gnubg/16x16/actions/resign_16.png", resign_16},
    {"/org/gnubg/24x24/actions/resign_24.png", resign_24},
    {"/org/gnubg/24x24/actions/resignsb_24.png", resignsb_24},
    {"/org/gnubg/24x24/actions/resignsg_24.png", resignsg_24},
    {"/org/gnubg/24x24/actions/resignsn_24.png", resignsn_24}
};

GdkPixbuf *
gdk_pixbuf_new_from_resource(const char *resource_path, GError **error)
{
    guint i;
    GdkPixbuf *pixbuf = NULL;

    for (i = 0; i < G_N_ELEMENTS(resource_to_inline) && !pixbuf; i++) {
        if (strcmp(resource_path, resource_to_inline[i].resource_path) == 0) {
            const guchar *inline_data = resource_to_inline[i].inline_data;
            pixbuf = gdk_pixbuf_new_from_inline(-1, inline_data, FALSE, error);
        }
    }

    return pixbuf;
}
#endif

#endif
