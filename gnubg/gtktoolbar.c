/*
 * Copyright (C) 2003 Joern Thyssen <jth@gnubg.org>
 * Copyright (C) 2003-2023 the AUTHORS
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
 * $Id: gtktoolbar.c,v 1.88 2023/12/20 14:17:30 plm Exp $
 */

#include "config.h"
#include "gtklocdefs.h"

#include <stdlib.h>

#include <gtk/gtk.h>

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "backgammon.h"
#include "gtktoolbar.h"
#include "gtkboard.h"
#include "gtkgame.h"
#include "gtk-multiview.h"
#include "gtkfile.h"
#include "drawboard.h"
#include "renderprefs.h"
#include "gnubgstock.h"

typedef struct {

    GtkWidget *pwNew;           /* button for "New" */
    GtkWidget *pwOpen;          /* button for "Open" */
    GtkWidget *pwSave;          /* button for "Double" */
    GtkWidget *pwDouble;
    GtkWidget *pwTake;          /* button for "Take" */
    GtkWidget *pwDrop;          /* button for "Drop" */
    GtkWidget *pwResign;        /* button for "Resign" */
    GtkWidget *pwEndGame;       /* button for "play game" */
    GtkWidget *pwHint;          /* button for "Hint" */
    GtkWidget *pwPrevMarked;    /* button for "Previous Marked" */
    GtkWidget *pwPrevCMarked;   /* button for "Previous CMarked" */
    GtkWidget *pwPrev;          /* button for "Previous Roll" */
    GtkWidget *pwPrevGame;      /* button for "Previous Game" */
    GtkWidget *pwNextGame;      /* button for "Next Game" */
    GtkWidget *pwNext;          /* button for "Next Roll" */
    GtkWidget *pwNextCMarked;   /* button for "Next CMarked" */
    GtkWidget *pwNextMarked;    /* button for "Next CMarked" */
    GtkWidget *pwReset;         /* button for "Reset" */
    GtkWidget *pwAnalyzeCurrent;        /* button for "Analyze Current" */
    GtkWidget *pwAnalyzeFile;	/* button for "Analyze File" */
    GtkWidget *pwEdit;          /* button for "Edit" */
    GtkWidget *pwButtonClockwise;       /* button for clockwise */

} toolbarwidget;

#if defined(USE_GTKITEMFACTORY)
static void
ButtonClicked(GtkWidget * UNUSED(pw), char *sz)
{

    UserCommand(sz);
}

static void
ButtonClickedYesNo(GtkWidget * UNUSED(pw), char *sz)
{

    if (ms.fResigned) {
        UserCommand(!strcmp(sz, "yes") ? "accept" : "decline");
        return;
    }

    if (ms.fDoubled) {
        UserCommand(!strcmp(sz, "yes") ? "take" : "drop");
        return;
    }

}

static GtkWidget *
toggle_button_from_images(GtkWidget * pwImageOff, GtkWidget * pwImageOn, char *sz)
{
    GtkWidget **aapw;
    GtkWidget *pwm = gtk_multiview_new();
    GtkWidget *pw = gtk_toggle_button_new();
    GtkWidget *pwvbox = gtk_vbox_new(FALSE, 0);

    aapw = (GtkWidget **) g_malloc(3 * sizeof(GtkWidget *));

    aapw[0] = pwImageOff;
    aapw[1] = pwImageOn;
    aapw[2] = pwm;

    gtk_container_add(GTK_CONTAINER(pwvbox), pwm);

    gtk_container_add(GTK_CONTAINER(pwm), pwImageOff);
    gtk_container_add(GTK_CONTAINER(pwm), pwImageOn);
    gtk_container_add(GTK_CONTAINER(pwvbox), gtk_label_new(sz));
    gtk_container_add(GTK_CONTAINER(pw), pwvbox);

    g_object_set_data_full(G_OBJECT(pw), "toggle_images", aapw, g_free);

    return pw;

}
#endif

#if 0	/* not (no longer? not yet?) used */
extern void
ToolbarSetAnalyzeCurrent(GtkWidget * pwToolbar, const int f)
{

    toolbarwidget *ptw = g_object_get_data(G_OBJECT(pwToolbar),
                                           "toolbarwidget");

    gtk_widget_set_sensitive(ptw->pwAnalyzeCurrent, f);
}

extern void
ToolbarSetAnalyzeFile(GtkWidget * pwToolbar, const int f)
{

    toolbarwidget *ptw = g_object_get_data(G_OBJECT(pwToolbar),
                                           "toolbarwidget");

    gtk_widget_set_sensitive(ptw->pwAnalyzeFile, f);
}
#endif

extern void
ToolbarSetPlaying(GtkWidget * pwToolbar, const int f)
{

    toolbarwidget *ptw = g_object_get_data(G_OBJECT(pwToolbar),
                                           "toolbarwidget");

    gtk_widget_set_sensitive(ptw->pwReset, f);

}

extern void
ToolbarSetClockwise(GtkWidget * pwToolbar, const int f)
{

    toolbarwidget *ptw = g_object_get_data(G_OBJECT(pwToolbar),
                                           "toolbarwidget");

#if !defined(USE_GTKITEMFACTORY)
    gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(ptw->pwButtonClockwise), f);
#else
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ptw->pwButtonClockwise), f);
#endif
}


// extern void ToggleClockwise(void)
// {
//         gchar *sz = g_strdup_printf("set clockwise %s", fClockwise ? "off" : "on");
//         UserCommand(sz);
//         g_free(sz);
//         UserCommand("save settings");
// }


#if !defined(USE_GTKITEMFACTORY)
extern void
ToggleClockwise(GtkToggleAction * action, gpointer UNUSED(user_data))
{
    int f = gtk_toggle_action_get_active(action);
    toolbarwidget *ptw = g_object_get_data(G_OBJECT(pwToolbar), "toolbarwidget");
    GtkWidget *img =
        gtk_image_new_from_stock(f ? GNUBG_STOCK_CLOCKWISE : GNUBG_STOCK_ANTI_CLOCKWISE, GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_show(img);
    gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(ptw->pwButtonClockwise), img);

    if (fClockwise != f) {

        gchar *sz = g_strdup_printf("set clockwise %s", f ? "on" : "off");
        UserCommand(sz);
        g_free(sz);
        UserCommand("save settings");
    }
}

#else
extern void
click_swapdirection(void)
{
    if (!inCallback) {
// #if defined(USE_GTKUIMANAGER)
//         gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(gtk_ui_manager_get_action(puim,
//                                                                                  "/MainMenu/ViewMenu/PlayClockwise")),
//                                      TRUE);
// #else
        toolbarwidget *ptw = g_object_get_data(G_OBJECT(pwToolbar), "toolbarwidget");
        gtk_button_clicked(GTK_BUTTON(ptw->pwButtonClockwise));
// #endif
    }
}

static void
ToolbarToggleClockwise(GtkWidget * pw, toolbarwidget * UNUSED(ptw))
{
    GtkWidget **aapw = (GtkWidget **) g_object_get_data(G_OBJECT(pw), "toggle_images");
    int f = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pw));

    gtk_multiview_set_current(GTK_MULTIVIEW(aapw[2]), aapw[f]);

    inCallback = TRUE;
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget(pif, "/View/Play Clockwise")), f);
    inCallback = FALSE;

    if (fClockwise != f) {
        gchar *sz = g_strdup_printf("set clockwise %s", f ? "on" : "off");
        UserCommand(sz);
        g_free(sz);
        UserCommand("save settings");
    }
}
#endif

static int editing = FALSE;

extern void
click_edit(void)
{
    if (!inCallback) {
#if !defined(USE_GTKITEMFACTORY)
        GtkAction *editstatus = gtk_ui_manager_get_action(puim, "/MainMenu/EditMenu/EditPosition");
        gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(editstatus), !gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(editstatus)));
#else
        toolbarwidget *ptw = g_object_get_data(G_OBJECT(pwToolbar), "toolbarwidget");
        gtk_button_clicked(GTK_BUTTON(ptw->pwEdit));
#endif
    }
}

#if !defined(USE_GTKITEMFACTORY)
extern void
ToggleEdit(GtkToggleAction * action, gpointer UNUSED(user_data))
{
    BoardData *pbd = BOARD(pwBoard)->board_data;

    if (gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action))) {
        if (ms.gs == GAME_NONE)
            edit_new(nDefaultLength);
        /* Undo any partial move that may have been made when 
         * entering edit mode, should be done before editing is true */
        GTKUndo();
        editing = TRUE;
    } else
        editing = FALSE;

    board_edit(pbd);
}
#else
static void
ToolbarToggleEdit(GtkWidget * pw)
{
    BoardData *pbd = BOARD(pwBoard)->board_data;

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pw))) {
        if (ms.gs == GAME_NONE)
            edit_new(nDefaultLength);
        /* Undo any partial move that may have been made when 
         * entering edit mode, should be done before editing is true */
        GTKUndo();
        editing = TRUE;
    } else
        editing = FALSE;

    inCallback = TRUE;
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM
                                   (gtk_item_factory_get_widget(pif, "/Edit/Edit Position")), editing);
    inCallback = FALSE;

    board_edit(pbd);
}
#endif

extern int
ToolbarIsEditing(GtkWidget * UNUSED(pwToolbar))
{
    return editing;
}

extern toolbarcontrol
ToolbarUpdate(GtkWidget * pwToolbar,
              const matchstate * pms, const DiceShown diceShown, const int fComputerTurn, const int fPlaying)
{

    toolbarwidget *ptw = g_object_get_data(G_OBJECT(pwToolbar),
                                           "toolbarwidget");
    toolbarcontrol c;
    int fEdit = ToolbarIsEditing(pwToolbar);

    g_assert(ptw);

    c = C_NONE;

    if (diceShown <= DICE_BELOW_BOARD)
        c = C_ROLLDOUBLE;

    if (pms->fDoubled)
        c = C_TAKEDROP;

    if (pms->fResigned)
        c = C_AGREEDECLINE;

    if (fComputerTurn)
        c = C_PLAY;

    if (fEdit || !fPlaying)
        c = C_NONE;

    /* We want to disable some buttons particularly when we are in the middle of running an analysis
    in the background => we use !fBackgroundAnalysisRunning */

    gtk_widget_set_sensitive(ptw->pwTake, (c == C_TAKEDROP || c == C_AGREEDECLINE) && !fBackgroundAnalysisRunning );
    gtk_widget_set_sensitive(ptw->pwDrop, (c == C_TAKEDROP || c == C_AGREEDECLINE) && !fBackgroundAnalysisRunning);
    gtk_widget_set_sensitive(ptw->pwDouble, ((c == C_TAKEDROP && !pms->nMatchTo) || c == C_ROLLDOUBLE) && !fBackgroundAnalysisRunning);

    gtk_widget_set_sensitive(ptw->pwSave, plGame != NULL && !fBackgroundAnalysisRunning);
    gtk_widget_set_sensitive(ptw->pwResign, fPlaying && !fEdit && !fBackgroundAnalysisRunning);
    gtk_widget_set_sensitive(ptw->pwHint, fPlaying && !fEdit && !fBackgroundAnalysisRunning);
    gtk_widget_set_sensitive(ptw->pwPrevMarked, fPlaying && !fEdit);
    gtk_widget_set_sensitive(ptw->pwPrevCMarked, fPlaying && !fEdit);
    gtk_widget_set_sensitive(ptw->pwPrev, fPlaying && !fEdit);
    gtk_widget_set_sensitive(ptw->pwPrevGame, fPlaying && !fEdit);
    gtk_widget_set_sensitive(ptw->pwNextGame, fPlaying && !fEdit);
    gtk_widget_set_sensitive(ptw->pwNext, fPlaying && !fEdit);
    gtk_widget_set_sensitive(ptw->pwNextCMarked, fPlaying && !fEdit);
    gtk_widget_set_sensitive(ptw->pwNextMarked, fPlaying && !fEdit);
    gtk_widget_set_sensitive(ptw->pwEndGame, fPlaying && !fEdit && !fBackgroundAnalysisRunning);
    gtk_widget_set_sensitive(ptw->pwEdit,  !fBackgroundAnalysisRunning);
    gtk_widget_set_sensitive(ptw->pwNew,  !fBackgroundAnalysisRunning);
    gtk_widget_set_sensitive(ptw->pwOpen,  !fBackgroundAnalysisRunning);
    gtk_widget_set_sensitive(ptw->pwReset,  !fBackgroundAnalysisRunning);
    gtk_widget_set_sensitive(ptw->pwAnalyzeCurrent, plGame != NULL &&  !fBackgroundAnalysisRunning);
    gtk_widget_set_sensitive(ptw->pwAnalyzeFile,  !fBackgroundAnalysisRunning);

    return c;
}

#if defined(USE_GTKITEMFACTORY)
static GtkWidget *
ToolbarAddButton(GtkToolbar * pwToolbar, const char *stockID, const char *label, const char *tooltip,
                 GCallback callback, void *data)
{
    GtkToolItem *but = gtk_tool_button_new_from_stock(stockID);
    gtk_widget_set_tooltip_text(GTK_WIDGET(but), tooltip);
    if (label)
        gtk_tool_button_set_label(GTK_TOOL_BUTTON(but), label);
    gtk_toolbar_insert(GTK_TOOLBAR(pwToolbar), but, -1);

    g_signal_connect(G_OBJECT(but), "clicked", callback, data);

    return GTK_WIDGET(but);
}

// static GtkWidget *
// ToolbarAddButtonFromFile(GtkToolbar * pwToolbar, const char *filename, const char *label, const char *tooltip,
//                  GCallback callback, void *data)
// {
//     GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
//     int width, height;
//     gdk_pixbuf_get_file_info (filename, &width, &height);
//     gtk_icon_theme_add_builtin_icon ("custom_icon", width, pixbuf);
//     g_object_unref (G_OBJECT (pixbuf)); 

// //    GtkToolButton *but = gtk_tool_button_new(



//     GtkToolItem *but = gtk_toggle_tool_button_new();
//     gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON(but), "custom_icon");


//     // GtkToolItem *but = gtk_image_new_from_file(filename);
//     // gtk_widget_set_tooltip_text(GTK_WIDGET(but), tooltip);
//     // if (label)
//     //     gtk_tool_button_set_label(GTK_TOOL_BUTTON(but), label);
//     gtk_toolbar_insert(GTK_TOOLBAR(pwToolbar), but, -1);

//     g_signal_connect(G_OBJECT(but), "clicked", callback, data);

//     return GTK_WIDGET(but);
// }

static GtkWidget *
ToolbarAddWidget(GtkToolbar * pwToolbar, GtkWidget * pWidget, const char *tooltip)
{
    GtkToolItem *ti = gtk_tool_item_new();
    gtk_widget_set_tooltip_text(GTK_WIDGET(ti), tooltip);
    gtk_container_add(GTK_CONTAINER(ti), pWidget);

    gtk_toolbar_insert(GTK_TOOLBAR(pwToolbar), ti, -1);

    return GTK_WIDGET(ti);
}

static void
ToolbarAddSeparator(GtkToolbar * pwToolbar)
{
    GtkToolItem *sep = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(pwToolbar), sep, -1);
}
#endif

extern GtkWidget *
ToolbarNew(void)
{
#if !defined(USE_GTKITEMFACTORY)
    GtkWidget *pwtb;
    toolbarwidget *ptw = (toolbarwidget *) g_malloc(sizeof(toolbarwidget));

    pwtb = gtk_ui_manager_get_widget(puim, "/MainToolBar");
    g_object_set_data_full(G_OBJECT(pwtb), "toolbarwidget", ptw, g_free);
    gtk_toolbar_set_style(GTK_TOOLBAR(pwtb), GTK_TOOLBAR_BOTH);

    ptw->pwNew = gtk_ui_manager_get_widget(puim, "/MainToolBar/New");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwNew), TRUE);
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(ptw->pwNew), NULL);
    ptw->pwOpen = gtk_ui_manager_get_widget(puim, "/MainToolBar/Open");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwOpen), TRUE);
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(ptw->pwOpen), NULL);
    ptw->pwSave = gtk_ui_manager_get_widget(puim, "/MainToolBar/Save");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwSave), TRUE);
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(ptw->pwSave), NULL);
    ptw->pwTake = gtk_ui_manager_get_widget(puim, "/MainToolBar/Accept");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwTake), TRUE);
    ptw->pwDrop = gtk_ui_manager_get_widget(puim, "/MainToolBar/Reject");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwDrop), TRUE);
    ptw->pwDouble = gtk_ui_manager_get_widget(puim, "/MainToolBar/Double");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwDouble), TRUE);
    ptw->pwResign = gtk_ui_manager_get_widget(puim, "/MainToolBar/Resign");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwResign), TRUE);
    ptw->pwEndGame = gtk_ui_manager_get_widget(puim, "/MainToolBar/EndGame");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwEndGame), FALSE);
    ptw->pwReset = gtk_ui_manager_get_widget(puim, "/MainToolBar/Undo");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwReset), TRUE);
    ptw->pwAnalyzeCurrent = gtk_ui_manager_get_widget(puim, "/MainToolBar/Analyse");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwAnalyzeCurrent), TRUE);
    ptw->pwAnalyzeFile = gtk_ui_manager_get_widget(puim, "/MainToolBar/AnalyseFile");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwAnalyzeFile), TRUE);
    ptw->pwHint = gtk_ui_manager_get_widget(puim, "/MainToolBar/Hint");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwHint), TRUE);
    ptw->pwEdit = gtk_ui_manager_get_widget(puim, "/MainToolBar/EditPosition");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwEdit), TRUE);
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(ptw->pwEdit), NULL);
    ptw->pwButtonClockwise = gtk_ui_manager_get_widget(puim, "/MainToolBar/PlayClockwise");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwButtonClockwise), FALSE);
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(ptw->pwButtonClockwise), _("Direction"));
    // ptw->pwAnalyzeCurrent = gtk_ui_manager_get_widget(puim, "/MainToolBar/AnalyzeCurrent");
    // gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwAnalyzeCurrent), TRUE);
    // ptw->pwAnalyzeFile = gtk_ui_manager_get_widget(puim, "/MainToolBar/AnalyzeFile");
    // gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwAnalyzeFile), TRUE);
    ptw->pwPrevCMarked = gtk_ui_manager_get_widget(puim, "/MainToolBar/PreviousCMarkedMove");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwPrevCMarked), FALSE);
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(ptw->pwPrevCMarked), "");
    ptw->pwPrevMarked = gtk_ui_manager_get_widget(puim, "/MainToolBar/PreviousMarkedMove");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwPrevMarked), FALSE);
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(ptw->pwPrevMarked), "");
    ptw->pwPrevGame = gtk_ui_manager_get_widget(puim, "/MainToolBar/PreviousGame");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwPrevGame), FALSE);
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(ptw->pwPrevGame), "");
    ptw->pwPrev = gtk_ui_manager_get_widget(puim, "/MainToolBar/PreviousRoll");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwPrev), FALSE);
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(ptw->pwPrev), "");
    ptw->pwNextCMarked = gtk_ui_manager_get_widget(puim, "/MainToolBar/NextCMarkedMove");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwNextCMarked), FALSE);
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(ptw->pwNextCMarked), "");
    ptw->pwNextMarked = gtk_ui_manager_get_widget(puim, "/MainToolBar/NextMarkedMove");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwNextMarked), FALSE);
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(ptw->pwNextMarked), "");
    ptw->pwNextGame = gtk_ui_manager_get_widget(puim, "/MainToolBar/NextGame");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwNextGame), FALSE);
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(ptw->pwNextGame), "");
    ptw->pwNext = gtk_ui_manager_get_widget(puim, "/MainToolBar/NextRoll");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwNext), FALSE);
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(ptw->pwNext), "");

    return pwtb;
#else
    GtkWidget *vbox_toolbar;
    GtkToolItem *ti;
    GtkWidget *pwtb, *pwvbox;

    toolbarwidget *ptw;

    /* 
     * Create toolbar 
     */

    ptw = (toolbarwidget *) g_malloc(sizeof(toolbarwidget));

    vbox_toolbar = gtk_vbox_new(FALSE, 0);

    g_object_set_data_full(G_OBJECT(vbox_toolbar), "toolbarwidget", ptw, g_free);
    pwtb = gtk_toolbar_new();
    gtk_toolbar_set_icon_size(GTK_TOOLBAR(pwtb), GTK_ICON_SIZE_LARGE_TOOLBAR);
    gtk_orientable_set_orientation(GTK_ORIENTABLE(pwtb), GTK_ORIENTATION_HORIZONTAL);
    gtk_toolbar_set_style(GTK_TOOLBAR(pwtb), GTK_TOOLBAR_BOTH);
        // gtk_toolbar_set_show_arrow(GTK_TOOLBAR(pwtb), FALSE);

    gtk_box_pack_start(GTK_BOX(vbox_toolbar), pwtb, FALSE, FALSE, 0);

    gtk_toolbar_set_tooltips(GTK_TOOLBAR(pwtb), TRUE);

    /* New button */
    ptw->pwNew =
        ToolbarAddButton(GTK_TOOLBAR(pwtb), GTK_STOCK_NEW, NULL, _("Start new game, match, session or position"),
                         G_CALLBACK(GTKNew), NULL);

    /* Open button */
    ptw->pwOpen =
        ToolbarAddButton(GTK_TOOLBAR(pwtb), GTK_STOCK_OPEN, NULL, _("Open game, match, session or position"),
                         G_CALLBACK(GTKOpen), NULL);

    /* Save button */
    ptw->pwSave =
        ToolbarAddButton(GTK_TOOLBAR(pwtb), GTK_STOCK_SAVE, NULL, _("Save match, session, game or position"),
                         G_CALLBACK(GTKSave), NULL);

    ToolbarAddSeparator(GTK_TOOLBAR(pwtb));

    /* Take/accept button */
    ptw->pwTake =
        ToolbarAddButton(GTK_TOOLBAR(pwtb), GNUBG_STOCK_ACCEPT, NULL,
                         _("Take the offered cube or accept the offered resignation"), G_CALLBACK(ButtonClickedYesNo),
                         "yes");

    /* drop button */
    ptw->pwDrop =
        ToolbarAddButton(GTK_TOOLBAR(pwtb), GNUBG_STOCK_REJECT, NULL,
                         _("Drop the offered cube or decline the offered resignation"), G_CALLBACK(ButtonClickedYesNo),
                         "no");

    /* Double button */
    ptw->pwDouble =
        ToolbarAddButton(GTK_TOOLBAR(pwtb), GNUBG_STOCK_DOUBLE, NULL, _("Double or redouble(beaver)"),
                         G_CALLBACK(ButtonClicked), "double");

    /* Resign button */
    ptw->pwResign =
        ToolbarAddButton(GTK_TOOLBAR(pwtb), GNUBG_STOCK_RESIGN, NULL, _("Resign the current game"),
                         G_CALLBACK(GTKResign), NULL);

    /* IK: I removed to make space for analysis buttons, but put back. I think it can be removed: 
    (1) it's a very seldom used button, 
    (2) it's on the menu as well, 
    (3) it also has a shortcut 
    */
    /* End game button */
    ptw->pwEndGame =
        ToolbarAddButton(GTK_TOOLBAR(pwtb), GNUBG_STOCK_END_GAME, NULL, _("Let the computer end the game"),
                         G_CALLBACK(ButtonClicked), "end game");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwEndGame), FALSE);

    ToolbarAddSeparator(GTK_TOOLBAR(pwtb));

    /* reset button */
    ptw->pwReset =
        ToolbarAddButton(GTK_TOOLBAR(pwtb), GTK_STOCK_UNDO, NULL, _("Undo moves"), G_CALLBACK(GTKUndo), NULL);

    /* Hint button */
    ptw->pwHint =
        ToolbarAddButton(GTK_TOOLBAR(pwtb), GNUBG_STOCK_HINT, NULL, _("Show the best moves or cube action"),
                         G_CALLBACK(ButtonClicked), "hint");

    /* edit button */
    ptw->pwEdit = gtk_toggle_button_new();
    pwvbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(pwvbox), gtk_image_new_from_stock(GTK_STOCK_EDIT, GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_container_add(GTK_CONTAINER(pwvbox), gtk_label_new(_("Edit")));
    gtk_container_add(GTK_CONTAINER(ptw->pwEdit), pwvbox);
    g_signal_connect(G_OBJECT(ptw->pwEdit), "toggled", G_CALLBACK(ToolbarToggleEdit), NULL);
    ti = GTK_TOOL_ITEM(ToolbarAddWidget(GTK_TOOLBAR(pwtb), ptw->pwEdit, _("Toggle Edit Mode")));
    gtk_tool_item_set_homogeneous(ti, TRUE);

    /* removed to make space for analysis buttons, but put back. I think it can be removed:  
    (1) it's a seldom used button, 
    (2) it's on the menu as well, 
    (3) it also has a shortcut 
    */
    // /* direction of play */
    ptw->pwButtonClockwise =
        toggle_button_from_images(gtk_image_new_from_stock(GNUBG_STOCK_ANTI_CLOCKWISE, GTK_ICON_SIZE_LARGE_TOOLBAR),
                                  gtk_image_new_from_stock(GNUBG_STOCK_CLOCKWISE, GTK_ICON_SIZE_LARGE_TOOLBAR),
                                  _("Direction"));
    g_signal_connect(G_OBJECT(ptw->pwButtonClockwise), "toggled", G_CALLBACK(ToolbarToggleClockwise), ptw);

    ToolbarAddWidget(GTK_TOOLBAR(pwtb), ptw->pwButtonClockwise, _("Reverse direction of play"));

    ToolbarAddSeparator(GTK_TOOLBAR(pwtb));
    
    /* Analyze current match button 
    ... switching to the British "analyse" even though "analyze" is much more common in the world...
    */
    ptw->pwAnalyzeCurrent =
        ToolbarAddButton(GTK_TOOLBAR(pwtb), GTK_STOCK_EXECUTE, _("Analyse"),
        _("Analyse current match (set default behaviour in Settings -> Analysis)"),
        G_CALLBACK(GTKAnalyzeCurrent), NULL);

    /* Analyze file button 
    ... switching to the British "analyse" even though "analyze" is much more common in the world...
    */
    ptw->pwAnalyzeFile =
        ToolbarAddButton(GTK_TOOLBAR(pwtb), GTK_STOCK_DIRECTORY, _("Analyse File"),
        _("Analyze match from file (set default behaviour in Settings -> Analysis -> Analysis Buttons:"
        "'batch analysis', 'single-file analysis', or 'smart analysis')"), 
        G_CALLBACK(GTKAnalyzeFile), NULL);

    ti = gtk_separator_tool_item_new();
    gtk_tool_item_set_expand(GTK_TOOL_ITEM(ti), TRUE);
    gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(ti), FALSE);
    gtk_toolbar_insert(GTK_TOOLBAR(pwtb), ti, -1);

    ptw->pwPrevCMarked =
        ToolbarAddButton(GTK_TOOLBAR(pwtb), GNUBG_STOCK_GO_PREV_MARKED, "", _("Go to Previous Marked"),
                         G_CALLBACK(ButtonClicked), "previous marked");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwPrevCMarked), FALSE);
    ptw->pwPrevMarked =
        ToolbarAddButton(GTK_TOOLBAR(pwtb), GNUBG_STOCK_GO_PREV_CMARKED, "", _("Go to Previous CMarked"),
                         G_CALLBACK(ButtonClicked), "previous cmarked");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwPrevMarked), FALSE);
    ptw->pwPrev =
        ToolbarAddButton(GTK_TOOLBAR(pwtb), GNUBG_STOCK_GO_PREV, "", _("Go to Previous Roll"),
                         G_CALLBACK(ButtonClicked), "previous roll");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwPrev), FALSE);
    ptw->pwPrevGame =
        ToolbarAddButton(GTK_TOOLBAR(pwtb), GNUBG_STOCK_GO_PREV_GAME, "", _("Go to Previous Game"),
                         G_CALLBACK(ButtonClicked), "previous game");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwPrevGame), FALSE);
    ptw->pwNextGame =
        ToolbarAddButton(GTK_TOOLBAR(pwtb), GNUBG_STOCK_GO_NEXT_GAME, "", _("Go to Next Game"),
                         G_CALLBACK(ButtonClicked), "next game");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwNextGame), FALSE);
    ptw->pwNext =
        ToolbarAddButton(GTK_TOOLBAR(pwtb), GNUBG_STOCK_GO_NEXT, "", _("Go to Next Roll"),
                         G_CALLBACK(ButtonClicked), "next roll");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwNext), FALSE);
    ptw->pwNextCMarked =
        ToolbarAddButton(GTK_TOOLBAR(pwtb), GNUBG_STOCK_GO_NEXT_CMARKED, "", _("Go to Next CMarked"),
                         G_CALLBACK(ButtonClicked), "next cmarked");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwNextCMarked), FALSE);
    ptw->pwNextMarked =
        ToolbarAddButton(GTK_TOOLBAR(pwtb), GNUBG_STOCK_GO_NEXT_MARKED, "", _("Go to Next Marked"),
                         G_CALLBACK(ButtonClicked), "next marked");
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(ptw->pwNextMarked), FALSE);

    return vbox_toolbar;
#endif
}

#if defined(USE_GTKITEMFACTORY)
static GtkWidget *
firstChild(GtkWidget * widget)
{
    GList *list = gtk_container_get_children(GTK_CONTAINER(widget));
    GtkWidget *child = g_list_nth_data(list, 0);
    g_list_free(list);
    return child;
}

static void
SetToolbarItemStyle(gpointer data, gpointer user_data)
{
    GtkWidget *widget = GTK_WIDGET(data);
    int style = GPOINTER_TO_INT(user_data);
    /* Find icon and text widgets from parent object */
    GList *buttonParts;
    GtkWidget *icon, *text;
    GtkWidget *buttonParent;

    buttonParent = firstChild(widget);
    buttonParts = gtk_container_get_children(GTK_CONTAINER(buttonParent));
    icon = g_list_nth_data(buttonParts, 0);
    text = g_list_nth_data(buttonParts, 1);
    g_list_free(buttonParts);

    if (!icon || !text)
        return;                 /* Didn't find them */

    /* Hide correct parts dependent on style value */
    if (style == GTK_TOOLBAR_ICONS || style == GTK_TOOLBAR_BOTH)
        gtk_widget_show(icon);
    else
        gtk_widget_hide(icon);
    if (style == GTK_TOOLBAR_TEXT || style == GTK_TOOLBAR_BOTH)
        gtk_widget_show(text);
    else
        gtk_widget_hide(text);
}
#endif

extern void
SetToolbarStyle(int value)
{
#if !defined(USE_GTKITEMFACTORY)
    toolbarwidget *ptw = g_object_get_data(G_OBJECT(pwToolbar), "toolbarwidget");
    GtkWidget *img = gtk_image_new_from_stock(fClockwise ? GNUBG_STOCK_CLOCKWISE : GNUBG_STOCK_ANTI_CLOCKWISE,
                                              GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_show(img);
    gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(ptw->pwButtonClockwise), img);
#endif

    if (value != nToolbarStyle) {
#if !defined(USE_GTKITEMFACTORY)
        gtk_toolbar_set_style(GTK_TOOLBAR(pwToolbar), (GtkToolbarStyle) value);
#else
        toolbarwidget *ptw = g_object_get_data(G_OBJECT(pwToolbar), "toolbarwidget");

        /* Set last 2 toolbar items separately - as special widgets not covered in gtk_toolbar_set_style */
        SetToolbarItemStyle(ptw->pwEdit, GINT_TO_POINTER(value));
        SetToolbarItemStyle(ptw->pwButtonClockwise, GINT_TO_POINTER(value));
        gtk_toolbar_set_style(GTK_TOOLBAR(firstChild(pwToolbar)), (GtkToolbarStyle) value);
#endif
        /* Resize handle box parent */
        gtk_widget_queue_resize(pwToolbar);
        nToolbarStyle = value;
#if defined(USE_GTKITEMFACTORY)
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM
                                       (gtk_item_factory_get_widget_by_action(pif, value + TOOLBAR_ACTION_OFFSET)),
                                       TRUE);
#endif
        UserCommand("save settings");
    }
}
