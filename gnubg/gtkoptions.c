/*
 * Copyright (C) 2003-2004 Joern Thyssen <jth@gnubg.org>
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
 * $Id: gtkoptions.c,v 1.143 2023/10/14 19:05:13 plm Exp $
 */

#include "config.h"
#include "gtklocdefs.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>

#include <gtk/gtk.h>

#include "backgammon.h"
#include "eval.h"
#include "dice.h"
#include "gtkgame.h"
#include "gtkfile.h"
#include "sound.h"
#include "drawboard.h"
#include "matchequity.h"
#include "format.h"
#include "gtkboard.h"
#include "renderprefs.h"
#include "gtkwindows.h"
#include "openurl.h"
#if defined(USE_BOARD3D)
#include "inc3d.h"
#endif
#include "multithread.h"
#include "gtkoptions.h"
#include "gtkrelational.h"

typedef struct {
    GtkWidget *pwNoteBook;
    GtkWidget *pwAutoBearoff;
    GtkWidget *pwAutoCrawford;
    GtkWidget *pwAutoEndGame;
    GtkWidget *pwAutoGame;
    GtkWidget *pwAutoMove;
    GtkWidget *pwAutoRoll;
    GtkWidget *pwTutor;
    GtkWidget *pwTutorCube;
    GtkWidget *pwTutorChequer;
    GtkWidget *pwTutorSkill;
    GtkWidget *pwQuiz;
    GtkWidget *pwQuizAutoAdd;
    GtkWidget *pwQuizOnePlayer;
    GtkWidget *pwQuizAtMoney;
    GtkWidget *pwQuizSkill;
    GtkAdjustment *padjCubeBeaver;
    GtkAdjustment *padjCubeAutomatic;
    GtkAdjustment *padjLength;
    GtkWidget *pwCubeUsecube;
    GtkWidget *pwCubeJacoby;
    GtkWidget *pwCubeInvert;
    // GtkWidget *pwKeyName;
    GtkWidget *pwGameClockwise;
    GtkWidget *apwVariations[NUM_VARIATIONS];
    GtkWidget *pwOutputMWC;
    GtkWidget *pwOutputGWC;
    GtkWidget *pwOutputMWCpst;
    GtkWidget *pwConfStart;
    GtkWidget *pwConfOverwrite;
    GtkWidget *apwDice[NUM_RNGS];
    GtkWidget *pwRngComboBox;
    GtkWidget *pwBeavers;
    GtkWidget *pwBeaversLabel;
    GtkWidget *pwAutomatic;
    GtkWidget *pwAutomaticLabel;
    GtkWidget *pwMETFrame;
    GtkWidget *pwLoadMET;
    GtkWidget *pwSeed;
    GtkWidget *pwRecordGames;
    GtkWidget *pwDisplay;
    GtkAdjustment *padjCache;
    GtkAdjustment *padjDelay;
    GtkAdjustment *padjSeed;
    GtkAdjustment *padjThreads;
    GtkWidget *pwAutoSaveTime;
    GtkWidget *pwAutoSaveRollout;
    GtkWidget *pwAutoSaveAnalysis;
    GtkWidget *pwAutoSaveConfirmDelete;
    GtkWidget *pwCheckUpdates;
    GtkWidget *pwIllegal;
    GtkWidget *pwUseDiceIcon;
    GtkWidget *pwShowIDs;
    GtkWidget *pwShowPips;
    GtkWidget *pwShowEPCs;
    GtkWidget *pwShowWastage;
    GtkWidget *pwAnimateNone;
    GtkWidget *pwAnimateBlink;
    GtkWidget *pwAnimateSlide;
    GtkWidget *pwHigherDieFirst;
    GtkWidget *pwGrayEdit;
    GtkWidget *pwSetWindowPos;
    GtkWidget *pwDragTargetHelp;
    GtkAdjustment *padjSpeed;
    GtkWidget *pwCheat;
    GtkWidget *pwCheatRollBox;
    GtkWidget *apwCheatRoll[2];
    GtkWidget *pwGotoFirstGame;
    GtkWidget *pwGameListStyles;
    GtkWidget *pwMarkedSamePlayer;
    GtkWidget *pwDefaultSGFFolder;
    GtkWidget *pwDefaultImportFolder;
    GtkWidget *pwDefaultExportFolder;
    GtkWidget *pwWebBrowser;
    GtkAdjustment *padjDigits;
    GtkWidget *pwDigits;
    int fChanged;
} optionswidget;

typedef struct {
    char *Path;
} SoundDetail;

static SoundDetail soundDetails[NUM_SOUNDS];

static GtkWidget *soundFrame;
static GtkWidget *soundEnabled;
static GtkWidget *soundPath;
static GtkWidget *soundPathButton;
static GtkWidget *soundPlayButton;
static GtkWidget *soundAllDefaultButton;
static GtkWidget *soundDefaultButton;
static GtkWidget *soundBeepIllegal;
static GtkWidget *soundsEnabled;
static GtkWidget *soundList;
static GtkWidget *pwSoundCommand;
static gnubgsound selSound;
static int SoundSkipUpdate;
static int relPage, relPageActivated;

static GtkTreeIter selected_iter;
static GtkListStore *nameStore;

static void
AddKeyNameClicked(GtkButton * UNUSED(button), gpointer treeview)
{
    GtkTreeIter iter;
    char *keyName = GTKGetInput(_("Add key name"), _("Key Player Name:"), NULL);
    if (keyName) {
        // g_message("message=%s",keyName);
        if (AddKeyName(keyName)) {
            gtk_list_store_append(GTK_LIST_STORE(nameStore), &iter);
            gtk_list_store_set(GTK_LIST_STORE(nameStore), &iter, 0, keyName, -1);
            gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), &iter);
            selected_iter = iter;
        } else {
            outputerrf(_("there was a problem adding this key name"));
        }
        g_free(keyName);
    }
}

static char *
GetSelectedName(GtkTreeView * treeview)
{
    GtkTreeModel *model;
    char *keyName = NULL;
    GtkTreeSelection *sel = gtk_tree_view_get_selection(treeview);
    if (gtk_tree_selection_count_selected_rows(sel) != 1)
        return NULL;

    /* Sets selected_iter to the currently selected node: */
    gtk_tree_selection_get_selected(sel, &model, &selected_iter);

    /* Gets the value of the char* cell (in column 0) in the row 
        referenced by selected_iter */
    gtk_tree_model_get(model, &selected_iter, 0, &keyName, -1);
    // g_message("GetSelectedName gives keyName=%s",keyName);
    return keyName;
}

static void
DeleteKeyNameClicked(GtkButton * UNUSED(button), gpointer treeview)
{
    char *keyName = GetSelectedName(GTK_TREE_VIEW(treeview));
    if (keyName) {
        if (DeleteKeyName(keyName)) {
            gtk_list_store_remove(GTK_LIST_STORE(nameStore), &selected_iter);
            // DisplayKeyNames();
        } else {
            outputerrf(_("there was a problem deleting this key name"));
        }
    }
}


extern void
GTKCommandEditKeyNames(GtkWidget * UNUSED(pw), GtkWidget * UNUSED(pwParent))
{
    GtkWidget *pwDialog;
    GtkWidget *pwMainHBox;
    GtkWidget *pwVBox;
    GtkWidget *hb1;
    GtkWidget *pwScrolled;
    GtkWidget *treeview;
    GtkWidget *addButton;
    GtkWidget *delButton;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    // GtkListStore *store;
    GtkTreeIter iter;
 
    pwScrolled = gtk_scrolled_window_new(NULL, NULL);

    pwDialog = GTKCreateDialog(_("Edit key player names"), DT_INFO, NULL, DIALOG_FLAG_MODAL | DIALOG_FLAG_CLOSEBUTTON, NULL, NULL);
    // pwDialog = GTKCreateDialog(_("Edit key player names"), DT_INFO, NULL, DIALOG_FLAG_MODAL, NULL, NULL);

#if GTK_CHECK_VERSION(3,0,0)
    pwMainHBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    pwMainHBox = gtk_hbox_new(FALSE, 0);
#endif

    gtk_container_add(GTK_CONTAINER(DialogArea(pwDialog, DA_MAIN)), pwMainHBox);

#if GTK_CHECK_VERSION(3,0,0)
    pwVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
    pwVBox = gtk_vbox_new(FALSE, 0);
#endif
    gtk_box_pack_start(GTK_BOX(pwMainHBox), pwVBox, FALSE, FALSE, 0);

    //AddTitle(pwVBox, _("?"));

    /* create list store */
    nameStore = gtk_list_store_new(1, G_TYPE_STRING);


    for (int i = 0; i < keyNamesFirstEmpty; i++) {
        gtk_list_store_append(nameStore, &iter);
        gtk_list_store_set(nameStore, &iter, 0, keyNames[i], -1);
        // g_message("in DisplayKeyNames: %d->%s", i,keyNames[i]);
    }

    /* create tree view */
    treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(nameStore));
    // g_object_unref(nameStore);
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("Key Player Names"), renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    gtk_container_set_border_width(GTK_CONTAINER(pwVBox), 8);
    gtk_box_pack_start(GTK_BOX(pwVBox), pwScrolled, TRUE, TRUE, 0);
    gtk_widget_set_size_request(pwScrolled, 100, 200);	//-1);
#if GTK_CHECK_VERSION(3, 8, 0)
    gtk_container_add(GTK_CONTAINER(pwScrolled), treeview);
#else
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(pwScrolled), treeview);
#endif
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(pwScrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

#if GTK_CHECK_VERSION(3,0,0)
    hb1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    hb1 = gtk_hbox_new(FALSE, 0);
#endif
    gtk_box_pack_start(GTK_BOX(pwVBox), hb1, FALSE, FALSE, 0);
    addButton = gtk_button_new_with_label(_("Add name"));
    g_signal_connect(addButton, "clicked", G_CALLBACK(AddKeyNameClicked), treeview);
    gtk_box_pack_start(GTK_BOX(hb1), addButton, FALSE, FALSE, 0);
    delButton = gtk_button_new_with_label(_("Delete name"));
    g_signal_connect(delButton, "clicked", G_CALLBACK(DeleteKeyNameClicked), treeview);
    gtk_box_pack_start(GTK_BOX(hb1), delButton, FALSE, FALSE, 4);

    GTKRunDialog(pwDialog);
}


static void
SeedChanged(GtkWidget * UNUSED(pw), int *pf)
{
    *pf = 1;
}

static void
UseCubeToggled(GtkWidget * UNUSED(pw), optionswidget * pow)
{
    gint n = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pow->pwCubeUsecube));

    gtk_widget_set_sensitive(pow->pwCubeJacoby, n);
    gtk_widget_set_sensitive(pow->pwBeavers, n);
    gtk_widget_set_sensitive(pow->pwAutomatic, n);
    gtk_widget_set_sensitive(pow->pwBeaversLabel, n);
    gtk_widget_set_sensitive(pow->pwAutomaticLabel, n);
    gtk_widget_set_sensitive(pow->pwAutoCrawford, n);
}

static void
DiceToggled(GtkWidget * UNUSED(pw), optionswidget * pow)
{
    gint n = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pow->apwDice[0]));
    gint m = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pow->pwCheat));
    gtk_widget_set_sensitive(pow->pwRngComboBox, n);
    gtk_widget_set_sensitive(pow->pwSeed, n);
    gtk_widget_set_sensitive(pow->pwCheatRollBox, m);
}

static void
TutorToggled(GtkWidget * UNUSED(pw), optionswidget * pow)
{
    gint n = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pow->pwTutor));

    gtk_widget_set_sensitive(pow->pwTutorCube, n);
    gtk_widget_set_sensitive(pow->pwTutorChequer, n);
    gtk_widget_set_sensitive(pow->pwTutorSkill, n);
}

static void QuizToggled(GtkWidget * UNUSED(pw), optionswidget * pow)
{
    gint n = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pow->pwQuiz));
    gint n2 = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pow->pwQuizAutoAdd));

    gtk_widget_set_sensitive(pow->pwQuizAutoAdd, n);
    gtk_widget_set_sensitive(pow->pwQuizSkill, n&&n2);
    gtk_widget_set_sensitive(pow->pwQuizOnePlayer, n&&n2);
    gtk_widget_set_sensitive(pow->pwQuizAtMoney, n);

    // gtk_widget_set_sensitive(pow->pwQuizAutoAdd, fUseQuiz);
    // gtk_widget_set_sensitive(pow->pwQuizSkill, fUseQuiz && fQuizAutoAdd);
    // gtk_widget_set_sensitive(pow->pwQuizOnePlayer, fUseQuiz && fQuizAutoAdd);
}

static void
ToggleAnimation(GtkWidget * pw, GtkWidget * pwSpeed)
{
    gtk_widget_set_sensitive(pwSpeed, !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pw)));
}

static char *aszTutor[] = {
    N_("Doubtful"), N_("Bad"), N_("Very bad"), NULL
};
static char *aszQuiz[] =  {
    N_("Doubtful"), N_("Bad"), N_("Very bad"), NULL
};

static void
SoundDefaultClicked(GtkWidget * UNUSED(widget), gpointer UNUSED(userdata))
{
    char *defaultSound = GetDefaultSoundFile(selSound);
    SoundSkipUpdate = TRUE;
    gtk_entry_set_text(GTK_ENTRY(soundPath), defaultSound);
    g_free(soundDetails[selSound].Path);
    soundDetails[selSound].Path = defaultSound;
}

static void
SoundAllDefaultClicked(GtkWidget * UNUSED(widget), gpointer UNUSED(userdata))
{
    gnubgsound i;

    for (i = (gnubgsound) 0; i < NUM_SOUNDS; i++) {
        g_free(soundDetails[i].Path);
        soundDetails[i].Path = GetDefaultSoundFile(i);
    }

    SoundDefaultClicked(NULL, NULL);
}


static void
SoundEnabledClicked(GtkWidget * UNUSED(widget), gpointer UNUSED(userdata))
{
    int enabled;
    if (!gtk_widget_get_realized(soundEnabled) || !gtk_widget_get_sensitive(soundEnabled))
        return;
    enabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(soundEnabled));
    gtk_widget_set_sensitive(soundPath, enabled);
    gtk_widget_set_sensitive(soundPathButton, enabled);
    gtk_widget_set_sensitive(soundPlayButton, enabled);
    gtk_widget_set_sensitive(soundDefaultButton, enabled);
    if (!enabled) {
        g_free(soundDetails[selSound].Path);
        soundDetails[selSound].Path = g_strdup("");
    } else if (!*soundDetails[selSound].Path)
        SoundDefaultClicked(0, 0);
}

static void
SoundGrabFocus(GtkWidget * UNUSED(pw), void *UNUSED(dummy))
{
    gtk_widget_grab_focus(soundList);
    SoundEnabledClicked(0, 0);
}

static void
SoundToggled(GtkWidget * UNUSED(pw), optionswidget * UNUSED(pow))
{
    int enabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(soundsEnabled));
    gtk_widget_set_sensitive(soundFrame, enabled);
    gtk_widget_set_sensitive(soundList, enabled);
    gtk_widget_set_sensitive(soundEnabled, enabled);
    gtk_widget_set_sensitive(soundPath, enabled);
    gtk_widget_set_sensitive(soundPathButton, enabled);
    gtk_widget_set_sensitive(soundPlayButton, enabled);
    gtk_widget_set_sensitive(soundDefaultButton, enabled);
    if (enabled)
        SoundGrabFocus(0, 0);
}

static void
SoundSelected(GtkTreeView * treeview, gpointer UNUSED(userdata))
{
    GtkTreePath *path;
    gtk_tree_view_get_cursor(GTK_TREE_VIEW(treeview), &path, NULL);
    selSound = (gnubgsound) gtk_tree_path_get_indices(path)[0];

    gtk_frame_set_label(GTK_FRAME(soundFrame), Q_(sound_description[selSound]));
    SoundSkipUpdate = TRUE;
    gtk_entry_set_text(GTK_ENTRY(soundPath), soundDetails[selSound].Path);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(soundEnabled), (*soundDetails[selSound].Path) ? TRUE : FALSE);
}

static void
SoundTidy(GtkWidget * UNUSED(pw), void *UNUSED(dummy))
{
    unsigned int i;

    for (i = 0; i < NUM_SOUNDS; i++) {  /* Tidy allocated memory */
        g_free(soundDetails[i].Path);
    }
}

static void
PathChanged(GtkWidget * widget, gpointer UNUSED(userdata))
{
    if (!SoundSkipUpdate) {
        g_free(soundDetails[selSound].Path);
        soundDetails[selSound].Path = gtk_editable_get_chars(GTK_EDITABLE(widget), 0, -1);
    } else
        SoundSkipUpdate = FALSE;
}

static void
SoundChangePathClicked(GtkWidget * UNUSED(widget), gpointer UNUSED(userdata))
{
    static char *lastSoundFolder = NULL;
    char *filename = GTKFileSelect(_("Select soundfile"), "*.wav", lastSoundFolder, NULL, GTK_FILE_CHOOSER_ACTION_OPEN);

    if (filename) {
        lastSoundFolder = g_path_get_dirname(filename);
        SoundSkipUpdate = TRUE;
        gtk_entry_set_text(GTK_ENTRY(soundPath), filename);
        g_free(soundDetails[selSound].Path);
        soundDetails[selSound].Path = filename;
    }
}

static void
SoundPlayClicked(GtkWidget * UNUSED(widget), gpointer UNUSED(userdata))
{
    playSoundFile(soundDetails[selSound].Path, TRUE);
}

static gchar *
CacheSizeString(GtkScale * UNUSED(scale), gdouble value)
{
    return g_strdup_printf("%iMB", GetCacheMB((int) value));
}

static void
append_game_options(optionswidget * pow)
{
    GtkWidget *pwvbox;
    GtkWidget *pwf;
    GtkWidget *pwb;
#if !GTK_CHECK_VERSION(3,0,0)
    GtkWidget *pwp;
#endif
    unsigned int i;
    static char *aszVariationsTooltips[NUM_VARIATIONS] = {
        N_("Use standard backgammon starting position"),
        N_("Use Nick \"Nack\" Ballard's Nackgammon starting position with " "standard rules."),
        N_("Play 1-chequer hypergammon (i.e., gammon and backgammons possible)"),
        N_("Play 2-chequer hypergammon (i.e., gammon and backgammons possible)"),
        N_("Play 3-chequer hypergammon (i.e., gammon and backgammons possible)")
    };

    /* Game options */

#if GTK_CHECK_VERSION(3,0,0)
    pwvbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(pwvbox, GTK_ALIGN_START);
    gtk_widget_set_valign(pwvbox, GTK_ALIGN_START);
    gtk_container_set_border_width(GTK_CONTAINER(pwvbox), 4);
    gtk_notebook_append_page(GTK_NOTEBOOK(pow->pwNoteBook), pwvbox, gtk_label_new(_("Game")));
#else
    pwvbox = gtk_vbox_new(FALSE, 0);
    pwp = gtk_alignment_new(0, 0, 0, 0);
    gtk_container_set_border_width(GTK_CONTAINER(pwp), 4);
    gtk_notebook_append_page(GTK_NOTEBOOK(pow->pwNoteBook), pwp, gtk_label_new(_("Game")));
    gtk_container_add(GTK_CONTAINER(pwp), pwvbox);
#endif

    pow->pwAutoGame = gtk_check_button_new_with_label(_("Start new games immediately"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwAutoGame, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwAutoGame,
                                _("Whenever a game is complete, automatically "
                                  "start another one in the same match or session."));

    pow->pwAutoRoll = gtk_check_button_new_with_label(_("Roll the dice automatically"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwAutoRoll, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwAutoRoll,
                                _("On a human player's turn, if they are not "
                                  "permitted to double, then roll the dice immediately."));

    pow->pwAutoMove = gtk_check_button_new_with_label(_("Play forced moves automatically"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwAutoMove, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwAutoMove,
                                _("On a human player's turn, if there are no "
                                  "legal moves or only one legal move, then " "finish their turn for them."));

    pow->pwAutoBearoff = gtk_check_button_new_with_label(_("Play bearoff moves automatically"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwAutoBearoff, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwAutoBearoff,
                                _("On a human player's turn in a "
                                  "non-contact bearoff, if there is an "
                                  "unambiguous move which bears off as "
                                  "many chequers as possible, then " "choose that move automatically."));

    pow->pwAutoEndGame = gtk_check_button_new_with_label(_("End Game asks confirmation"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwAutoEndGame, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwAutoEndGame,
                                _("Ask confirmation when the End Game button from the toolbar is used."));

    pow->pwIllegal = gtk_check_button_new_with_label(_("Allow dragging to illegal points"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwIllegal), fGUIIllegal);
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwIllegal, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwIllegal,
                                _("If set, when considering your move "
                                  "you may temporarily move chequers onto "
                                  "points which cannot be reached with "
                                  "the current dice roll. If unset, you "
                                  "may move chequers only onto legal "
                                  "points.  Either way, the resulting "
                                  "move must be legal when you pick up " "the dice, or it will not be accepted."));

    pwf = gtk_frame_new(_("Variations"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pwf, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(pwf), 4);
#if GTK_CHECK_VERSION(3,0,0)
    pwb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
    pwb = gtk_vbox_new(FALSE, 0);
#endif
    gtk_container_add(GTK_CONTAINER(pwf), pwb);

    for (i = 0; i < NUM_VARIATIONS; ++i) {

        pow->apwVariations[i] =
            i ?
            gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON
                                                        (pow->apwVariations[0]),
                                                        gettext(aszVariations[i])) :
            gtk_radio_button_new_with_label(NULL, gettext(aszVariations[i]));

        gtk_box_pack_start(GTK_BOX(pwb), pow->apwVariations[i], FALSE, FALSE, 0);

        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->apwVariations[i]), bgvDefault == (bgvariation) i);

        gtk_widget_set_tooltip_text(pow->apwVariations[i], gettext(aszVariationsTooltips[i]));

    }

    /* disable entries if hypergammon databases are not available */

    for (i = 0; i < 3; ++i)
        gtk_widget_set_sensitive(GTK_WIDGET(pow->apwVariations[i + VARIATION_HYPERGAMMON_1]), apbcHyper[i] != NULL);
}

static void
append_cube_options(optionswidget * pow)
{
    GtkWidget *pwvbox;
    GtkWidget *pwev;
    GtkWidget *pwhbox;
#if !GTK_CHECK_VERSION(3,0,0)
    GtkWidget *pwp;
#endif

    /* Cube options */

#if GTK_CHECK_VERSION(3,0,0)
    pwvbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(pwvbox, GTK_ALIGN_START);
    gtk_widget_set_valign(pwvbox, GTK_ALIGN_START);
    gtk_container_set_border_width(GTK_CONTAINER(pwvbox), 4);
    gtk_notebook_append_page(GTK_NOTEBOOK(pow->pwNoteBook), pwvbox, gtk_label_new(_("Cube")));
#else
    pwvbox = gtk_vbox_new(FALSE, 0);
    pwp = gtk_alignment_new(0, 0, 0, 0);
    gtk_container_set_border_width(GTK_CONTAINER(pwp), 4);
    gtk_notebook_append_page(GTK_NOTEBOOK(pow->pwNoteBook), pwp, gtk_label_new(_("Cube")));
    gtk_container_add(GTK_CONTAINER(pwp), pwvbox);
#endif

    pow->pwCubeUsecube = gtk_check_button_new_with_label(_("Use doubling cube"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwCubeUsecube, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwCubeUsecube,
                                _("When the doubling cube is used, under certain "
                                  "conditions players may offer to raise the stakes "
                                  "of the game by using the \"double\" command."));
    g_signal_connect(G_OBJECT(pow->pwCubeUsecube), "toggled", G_CALLBACK(UseCubeToggled), pow);

    pow->pwAutoCrawford = gtk_check_button_new_with_label(_("Use Crawford rule"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwAutoCrawford, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwAutoCrawford,
                                _("In match play, the Crawford rule specifies that "
                                  "if either player reaches match point (i.e. is "
                                  "one point away from winning the match), then "
                                  "the doubling cube may not be used for the next " "game only."));

    pow->pwCubeJacoby = gtk_check_button_new_with_label(_("Use Jacoby rule"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwCubeJacoby, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwCubeJacoby,
                                _("Under the Jacoby rule, players may not score "
                                  "double or triple for a gammon or backgammon "
                                  "unless the cube has been doubled and accepted.  "
                                  "The Jacoby rule is only ever used in money " "games, not matches."));

    pwev = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(pwev), FALSE);
    gtk_box_pack_start(GTK_BOX(pwvbox), pwev, FALSE, FALSE, 0);
#if GTK_CHECK_VERSION(3,0,0)
    pwhbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
    pwhbox = gtk_hbox_new(FALSE, 4);
#endif
    gtk_container_add(GTK_CONTAINER(pwev), pwhbox);

    pow->pwBeaversLabel = gtk_label_new(_("Maximum number of beavers:"));
    gtk_box_pack_start(GTK_BOX(pwhbox), pow->pwBeaversLabel, FALSE, FALSE, 0);

    pow->padjCubeBeaver = GTK_ADJUSTMENT(gtk_adjustment_new(1, 0, 12, 1, 1, 0));
    pow->pwBeavers = gtk_spin_button_new(GTK_ADJUSTMENT(pow->padjCubeBeaver), 1, 0);
    gtk_box_pack_start(GTK_BOX(pwhbox), pow->pwBeavers, TRUE, TRUE, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(pow->pwBeavers), TRUE);
    gtk_widget_set_tooltip_text(pwev,
                                _("When doubled, a player may \"beaver\" (instantly "
                                  "redouble).  This option allows you to specify "
                                  "how many consecutive redoubles are permitted.  "
                                  "Beavers are only ever used in money games, not " "matches."));

    pwev = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(pwev), FALSE);
    gtk_box_pack_start(GTK_BOX(pwvbox), pwev, FALSE, FALSE, 0);
#if GTK_CHECK_VERSION(3,0,0)
    pwhbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
    pwhbox = gtk_hbox_new(FALSE, 4);
#endif
    gtk_container_add(GTK_CONTAINER(pwev), pwhbox);

    pow->pwAutomaticLabel = gtk_label_new(_("Maximum automatic doubles:"));
    gtk_box_pack_start(GTK_BOX(pwhbox), pow->pwAutomaticLabel, FALSE, FALSE, 0);

    pow->padjCubeAutomatic = GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 12, 1, 1, 0));
    pow->pwAutomatic = gtk_spin_button_new(GTK_ADJUSTMENT(pow->padjCubeAutomatic), 1, 0);
    gtk_box_pack_start(GTK_BOX(pwhbox), pow->pwAutomatic, TRUE, TRUE, 0);
    gtk_widget_set_tooltip_text(pwev,
                                _("If the opening roll is a double, the players "
                                  "may choose to increase the cube value and "
                                  "reroll (an \"automatic double\").  This option "
                                  "allows you to control how many automatic doubles "
                                  "may be applied.  Automatic doubles are only "
                                  "ever used in money games, not matches."));
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(pow->pwAutomatic), TRUE);
}

static void
append_tutor_options(optionswidget * pow)
{
    GtkWidget *pwvbox;
    GtkWidget *pwf;
    GtkWidget *pwev;
    GtkWidget *pwhbox;
#if !GTK_CHECK_VERSION(3,0,0)
    GtkWidget *pwp;
#endif
    char **ppch;

    /* Tutor options */

    pwf = gtk_frame_new(NULL);

#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_halign(pwf, GTK_ALIGN_START);
    gtk_widget_set_valign(pwf, GTK_ALIGN_START);
    pwvbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_set_border_width(GTK_CONTAINER(pwf), 4);
    gtk_notebook_append_page(GTK_NOTEBOOK(pow->pwNoteBook), pwf, gtk_label_new(_("Tutor")));
    gtk_container_set_border_width(GTK_CONTAINER(pwvbox), 6);
    gtk_container_add(GTK_CONTAINER(pwf), pwvbox);
#else
    pwp = gtk_alignment_new(0, 0, 0, 0);
    gtk_container_set_border_width(GTK_CONTAINER(pwp), 4);
    gtk_notebook_append_page(GTK_NOTEBOOK(pow->pwNoteBook), pwp, gtk_label_new(_("Tutor")));
    gtk_container_add(GTK_CONTAINER(pwp), pwf);
    pwvbox = gtk_vbox_new(FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(pwvbox), 6);
    gtk_container_add(GTK_CONTAINER(pwf), pwvbox);
#endif

    pow->pwTutor = gtk_check_button_new_with_label(_("Tutor mode"));
    gtk_frame_set_label_widget(GTK_FRAME(pwf), pow->pwTutor);
    gtk_widget_set_tooltip_text(pow->pwTutor,
                                _("When using the tutor, GNU Backgammon will "
                                  "analyse your decisions during play and prompt "
                                  "you if it thinks you are making a mistake."));

    g_signal_connect(G_OBJECT(pow->pwTutor), "toggled", G_CALLBACK(TutorToggled), pow);

    pow->pwTutorCube = gtk_check_button_new_with_label(_("Cube Decisions"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwTutorCube, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwTutorCube, _("Use the tutor for cube decisions."));

    pow->pwTutorChequer = gtk_check_button_new_with_label(_("Chequer play"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwTutorChequer, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwTutorChequer, _("Use the tutor for chequer play decisions."));

    pwev = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(pwev), FALSE);
    gtk_box_pack_start(GTK_BOX(pwvbox), pwev, FALSE, FALSE, 4);
#if GTK_CHECK_VERSION(3,0,0)
    pwhbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
    pwhbox = gtk_hbox_new(FALSE, 4);
#endif
    gtk_container_add(GTK_CONTAINER(pwev), pwhbox);

    gtk_box_pack_start(GTK_BOX(pwhbox), gtk_label_new(_("Warning level:")), FALSE, FALSE, 0);
    pow->pwTutorSkill = gtk_combo_box_text_new();
    gtk_box_pack_start(GTK_BOX(pwhbox), pow->pwTutorSkill, FALSE, FALSE, 0);
    for (ppch = aszTutor; *ppch; ppch++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pow->pwTutorSkill), gettext(*ppch));
    }
    g_assert(nTutorSkillCurrent >= 0 && nTutorSkillCurrent <= 2);
    gtk_widget_set_tooltip_text(pwev,
                                _("Specify how bad GNU Backgammon must think a "
                                  "decision is before questioning you about a " "possible mistake."));

    gtk_widget_set_sensitive(pow->pwTutorSkill, fTutor);
    gtk_widget_set_sensitive(pow->pwTutorCube, fTutor);
    gtk_widget_set_sensitive(pow->pwTutorChequer, fTutor);
}

static void
append_quiz_options(optionswidget * pow)
{
    GtkWidget *pwvbox;
    GtkWidget *pwvbox2;
    GtkWidget *pwf;
    GtkWidget *pwf2;
    GtkWidget *pwev;
    GtkWidget *pwhbox;
#if !GTK_CHECK_VERSION(3,0,0)
    GtkWidget *pwp;
#endif
    char **ppch;

    /* Quiz options */

    pwf = gtk_frame_new(NULL);
    pwf2 = gtk_frame_new(NULL);

#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_halign(pwf, GTK_ALIGN_START);
    gtk_widget_set_valign(pwf, GTK_ALIGN_START);
    gtk_container_set_border_width(GTK_CONTAINER(pwf), 4);
    gtk_notebook_append_page(GTK_NOTEBOOK(pow->pwNoteBook), pwf, gtk_label_new(_("Quiz")));
    pwvbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_set_border_width(GTK_CONTAINER(pwvbox), 6);
    gtk_container_add(GTK_CONTAINER(pwf), pwvbox);

    gtk_container_add(GTK_CONTAINER(pwvbox), pwf2);
    pwvbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_set_border_width(GTK_CONTAINER(pwvbox2), 6);
    gtk_container_add(GTK_CONTAINER(pwf2), pwvbox2);
#else
    pwp = gtk_alignment_new(0, 0, 0, 0);
    gtk_container_set_border_width(GTK_CONTAINER(pwp), 4);
    gtk_notebook_append_page(GTK_NOTEBOOK(pow->pwNoteBook), pwp, gtk_label_new(_("Quiz")));
    gtk_container_add(GTK_CONTAINER(pwp), pwf);
    pwvbox = gtk_vbox_new(FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(pwvbox), 6);
    gtk_container_add(GTK_CONTAINER(pwf), pwvbox);

    gtk_container_add(GTK_CONTAINER(pwvbox), pwf2);
    pwvbox2 = gtk_vbox_new(FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(pwvbox2), 6);
    gtk_container_add(GTK_CONTAINER(pwf2), pwvbox2);
#endif
    // GtkWidget *pwQuiz;
    // GtkWidget *pwQuizAutoAdd;
    // GtkWidget *pwQuizSkill;
    // GtkWidget *pwQuizOnePlayer;

    pow->pwQuiz = gtk_check_button_new_with_label(_("Allow quiz"));
    gtk_frame_set_label_widget(GTK_FRAME(pwf), pow->pwQuiz);
    gtk_widget_set_tooltip_text(pow->pwQuiz,
                                _("The quiz mode allows replaying collected mistakes. "
                                  "Selecting this option allows GNU Backgammon to get positions "
                                  "ready to be collected."));
    g_signal_connect(G_OBJECT(pow->pwQuiz), "toggled", G_CALLBACK(QuizToggled), pow);

    pow->pwQuizAutoAdd = gtk_check_button_new_with_label(_("Automatically collect mistakes"));
    gtk_frame_set_label_widget(GTK_FRAME(pwf2), pow->pwQuizAutoAdd);
    gtk_widget_set_tooltip_text(pow->pwQuizAutoAdd,
                                _("Allow GNU Backgammon to automatically collect mistakes so "
                                "they can later be replayed. "));
    g_signal_connect(G_OBJECT(pow->pwQuizAutoAdd), "toggled", G_CALLBACK(QuizToggled), pow);

    pow->pwQuizOnePlayer = gtk_check_button_new_with_label(_("Automatically collect for player 1 only"));
    // gtk_frame_set_label_widget(GTK_FRAME(pwf2), pow->pwQuizOnePlayer);
    gtk_box_pack_start(GTK_BOX(pwvbox2), pow->pwQuizOnePlayer, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwQuizOnePlayer,
                                _("Set quiz automatic collection for player 1 only."));
    // g_signal_connect(G_OBJECT(pow->pwQuizOnePlayer), "toggled", G_CALLBACK(QuizToggled), pow);



    // pow->pwQuizCube = gtk_check_button_new_with_label(_("Cube Decisions"));
    // gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwQuizCube, FALSE, FALSE, 0);
    // gtk_widget_set_tooltip_text(pow->pwQuizCube, _("Use the Quiz for cube decisions."));

    // pow->pwQuizChequer = gtk_check_button_new_with_label(_("Chequer play"));
    // gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwQuizChequer, FALSE, FALSE, 0);
    // gtk_widget_set_tooltip_text(pow->pwQuizChequer, _("Use the Quiz for chequer play decisions."));

    pwev = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(pwev), FALSE);
    gtk_box_pack_start(GTK_BOX(pwvbox2), pwev, FALSE, FALSE, 4);
#if GTK_CHECK_VERSION(3,0,0)
    pwhbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
    pwhbox = gtk_hbox_new(FALSE, 4);
#endif
    gtk_container_add(GTK_CONTAINER(pwev), pwhbox);

    gtk_box_pack_start(GTK_BOX(pwhbox), gtk_label_new(_("Automatically collect with minimum mistake level:")), FALSE, FALSE, 0);
    pow->pwQuizSkill = gtk_combo_box_text_new();
    gtk_box_pack_start(GTK_BOX(pwhbox), pow->pwQuizSkill, FALSE, FALSE, 0);
    for (ppch = aszQuiz; *ppch; ppch++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pow->pwQuizSkill), gettext(*ppch));
    }
    g_assert(nQuizSkillCurrent >= 0 && nQuizSkillCurrent <= 2);
    gtk_widget_set_tooltip_text(pwev,
                                _("Specify how bad GNU Backgammon must think a "
                                  "decision is before adding it to the quiz positions."));

    pow->pwQuizAtMoney = gtk_check_button_new_with_label(_("Evaluate quiz positions assuming money play"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwQuizAtMoney, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwQuizAtMoney,
                                _("Intuitively allow quiz questions to erase the match scores at which the quiz positions were collected "
                                "so they don't impact the result. "));
    g_signal_connect(G_OBJECT(pow->pwQuizAtMoney), "toggled", G_CALLBACK(QuizToggled), pow);

    gtk_widget_set_sensitive(pow->pwQuizAutoAdd, TRUE);//fUseQuiz);
    gtk_widget_set_sensitive(pow->pwQuizSkill, TRUE);//fUseQuiz && fQuizAutoAdd);
    gtk_widget_set_sensitive(pow->pwQuizOnePlayer, TRUE);//fUseQuiz && fQuizAutoAdd);
    gtk_widget_set_sensitive(pow->pwQuizAtMoney, TRUE);//fUseQuiz && fQuizAutoAdd);
}


static void
append_display_options(optionswidget * pow)
{
    GtkWidget *pwvbox;
    GtkWidget *pwev;
    GtkWidget *pwhbox;
    GtkWidget *pw;
    GtkWidget *pwh;
    GtkWidget *pwEdit;
    GtkWidget *pwAnimBox;
    GtkWidget *pwFrame;
    GtkWidget *pwBox;
    GtkWidget *pwSpeed;
    GtkWidget *pwScale;
#if !GTK_CHECK_VERSION(3,0,0)
    GtkWidget *pwp;
#endif

    BoardData *bd = BOARD(pwBoard)->board_data;

    /* Display options */

#if GTK_CHECK_VERSION(3,0,0)
    pwvbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(pwvbox, GTK_ALIGN_START);
    gtk_widget_set_valign(pwvbox, GTK_ALIGN_START);
    gtk_container_set_border_width(GTK_CONTAINER(pwvbox), 4);
    gtk_notebook_append_page(GTK_NOTEBOOK(pow->pwNoteBook), pwvbox, gtk_label_new(_("Display")));
#else
    pwvbox = gtk_vbox_new(FALSE, 0);
    pwp = gtk_alignment_new(0, 0, 0, 0);
    gtk_container_set_border_width(GTK_CONTAINER(pwp), 4);
    gtk_notebook_append_page(GTK_NOTEBOOK(pow->pwNoteBook), pwp, gtk_label_new(_("Display")));
    gtk_container_add(GTK_CONTAINER(pwp), pwvbox);
#endif

    pow->pwGameClockwise = gtk_check_button_new_with_label(_("Clockwise movement"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwGameClockwise, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwGameClockwise,
                                _("Orient up the board so that player 1's chequers "
                                  "advance clockwise (and player 0 moves "
                                  "anticlockwise).  Otherwise, player 1 moves "
                                  "anticlockwise and player 0 moves clockwise."));

#if GTK_CHECK_VERSION(3,0,0)
    pwh = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
#else
    pwh = gtk_hbox_new(FALSE, 8);
#endif

    gtk_box_pack_start(GTK_BOX(pwvbox), pwh, FALSE, FALSE, 0);

    // pow->pwKeyName = gtk_check_button_new_with_label(_("Use SmartSit to sit at bottom of board in opened matches"));
    // gtk_box_pack_start(GTK_BOX(pwh), pow->pwKeyName, FALSE, FALSE, 0);
    // gtk_widget_set_tooltip_text(pow->pwKeyName,
    //                             _("(1) If you select a player to be player1 (the second player) "
    //                               "and sit at the bottom of the board, the player's name is "
    //                               "automatically added to the list of key player names. "
    //                               "(2) Then, when you open a new match, if such a key player is player0 "
    //                               "and player1 is unknown, they swap places."));

    pwEdit = gtk_button_new_with_label(_("Edit"));
    g_signal_connect(G_OBJECT(pwEdit), "clicked", G_CALLBACK(GTKCommandEditKeyNames), pow);	//(void *) pAnalDetails);
    gtk_box_pack_start(GTK_BOX(pwh), pwEdit, FALSE, FALSE, 0);
    AddText(pwh, _("Use SmartSit to automatically sit at bottom of board"));
    gtk_widget_set_tooltip_text(pwh,
                                _("SmartSit assumes that you'd like to arrange the board so "
                                  "you can sit at the bottom (i.e. so you can be player1, the "
                                  "second player). "
                                  "\n(1) LEARNING: If you select a player to be player1 "
                                  "and sit at the bottom of the board, SmartSit guesses that "
                                  "the player's name is one of your aliases, and adds it"
                                  "to a list of key player names (which you can edit here).  "
                                  "\n(2) APPLYING: When you open a new match from a file "
                                  "(e.g., from a game played on the internet), if "
                                  "player0 is a known key player name while player1 is unknown, "
                                  "they swap places automatically, so you can sit at the bottom "
                                  "of the board."));

    pwev = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(pwev), FALSE);
    gtk_box_pack_start(GTK_BOX(pwvbox), pwev, FALSE, FALSE, 0);
#if GTK_CHECK_VERSION(3,0,0)
    pwhbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
    pwhbox = gtk_hbox_new(FALSE, 4);
#endif
    gtk_container_add(GTK_CONTAINER(pwev), pwhbox);

    gtk_box_pack_start(GTK_BOX(pwhbox), gtk_label_new(_("Move delay:")), FALSE, FALSE, 0);
    pow->padjDelay = GTK_ADJUSTMENT(gtk_adjustment_new(nDelay, 0, 3000, 1, 10, 0));
    pw = gtk_spin_button_new(GTK_ADJUSTMENT(pow->padjDelay), 1, 0);
    gtk_box_pack_start(GTK_BOX(pwhbox), pw, TRUE, TRUE, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(pw), TRUE);
    gtk_box_pack_start(GTK_BOX(pwhbox), gtk_label_new(_("ms")), FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pwev,
                                _("Set a delay so that GNU Backgammon "
                                  "will pause between each move, to give you a " "chance to see it."));

    pow->pwUseDiceIcon = gtk_check_button_new_with_label(_("Show dice below board when human player on roll"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwUseDiceIcon), bd->rd->fDiceArea);
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwUseDiceIcon, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwUseDiceIcon,
                                _("When it is your turn to roll, a pair of dice "
                                  "will be shown below the board, and you can "
                                  "click on them to roll.  Even if you choose not "
                                  "to show the dice, you can always roll by "
                                  "clicking the area in the middle of the board " "where the dice will land."));

    pow->pwShowIDs = gtk_check_button_new_with_label(_("Show Position ID in status bar"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwShowIDs), fShowIDs);
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwShowIDs, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwShowIDs,
                                _("One entry field will be shown above the board, "
                                  "which can be useful for recording, entering and "
                                  "exchanging board positions and match situations."));

#if GTK_CHECK_VERSION(3,0,0)
    pwhbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
#else
    pwhbox = gtk_hbox_new(FALSE, 2);
#endif
    pow->pwShowPips = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pow->pwShowPips), _("None"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pow->pwShowPips), _("Pips"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pow->pwShowPips), _("Pips or EPC"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pow->pwShowPips), _("Pips and EPC"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(pow->pwShowPips), gui_show_pips);
    gtk_box_pack_start(GTK_BOX(pwhbox), gtk_label_new(_("Show Pips")), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pwhbox), pow->pwShowPips, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pwvbox), pwhbox, FALSE, FALSE, 0);

#if GTK_CHECK_VERSION(3,0,0)
    pwAnimBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    pwAnimBox = gtk_hbox_new(FALSE, 0);
#endif
    gtk_box_pack_start(GTK_BOX(pwvbox), pwAnimBox, FALSE, FALSE, 0);

    pwFrame = gtk_frame_new(_("Animation"));
    gtk_box_pack_start(GTK_BOX(pwAnimBox), pwFrame, FALSE, FALSE, 4);

#if GTK_CHECK_VERSION(3,0,0)
    pwBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
    pwBox = gtk_vbox_new(FALSE, 0);
#endif
    gtk_container_add(GTK_CONTAINER(pwFrame), pwBox);

    pow->pwAnimateNone = gtk_radio_button_new_with_label(NULL, _("None"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwAnimateNone), animGUI == ANIMATE_NONE);
    gtk_box_pack_start(GTK_BOX(pwBox), pow->pwAnimateNone, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwAnimateNone,
                                _("Do not display any kind of animation for " "automatically moved chequers."));

    pow->pwAnimateBlink =
        gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pow->pwAnimateNone), _("Blink moving chequers"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwAnimateBlink), animGUI == ANIMATE_BLINK);
    gtk_box_pack_start(GTK_BOX(pwBox), pow->pwAnimateBlink, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwAnimateBlink,
                                _("When automatically moving chequers, flash "
                                  "them between the original and final points."));

    pow->pwAnimateSlide =
        gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pow->pwAnimateNone), _("Slide moving chequers"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwAnimateSlide), animGUI == ANIMATE_SLIDE);
    gtk_box_pack_start(GTK_BOX(pwBox), pow->pwAnimateSlide, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwAnimateSlide,
                                _("Show automatically moved chequers moving across " "the board between the points."));

    pwev = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(pwev), FALSE);
    gtk_box_pack_start(GTK_BOX(pwAnimBox), pwev, FALSE, FALSE, 0);
#if GTK_CHECK_VERSION(3,0,0)
    pwSpeed = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    pwSpeed = gtk_hbox_new(FALSE, 0);
#endif
    gtk_container_add(GTK_CONTAINER(pwev), pwSpeed);

    pow->padjSpeed = GTK_ADJUSTMENT(gtk_adjustment_new(nGUIAnimSpeed, 0, 7, 1, 1, 0));
#if GTK_CHECK_VERSION(3,0,0)
    pwScale = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, pow->padjSpeed);
#else
    pwScale = gtk_hscale_new(pow->padjSpeed);
#endif
    gtk_widget_set_size_request(pwScale, 100, -1);
    gtk_scale_set_draw_value(GTK_SCALE(pwScale), FALSE);
    gtk_scale_set_digits(GTK_SCALE(pwScale), 0);

    gtk_box_pack_start(GTK_BOX(pwSpeed), gtk_label_new(_("Speed:")), FALSE, FALSE, 8);
    gtk_box_pack_start(GTK_BOX(pwSpeed), gtk_label_new(_("Slow")), FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(pwSpeed), pwScale, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(pwSpeed), gtk_label_new(_("Fast")), FALSE, FALSE, 4);
    gtk_widget_set_tooltip_text(pwev, _("Control the rate at which blinking or sliding " "chequers are displayed."));

    g_signal_connect(G_OBJECT(pow->pwAnimateNone), "toggled", G_CALLBACK(ToggleAnimation), pwSpeed);
    ToggleAnimation(pow->pwAnimateNone, pwSpeed);

    pow->pwGrayEdit = gtk_check_button_new_with_label(_("Gray board in edit mode"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwGrayEdit), fGUIGrayEdit);
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwGrayEdit, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwGrayEdit, _("Gray board in edit mode to make it clearer"));

    pow->pwDragTargetHelp = gtk_check_button_new_with_label(_("Show target help while dragging a chequer"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwDragTargetHelp), fGUIDragTargetHelp);
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwDragTargetHelp, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwDragTargetHelp,
                                _("The possible target points for a move will be "
                                  "indicated by coloured rectangles when a chequer "
                                  "has been dragged a short distance."));

    pow->pwDisplay = gtk_check_button_new_with_label(_("Display computer moves"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwDisplay, FALSE, FALSE, 0);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwDisplay), fDisplay);
    gtk_widget_set_tooltip_text(pow->pwDisplay,
                                _("Show each move made by a computer player.  You "
                                  "might want to turn this off when playing games "
                                  "between computer players, to speed things up."));

    pow->pwSetWindowPos = gtk_check_button_new_with_label(_("Restore window positions"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwSetWindowPos, FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwSetWindowPos), fGUISetWindowPos);
    gtk_widget_set_tooltip_text(pow->pwSetWindowPos,
                                _("Restore the previous size and position when "
                                  "recreating windows.  This is really the job "
                                  "of the session manager and window manager, "
                                  "but since some platforms have poor or missing "
                                  "window managers, GNU Backgammon tries to " "do the best it can."));

    pow->pwOutputMWC = gtk_check_button_new_with_label(_("Match equity as MWC"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwOutputMWC, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwOutputMWC,
                                _("Show match equities as match winning chances.  "
                                  "Otherwise, match equities will be shown as "
                                  "EMG (equivalent equity in a money game) " "points-per-game."));

    pow->pwOutputGWC = gtk_check_button_new_with_label(_("GWC as percentage"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwOutputGWC, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwOutputGWC,
                                _("Show game winning chances as percentages (e.g. "
                                  "58.3%).  Otherwise, game winning chances will "
                                  "be shown as probabilities (e.g. 0.583)."));

    pow->pwOutputMWCpst = gtk_check_button_new_with_label(_("MWC as percentage"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwOutputMWCpst, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwOutputMWCpst,
                                _("Show match winning chances as percentages (e.g. "
                                  "71.2%).  Otherwise, match winning chances will "
                                  "be shown as probabilities (e.g. 0.712)."));

    /* number of digits in output */

    pwev = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(pwev), FALSE);
    gtk_box_pack_start(GTK_BOX(pwvbox), pwev, FALSE, FALSE, 0);
#if GTK_CHECK_VERSION(3,0,0)
    pwhbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
    pwhbox = gtk_hbox_new(FALSE, 4);
#endif
    gtk_container_add(GTK_CONTAINER(pwev), pwhbox);

    pw = gtk_label_new(_("Number of digits in output:"));
    gtk_box_pack_start(GTK_BOX(pwhbox), pw, FALSE, FALSE, 0);

    pow->padjDigits = GTK_ADJUSTMENT(gtk_adjustment_new(1, 0, MAX_OUTPUT_DIGITS, 1, 1, 0));
    pow->pwDigits = gtk_spin_button_new(GTK_ADJUSTMENT(pow->padjDigits), 1, 0);
    gtk_box_pack_start(GTK_BOX(pwhbox), pow->pwDigits, TRUE, TRUE, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(pow->pwDigits), TRUE);
    gtk_widget_set_tooltip_text(pwev,
                                _("Control the number of digits to be "
                                  "shown after the decimal point in "
                                  "probabilities and equities. "
                                  "This value is used throughout GNU Backgammon, "
                                  "i.e., in exported files, hints, analysis etc."
                                  "The default value of 3 results "
                                  "in equities output as +0.123. "
                                  "The equities and probabilities are internally "
                                  "stored with 7-8 digits, so it's possible to "
                                  "change the value "
                                  "after an analysis if you want more digits "
                                  "shown in the output. The output of match "
                                  "winning chances are derived from this value "
                                  "to produce numbers with approximately the same "
                                  "number of digits. The default value of 3 results "
                                  "in MWCs being output as 50.33%."));
}

static void
append_match_options(optionswidget * pow)
{
    GtkWidget *pwvbox;
    GtkWidget *pwev;
    GtkWidget *pwhbox;
    GtkWidget *pw;
    GtkWidget *pwf;
    GtkWidget *pwb;
    GtkWidget *pwhoriz;
    GtkWidget *pwLabelFile;
#if !GTK_CHECK_VERSION(3,0,0)
    GtkWidget *pwp;
#endif

    /* Match options */

#if GTK_CHECK_VERSION(3,0,0)
    pwvbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(pwvbox, GTK_ALIGN_START);
    gtk_widget_set_valign(pwvbox, GTK_ALIGN_START);
    gtk_container_set_border_width(GTK_CONTAINER(pwvbox), 4);
    gtk_notebook_append_page(GTK_NOTEBOOK(pow->pwNoteBook), pwvbox, gtk_label_new(_("Match")));
#else
    pwvbox = gtk_vbox_new(FALSE, 0);
    pwp = gtk_alignment_new(0, 0, 0, 0);
    gtk_container_set_border_width(GTK_CONTAINER(pwp), 4);
    gtk_notebook_append_page(GTK_NOTEBOOK(pow->pwNoteBook), pwp, gtk_label_new(_("Match")));
    gtk_container_add(GTK_CONTAINER(pwp), pwvbox);
#endif

    pwev = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(pwev), FALSE);
    gtk_box_pack_start(GTK_BOX(pwvbox), pwev, FALSE, FALSE, 0);
#if GTK_CHECK_VERSION(3,0,0)
    pwhbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
    pwhbox = gtk_hbox_new(FALSE, 4);
#endif
    gtk_container_add(GTK_CONTAINER(pwev), pwhbox);

    gtk_box_pack_start(GTK_BOX(pwhbox), gtk_label_new(_("Default match length:")), FALSE, FALSE, 0);

    pow->padjLength = GTK_ADJUSTMENT(gtk_adjustment_new(nDefaultLength, 0, 99, 1, 1, 0));
    pw = gtk_spin_button_new(GTK_ADJUSTMENT(pow->padjLength), 1, 0);
    gtk_box_pack_start(GTK_BOX(pwhbox), pw, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(pwhbox), gtk_label_new(Q_("length|points")), FALSE, FALSE, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(pw), TRUE);
    gtk_widget_set_tooltip_text(pwev, _("Specify the default length to use when starting " "new matches."));

    pwf = gtk_frame_new(_("Match equity table"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pwf, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(pwf), 4);
#if GTK_CHECK_VERSION(3,0,0)
    pwb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
    pwb = gtk_vbox_new(FALSE, 0);
#endif
    gtk_container_add(GTK_CONTAINER(pwf), pwb);

#if GTK_CHECK_VERSION(3,0,0)
    pwhoriz = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
    pwhoriz = gtk_hbox_new(FALSE, 4);
#endif
    gtk_container_add(GTK_CONTAINER(pwb), pwhoriz);
    gtk_box_pack_start(GTK_BOX(pwhoriz), gtk_label_new(_("Current:")), FALSE, FALSE, 2);
    pwLabelFile = gtk_label_new((char *) miCurrent.szFileName);
    gtk_box_pack_end(GTK_BOX(pwhoriz), pwLabelFile, FALSE, FALSE, 2);
    pow->pwLoadMET = gtk_button_new_with_label(_("Load..."));
    gtk_box_pack_start(GTK_BOX(pwb), pow->pwLoadMET, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(pow->pwLoadMET), 2);
    gtk_widget_set_tooltip_text(pow->pwLoadMET, _("Read a file containing a match equity table."));

    g_signal_connect(G_OBJECT(pow->pwLoadMET), "clicked", G_CALLBACK(SetMET), (gpointer) pwLabelFile);

    pow->pwCubeInvert = gtk_check_button_new_with_label(_("Invert table"));
    gtk_box_pack_start(GTK_BOX(pwb), pow->pwCubeInvert, FALSE, FALSE, 0);

    /* similar tooltip is used in gtkmet.c:GTKShowMatchEquityTable() */
    gtk_widget_set_tooltip_text(pow->pwCubeInvert,
                                _("Use the specified match equity table "
                                  "around the other way (i.e., swap the players before "
                                  "looking up equities in the table)."));

    gtk_widget_set_sensitive(pow->pwLoadMET, fCubeUse);
    gtk_widget_set_sensitive(pow->pwCubeInvert, fCubeUse);
}

static void
append_sound_options(optionswidget * pow)
{
    GtkWidget *pwvboxMain;
    GtkWidget *pwhboxTop;
    GtkWidget *pwvboxTop;
    GtkWidget *pwScrolled;
    GtkWidget *pwhbox;
    GtkWidget *pwvboxDetails;
    GtkWidget *pwLabel;
    gnubgsound i;
    GtkTreeIter iter;
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkListStore *store = gtk_list_store_new(1, G_TYPE_STRING);

#if GTK_CHECK_VERSION(3,0,0)
    pwvboxMain = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
    pwvboxMain = gtk_vbox_new(FALSE, 0);
#endif
    gtk_container_set_border_width(GTK_CONTAINER(pwvboxMain), 4);
    gtk_notebook_append_page(GTK_NOTEBOOK(pow->pwNoteBook), pwvboxMain, gtk_label_new(_("Sound")));
#if GTK_CHECK_VERSION(3,0,0)
    pwhboxTop = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    pwhboxTop = gtk_hbox_new(FALSE, 0);
#endif
    gtk_box_pack_start(GTK_BOX(pwvboxMain), pwhboxTop, TRUE, TRUE, 0);
#if GTK_CHECK_VERSION(3,0,0)
    pwvboxTop = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
    pwvboxTop = gtk_vbox_new(FALSE, 0);
#endif
    gtk_box_pack_start(GTK_BOX(pwhboxTop), pwvboxTop, TRUE, TRUE, 0);

#if GTK_CHECK_VERSION(3,0,0)
    pwhbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    pwhbox = gtk_hbox_new(FALSE, 0);
#endif
    pwLabel = gtk_label_new(_("Sound command:"));
    gtk_box_pack_start(GTK_BOX(pwhbox), pwLabel, FALSE, FALSE, 0);
    pwSoundCommand = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(pwSoundCommand), sound_get_command());
    gtk_box_pack_start(GTK_BOX(pwhbox), pwSoundCommand, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(pwvboxTop), pwhbox, FALSE, FALSE, 0);

    soundBeepIllegal = gtk_check_button_new_with_label(_("Beep on invalid input"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(soundBeepIllegal), fGUIBeep);
    gtk_box_pack_start(GTK_BOX(pwvboxTop), soundBeepIllegal, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(soundBeepIllegal, _("Emit a warning beep if invalid moves are attempted."));

    soundsEnabled = gtk_check_button_new_with_label(_("Enable sound effects"));
    gtk_box_pack_start(GTK_BOX(pwvboxTop), soundsEnabled, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(soundsEnabled,
                                _("Have GNU Backgammon make " "sound effects when various events occur."));
    g_signal_connect(G_OBJECT(soundsEnabled), "toggled", G_CALLBACK(SoundToggled), NULL);
#define SOUND_COL 0
    for (i = (gnubgsound) 0; i < NUM_SOUNDS; i++) {
        /* Copy sound path data to be used in dialog */
        soundDetails[i].Path = GetSoundFile(i);

        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, SOUND_COL, Q_(sound_description[i]), -1);
    }

    soundList = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(G_OBJECT(store));    /* The view now holds a reference.  We can get rid
                                         * of our own reference */
    gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(soundList)), GTK_SELECTION_BROWSE);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(soundList), -1, _("Sound Event"),
                                                renderer, "text", SOUND_COL, NULL);
    gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(soundList), FALSE);
    g_signal_connect(soundList, "cursor-changed", G_CALLBACK(SoundSelected), NULL);
    g_signal_connect(soundList, "map_event", G_CALLBACK(SoundGrabFocus), NULL);
    g_signal_connect(soundList, "destroy", G_CALLBACK(SoundTidy), NULL);

    pwScrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(pwScrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(pwScrolled), soundList);

    gtk_box_pack_start(GTK_BOX(pwvboxTop), pwScrolled, TRUE, TRUE, 0);

    soundFrame = gtk_frame_new(NULL);
    gtk_box_pack_start(GTK_BOX(pwvboxMain), soundFrame, FALSE, FALSE, 0);

#if GTK_CHECK_VERSION(3,0,0)
    pwvboxDetails = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
#else
    pwvboxDetails = gtk_vbox_new(FALSE, 4);
#endif
    gtk_container_set_border_width(GTK_CONTAINER(pwvboxDetails), 4);
    soundEnabled = gtk_check_button_new_with_label(_("Enabled"));
    g_signal_connect(soundEnabled, "clicked", G_CALLBACK(SoundEnabledClicked), NULL);
    gtk_box_pack_start(GTK_BOX(pwvboxDetails), soundEnabled, FALSE, FALSE, 0);

#if GTK_CHECK_VERSION(3,0,0)
    pwhbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    pwhbox = gtk_hbox_new(FALSE, 0);
#endif
    gtk_box_pack_start(GTK_BOX(pwhbox), gtk_label_new(_("Path:")), FALSE, FALSE, 0);
    soundPath = gtk_entry_new();
    g_signal_connect(soundPath, "changed", G_CALLBACK(PathChanged), NULL);
    gtk_box_pack_start(GTK_BOX(pwhbox), soundPath, TRUE, TRUE, 0);
    soundPathButton = gtk_button_new_with_label(_("Browse"));
    g_signal_connect(soundPathButton, "clicked", G_CALLBACK(SoundChangePathClicked), NULL);
    gtk_box_pack_start(GTK_BOX(pwhbox), soundPathButton, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pwvboxDetails), pwhbox, FALSE, FALSE, 0);

#if GTK_CHECK_VERSION(3,0,0)
    pwhbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
    pwhbox = gtk_hbox_new(FALSE, 4);
#endif
    soundPlayButton = gtk_button_new_with_label(_("Play Sound"));
    g_signal_connect(soundPlayButton, "clicked", G_CALLBACK(SoundPlayClicked), NULL);
    gtk_box_pack_start(GTK_BOX(pwhbox), soundPlayButton, FALSE, FALSE, 0);
    /* sound default button */
    soundDefaultButton = gtk_button_new_with_label(_("Reset Default"));
    g_signal_connect(soundDefaultButton, "clicked", G_CALLBACK(SoundDefaultClicked), NULL);
    gtk_box_pack_start(GTK_BOX(pwhbox), soundDefaultButton, FALSE, FALSE, 0);

    /* sound all defaults button */
    soundAllDefaultButton = gtk_button_new_with_label(_("Reset All to Defaults"));
    g_signal_connect(soundAllDefaultButton, "clicked", G_CALLBACK(SoundAllDefaultClicked), NULL);
    gtk_box_pack_start(GTK_BOX(pwhbox), soundAllDefaultButton, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(pwvboxDetails), pwhbox, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(soundFrame), pwvboxDetails);

    /* Set true so event fired */
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(soundsEnabled), TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(soundsEnabled), fSound);
}

static void
append_dice_options(optionswidget * pow)
{
    GtkWidget *pwvbox, *pwhbox, *pwvbox2, *pwvbox3, *pw, *frame;
#if !GTK_CHECK_VERSION(3,0,0)
    GtkWidget *pwp;
#endif
    unsigned int i;
    unsigned long nRandom;
#if defined(HAVE_LIBGMP)
    int blumblum = TRUE;
#else
    int blumblum = FALSE;
#endif
    int rngsAdded = 0, rngSelected = -1;

    free(InitRNG(&nRandom, NULL, FALSE, rngCurrent));

#if GTK_CHECK_VERSION(3,0,0)
    pwvbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(pwvbox, GTK_ALIGN_START);
    gtk_widget_set_valign(pwvbox, GTK_ALIGN_START);
    gtk_container_set_border_width(GTK_CONTAINER(pwvbox), 4);
    gtk_notebook_append_page(GTK_NOTEBOOK(pow->pwNoteBook), pwvbox, gtk_label_new(_("Dice")));
#else
    pwvbox = gtk_vbox_new(FALSE, 0);
    pwp = gtk_alignment_new(0, 0, 0, 0);
    gtk_container_set_border_width(GTK_CONTAINER(pwp), 4);
    gtk_notebook_append_page(GTK_NOTEBOOK(pow->pwNoteBook), pwp, gtk_label_new(_("Dice")));
    gtk_container_add(GTK_CONTAINER(pwp), pwvbox);
#endif

    frame = gtk_frame_new(_("Dice generation"));
    gtk_box_pack_start(GTK_BOX(pwvbox), frame, TRUE, TRUE, 0);
#if GTK_CHECK_VERSION(3,0,0)
    pwvbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
#else
    pwvbox2 = gtk_vbox_new(FALSE, 1);
#endif
    gtk_container_set_border_width(GTK_CONTAINER(pwvbox2), 4);
    gtk_container_add(GTK_CONTAINER(frame), pwvbox2);

    pow->pwRngComboBox = NULL;

    for (i = 0; i < NUM_RNGS; i++) {
        if (i >= NUM_RNGS - 3) {
            pow->apwDice[i] =
                gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pow->apwDice[0]), gettext(aszRNG[i]));
            gtk_box_pack_start(GTK_BOX(pwvbox2), pow->apwDice[i], FALSE, FALSE, 0);
            gtk_widget_set_tooltip_text(pow->apwDice[i], gettext(aszRNGTip[i]));
            g_signal_connect(G_OBJECT(pow->apwDice[i]), "toggled", G_CALLBACK(DiceToggled), pow);
        }
        if (i < NUM_RNGS - 3) {
            if (i == 0) {
                pow->apwDice[0] = gtk_radio_button_new_with_label(NULL, _("Random number generator"));
                gtk_box_pack_start(GTK_BOX(pwvbox2), pow->apwDice[0], FALSE, FALSE, 0);
                g_signal_connect(G_OBJECT(pow->apwDice[0]), "toggled", G_CALLBACK(DiceToggled), pow);

#if GTK_CHECK_VERSION(3,0,0)
                pwvbox3 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
                pwvbox3 = gtk_vbox_new(FALSE, 0);
#endif
                gtk_box_pack_start(GTK_BOX(pwvbox2), pwvbox3, FALSE, FALSE, 0);
#if GTK_CHECK_VERSION(3,0,0)
                pwhbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
                pwhbox = gtk_hbox_new(FALSE, 0);
#endif
                gtk_box_pack_start(GTK_BOX(pwvbox3), pwhbox, FALSE, FALSE, 0);

                /* RNG types */
                pow->pwRngComboBox = gtk_combo_box_text_new();
                gtk_box_pack_start(GTK_BOX(pwhbox), pow->pwRngComboBox, FALSE, FALSE, 26);
                /* NB. Doesn't look like gtk currently supports tooltips for individual combobox entries */
                gtk_widget_set_tooltip_text(pow->pwRngComboBox, _("Select a random number generator to use"));

                /* Seed */
#if GTK_CHECK_VERSION(3,0,0)
                pwhbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
                pwhbox = gtk_hbox_new(FALSE, 0);
#endif
                gtk_box_pack_start(GTK_BOX(pwvbox3), pwhbox, FALSE, FALSE, 0);
#if GTK_CHECK_VERSION(3,0,0)
                pow->pwSeed = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
                pow->pwSeed = gtk_hbox_new(FALSE, 0);
#endif
                gtk_box_pack_start(GTK_BOX(pwhbox), pow->pwSeed, TRUE, TRUE, 26);
                gtk_box_pack_start(GTK_BOX(pow->pwSeed), gtk_label_new(_("Seed: ")), FALSE, FALSE, 0);

#if defined(HAVE_LIBGMP)
                /* 9007199254740992 is 2^53, the largest integer
                 * that can be represented exactly in a gdouble */
                pow->padjSeed = GTK_ADJUSTMENT(gtk_adjustment_new((gdouble) 0, 0, 9007199254740992, 1, 1, 0));
#else
                pow->padjSeed = GTK_ADJUSTMENT(gtk_adjustment_new((gdouble) 0, 0, UINT_MAX, 1, 1, 0));
#endif

                pw = gtk_spin_button_new(GTK_ADJUSTMENT(pow->padjSeed), 1, 0);
                gtk_box_pack_start(GTK_BOX(pow->pwSeed), pw, FALSE, FALSE, 0);
                gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(pw), TRUE);

                gtk_widget_set_tooltip_text(pow->pwSeed,
                                            _("The seed is a number used to initialise the dice rolls generator. Reusing the same seed allows to reproduce the dice sequence. 0 is a special value that leaves GNU Backgammon use a random value."));

                pow->fChanged = 0;
                g_signal_connect(G_OBJECT(pw), "changed", G_CALLBACK(SeedChanged), &pow->fChanged);

            }

            if (!(i == RNG_BBS && !blumblum)) {
                gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pow->pwRngComboBox), gettext(aszRNG[i]));
                if (i == rngCurrent)
                    rngSelected = rngsAdded;
                rngsAdded++;
            }
        }
    }

    if (rngSelected == -1)
        rngSelected = rngsAdded - 1;
    gtk_combo_box_set_active(GTK_COMBO_BOX(pow->pwRngComboBox), rngSelected);

    /* dice manipulation */

    /* Enable dice manipulation-widget */

    pow->pwCheat =
        gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON
                                                    (pow->apwDice[NUM_RNGS - 1]), _("Dice manipulation"));

    gtk_box_pack_start(GTK_BOX(pwvbox2), pow->pwCheat, FALSE, FALSE, 0);

    /* select rolls for player */

#if GTK_CHECK_VERSION(3,0,0)
    pow->pwCheatRollBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
    pow->pwCheatRollBox = gtk_vbox_new(FALSE, 0);
#endif
    gtk_box_pack_start(GTK_BOX(pwvbox2), pow->pwCheatRollBox, FALSE, FALSE, 0);

    for (i = 0; i < 2; ++i) {
        static const char *aszCheatRoll[] = {
            N_("best"), N_("second best"), N_("third best"),
            N_("4th best"), N_("5th best"), N_("6th best"),
            N_("7th best"), N_("8th best"), N_("9th best"),
            N_("10th best"),
            N_("median"),
            N_("10th worst"),
            N_("9th worst"), N_("8th worst"), N_("7th worst"),
            N_("6th worst"), N_("5th worst"), N_("4th worst"),
            N_("third worst"), N_("second worst"), N_("worst"),
            NULL
        };
        char *sz;
        const char **ppch;

#if GTK_CHECK_VERSION(3,0,0)
        pwhbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
        pwhbox = gtk_hbox_new(FALSE, 4);
#endif
        gtk_box_pack_start(GTK_BOX(pow->pwCheatRollBox), pwhbox, TRUE, TRUE, 0);

        gtk_box_pack_start(GTK_BOX(pwhbox), gtk_label_new(_("Always roll the ")), FALSE, FALSE, 0);

        pow->apwCheatRoll[i] = gtk_combo_box_text_new();
        gtk_box_pack_start(GTK_BOX(pwhbox), pow->apwCheatRoll[i], FALSE, FALSE, 0);
        gtk_container_set_border_width(GTK_CONTAINER(pow->apwCheatRoll[i]), 1);

        for (ppch = aszCheatRoll; *ppch; ++ppch)
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pow->apwCheatRoll[i]), gettext(*ppch));

        sz = g_strdup_printf(_("roll for player %s."), ap[i].szName);

        gtk_box_pack_start(GTK_BOX(pwhbox), gtk_label_new(sz), FALSE, FALSE, 0);

        g_free(sz);

    }
    gtk_widget_set_tooltip_text(pow->pwCheat,
                                _("Now it's proven! GNU Backgammon is able to "
                                  "manipulate the dice. This is meant as a "
                                  "learning tool. Examples of use: (a) learn "
                                  "how to double aggressively after a good opening "
                                  "sequence, (b) learn to control your temper "
                                  "while things are going bad, (c) learn to play "
                                  "very good or very bad rolls, or (d) just have fun."));

    pow->pwHigherDieFirst = gtk_check_button_new_with_label(_("Show higher die on left"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwHigherDieFirst), fGUIHighDieFirst);
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwHigherDieFirst, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwHigherDieFirst,
                                _("Force the higher of the two dice to be shown " "on the left."));

    /* get the sensitivity of the widgets right */
    DiceToggled(NULL, pow);
}

static void
append_other_options(optionswidget * pow)
{
    GtkWidget *pwvbox;
    GtkWidget *pwev;
    GtkWidget *pwhbox;
#if GTK_CHECK_VERSION(3,0,0)
    GtkWidget *grid;
#else
    GtkWidget *table;
    GtkWidget *pwp;
#endif
    GtkWidget *label;
    GtkWidget *pw;
    GtkWidget *pwScale;

    /* Other options */

#if GTK_CHECK_VERSION(3,0,0)
    pwvbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(pwvbox, GTK_ALIGN_START);
    gtk_widget_set_valign(pwvbox, GTK_ALIGN_START);
    gtk_container_set_border_width(GTK_CONTAINER(pwvbox), 4);
    gtk_notebook_append_page(GTK_NOTEBOOK(pow->pwNoteBook), pwvbox, gtk_label_new(_("Other")));
#else
    pwvbox = gtk_vbox_new(FALSE, 0);
    pwp = gtk_alignment_new(0, 0, 0, 0);
    gtk_container_set_border_width(GTK_CONTAINER(pwp), 4);
    gtk_notebook_append_page(GTK_NOTEBOOK(pow->pwNoteBook), pwp, gtk_label_new(_("Other")));
    gtk_container_add(GTK_CONTAINER(pwp), pwvbox);
#endif

    pow->pwConfStart = gtk_check_button_new_with_label(_("Confirm when aborting game"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwConfStart, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwConfStart,
                                _("Ask for confirmation when ending a game or "
                                  "starting a new game would erase the record " "of the game in progress."));

    pow->pwConfOverwrite = gtk_check_button_new_with_label(_("Confirm when overwriting existing files"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwConfOverwrite, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwConfOverwrite, _("Ask for confirmation when overwriting existing files."));

    pow->pwRecordGames = gtk_check_button_new_with_label(_("Record all games"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwRecordGames, FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwRecordGames), fRecord);
    gtk_widget_set_tooltip_text(pow->pwRecordGames,
                                _("Keep the game records for all previous games in "
                                  "the current match or session.  You might want "
                                  "to disable this when playing extremely long "
                                  "matches or sessions, to save memory."));

    pwev = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(pwev), FALSE);
    gtk_box_pack_start(GTK_BOX(pwvbox), pwev, FALSE, FALSE, 0);
#if GTK_CHECK_VERSION(3,0,0)
    pwhbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
    pwhbox = gtk_hbox_new(FALSE, 4);
#endif
    gtk_container_add(GTK_CONTAINER(pwev), pwhbox);

    /* goto first game upon loading option */

    pow->pwGotoFirstGame = gtk_check_button_new_with_label(_("Goto first game when loading matches or sessions"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwGotoFirstGame, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwGotoFirstGame,
                                _("This option controls whether GNU Backgammon "
                                  "shows the board after the last move in the "
                                  "match, game, or session or whether it should "
                                  "show the first move in the first game"));

    /* display styles in game list */

    pow->pwGameListStyles = gtk_check_button_new_with_label(_("Display colours for marked moves in game list"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwGameListStyles, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwGameListStyles,
                                _("This option controls whether moves in the "
                                  "game list window are shown in different " "colours depending on their analysis"));

    /* focus on same player when moving to next marked move */

    pow->pwMarkedSamePlayer = gtk_check_button_new_with_label(_("Focus on same player when moving between marked (wrong) moves"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwMarkedSamePlayer, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwMarkedSamePlayer,
                                _("This option enables jumps between moves in the "
                                  "game list window (using the red arrows) to stay within the same player, " 
                                  "thus allowing a player to focus on his own mistakes only"));



#if GTK_CHECK_VERSION(3,0,0)
    grid = gtk_grid_new();
#else
    table = gtk_table_new(2, 3, FALSE);
#endif

    label = gtk_label_new(_("Default SGF folder:"));
#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_valign(label, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(label, TRUE);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
#else
    gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 0, 1);
#endif
    pow->pwDefaultSGFFolder = gtk_file_chooser_button_new(NULL, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    if (default_sgf_folder)
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(pow->pwDefaultSGFFolder), default_sgf_folder);
#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_hexpand(pow->pwDefaultSGFFolder, TRUE);
    gtk_grid_attach(GTK_GRID(grid), pow->pwDefaultSGFFolder, 1, 0, 1, 1);
#else
    gtk_table_attach_defaults(GTK_TABLE(table), pow->pwDefaultSGFFolder, 1, 2, 0, 1);
#endif

    label = gtk_label_new(_("Default Import folder:"));
#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_valign(label, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(label, TRUE);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
#else
    gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 1, 2);
#endif
    pow->pwDefaultImportFolder = gtk_file_chooser_button_new(NULL, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    if (default_import_folder)
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(pow->pwDefaultImportFolder), default_import_folder);
#if GTK_CHECK_VERSION(3,0,0)
    gtk_grid_attach(GTK_GRID(grid), pow->pwDefaultImportFolder, 1, 1, 1, 1);
    gtk_widget_set_hexpand(pow->pwDefaultImportFolder, TRUE);
#else
    gtk_table_attach_defaults(GTK_TABLE(table), pow->pwDefaultImportFolder, 1, 2, 1, 2);
#endif

    label = gtk_label_new(_("Default Export folder:"));
#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_valign(label, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(label, TRUE);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 1, 1);
#else
    gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 2, 3);
#endif
    pow->pwDefaultExportFolder = gtk_file_chooser_button_new(NULL, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    if (default_export_folder)
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(pow->pwDefaultExportFolder), default_export_folder);
#if GTK_CHECK_VERSION(3,0,0)
    gtk_grid_attach(GTK_GRID(grid), pow->pwDefaultExportFolder, 1, 2, 1, 1);
    gtk_widget_set_hexpand(pow->pwDefaultExportFolder, TRUE);
#else
    gtk_table_attach_defaults(GTK_TABLE(table), pow->pwDefaultExportFolder, 1, 2, 2, 3);
#endif

#if GTK_CHECK_VERSION(3,0,0)
    gtk_box_pack_start(GTK_BOX(pwvbox), grid, FALSE, FALSE, 3);
#else
    gtk_box_pack_start(GTK_BOX(pwvbox), table, FALSE, FALSE, 3);
#endif

#if GTK_CHECK_VERSION(3,0,0)
    pwhbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
    pwhbox = gtk_hbox_new(FALSE, 4);
#endif
    label = gtk_label_new(_("Web browser:"));
    gtk_box_pack_start(GTK_BOX(pwhbox), label, FALSE, FALSE, 0);

    pow->pwWebBrowser = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(pow->pwWebBrowser), get_web_browser());
    gtk_box_pack_start(GTK_BOX(pwhbox), pow->pwWebBrowser, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(pwvbox), pwhbox, FALSE, FALSE, 3);

    pwev = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(pwev), FALSE);
    gtk_box_pack_start(GTK_BOX(pwvbox), pwev, FALSE, FALSE, 0);
#if GTK_CHECK_VERSION(3,0,0)
    pw = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    pw = gtk_hbox_new(FALSE, 0);
#endif
    gtk_container_add(GTK_CONTAINER(pwev), pw);

    pow->padjCache = GTK_ADJUSTMENT(gtk_adjustment_new(GetEvalCacheSize(), 0, CACHE_SIZE_GUIMAX - 16, 1, 1, 0));
#if GTK_CHECK_VERSION(3,0,0)
    pwScale = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, pow->padjCache);
#else
    pwScale = gtk_hscale_new(pow->padjCache);
#endif
    gtk_widget_set_size_request(pwScale, 100, -1);
    gtk_scale_set_draw_value(GTK_SCALE(pwScale), TRUE);
    g_signal_connect(G_OBJECT(pwScale), "format-value", G_CALLBACK(CacheSizeString), NULL);
    gtk_scale_set_digits(GTK_SCALE(pwScale), 0);

    gtk_box_pack_start(GTK_BOX(pw), gtk_label_new(_("Cache:")), FALSE, FALSE, 8);
    gtk_box_pack_start(GTK_BOX(pw), gtk_label_new(_("Small")), FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(pw), pwScale, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(pw), gtk_label_new(_("Large")), FALSE, FALSE, 4);
    gtk_widget_set_tooltip_text(pwev,
                                _("GNU Backgammon uses a cache of previous "
                                  "evaluations to speed up processing. "
                                  "Increasing the size will help evaluations "
                                  "complete more quickly, although its benefits "
                                  "decreases relatively fast.\n"
                                  "The default value is fine for "
                                  "2-ply play and analysis. Higher values "
                                  "will help a little for higher plies "
                                  "evaluations and for rollouts."));

#if defined(USE_MULTITHREAD)
    pwev = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(pwev), FALSE);
    gtk_box_pack_start(GTK_BOX(pwvbox), pwev, FALSE, FALSE, 0);
#if GTK_CHECK_VERSION(3,0,0)
    pwhbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
    pwhbox = gtk_hbox_new(FALSE, 4);
#endif
    gtk_container_add(GTK_CONTAINER(pwev), pwhbox);

    gtk_box_pack_start(GTK_BOX(pwhbox), gtk_label_new(_("Eval Threads:")), FALSE, FALSE, 0);
    pow->padjThreads = GTK_ADJUSTMENT(gtk_adjustment_new(MT_GetNumThreads(), 1, MAX_NUMTHREADS, 1, 1, 0));
    pw = gtk_spin_button_new(GTK_ADJUSTMENT(pow->padjThreads), 1, 0);
    gtk_widget_set_size_request(GTK_WIDGET(pw), 50, -1);
    gtk_box_pack_start(GTK_BOX(pwhbox), pw, FALSE, FALSE, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(pw), TRUE);
    gtk_box_pack_start(GTK_BOX(pwhbox), gtk_label_new(_("threads")), FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pwev,
                                _("The number of threads to use in multi-threaded operations,"
                                  " this should be set to the number of logical processing units available"));
#endif
#if GTK_CHECK_VERSION(3,0,0)
    pwhbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
    pwhbox = gtk_hbox_new(FALSE, 4);
#endif
    gtk_box_pack_start(GTK_BOX(pwvbox), pwhbox, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(pwhbox), gtk_label_new(_("Auto save frequency")), FALSE, FALSE, 0);

    pow->pwAutoSaveTime = gtk_spin_button_new_with_range(1, 240, 1);
    gtk_box_pack_start(GTK_BOX(pwhbox), pow->pwAutoSaveTime, FALSE, FALSE, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(pow->pwAutoSaveTime), TRUE);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(pow->pwAutoSaveTime), nAutoSaveTime);
    gtk_widget_set_tooltip_text(pow->pwAutoSaveTime,
                                _
                                ("Set the auto save frequency. You must also enable backup during analysis and/or during rollout"));
    gtk_box_pack_start(GTK_BOX(pwhbox), gtk_label_new(_("minute(s)")), FALSE, FALSE, 0);

    pow->pwAutoSaveRollout = gtk_check_button_new_with_label(_("Auto save rollouts"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwAutoSaveRollout, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwAutoSaveRollout, _("Auto save during and after rollouts"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwAutoSaveRollout), fAutoSaveRollout);

    pow->pwAutoSaveAnalysis = gtk_check_button_new_with_label(_("Auto save analysis"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwAutoSaveAnalysis, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwAutoSaveAnalysis, _("Auto save during and after analysis of games and matches"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwAutoSaveAnalysis), fAutoSaveAnalysis);

    pow->pwAutoSaveConfirmDelete = gtk_check_button_new_with_label(_("Confirm deletion of auto saves"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwAutoSaveConfirmDelete, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwAutoSaveConfirmDelete, _("Ask before auto saves are deleted"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwAutoSaveConfirmDelete), fAutoSaveConfirmDelete);

    pow->pwCheckUpdates = gtk_check_button_new_with_label(_("Check GNU-Backgammon updates online"));
    gtk_box_pack_start(GTK_BOX(pwvbox), pow->pwCheckUpdates, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(pow->pwCheckUpdates, _("Automatically check GNU-Backgammon updates online"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwCheckUpdates), fCheckUpdates);
}

static GtkWidget *
OptionsPages(optionswidget * pow)
{
    GtkWidget *pwp;

    pow->pwNoteBook = gtk_notebook_new();
    gtk_container_set_border_width(GTK_CONTAINER(pow->pwNoteBook), 8);

    append_game_options(pow);
    append_cube_options(pow);
    append_tutor_options(pow);
    append_quiz_options(pow);
    append_display_options(pow);
    append_match_options(pow);
    append_sound_options(pow);
    append_dice_options(pow);

    /* Database options */
#if GTK_CHECK_VERSION(3,0,0)
    pwp = RelationalOptions();
#else
    pwp = gtk_alignment_new(0, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(pwp), RelationalOptions());
#endif
    gtk_container_set_border_width(GTK_CONTAINER(pwp), 4);
    relPage = gtk_notebook_append_page(GTK_NOTEBOOK(pow->pwNoteBook), pwp, gtk_label_new(_("Database")));
    relPageActivated = FALSE;

    append_other_options(pow);

    return pow->pwNoteBook;
}

static char checkupdate_sz[128];
static int checkupdate_n;

#define CHECKUPDATE(button,flag,string) \
   checkupdate_n = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( (button) ) ); \
   if ( checkupdate_n != (flag)){ \
           sprintf(checkupdate_sz, (string), checkupdate_n ? "on" : "off"); \
           UserCommand(checkupdate_sz); \
   }

static void
SetSoundSettings(void)
{
    gnubgsound i;
    outputoff();
    fGUIBeep = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(soundBeepIllegal));
    CHECKUPDATE(soundBeepIllegal, fGUIBeep, "set gui beep %s");
    CHECKUPDATE(soundsEnabled, fSound, "set sound enable %s");
    for (i = (gnubgsound) 0; i < NUM_SOUNDS; i++) {
        if (*soundDetails[i].Path)
            SetSoundFile(i, soundDetails[i].Path);
        else
            SetSoundFile(i, "");
    }
    sound_set_command(gtk_entry_get_text(GTK_ENTRY(pwSoundCommand)));
    outputon();
}

static void
OptionsOK(GtkWidget * pw, optionswidget * pow)
{
    char sz[128];
    int n, f;
    unsigned int u, i;
    gchar *filename, *command, *tmp, *newfolder;
    const gchar *new_browser;

    BoardData *bd = BOARD(pwBoard)->board_data;

    gtk_widget_hide(gtk_widget_get_toplevel(pw));

    CHECKUPDATE(pow->pwAutoBearoff, fAutoBearoff, "set automatic bearoff %s");
    CHECKUPDATE(pow->pwAutoCrawford, fAutoCrawford, "set automatic crawford %s");
    f = GetWarningEnabled(WARN_ENDGAME);
    CHECKUPDATE(pow->pwAutoEndGame, f, "set warning endgame %s");
    CHECKUPDATE(pow->pwAutoGame, fAutoGame, "set automatic game %s");
    CHECKUPDATE(pow->pwAutoRoll, fAutoRoll, "set automatic roll %s");
    CHECKUPDATE(pow->pwAutoMove, fAutoMove, "set automatic move %s");
    CHECKUPDATE(pow->pwTutor, fTutor, "set tutor mode %s");
    CHECKUPDATE(pow->pwTutorCube, fTutorCube, "set tutor cube %s");
    CHECKUPDATE(pow->pwTutorChequer, fTutorChequer, "set tutor chequer %s");
    {
        GtkTreeModel *model;
        GtkTreeIter iter;

        model = gtk_combo_box_get_model(GTK_COMBO_BOX(pow->pwTutorSkill));

        if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(pow->pwTutorSkill), &iter)) {
            gchar *selection;
            gtk_tree_model_get(model, &iter, 0, &selection, -1);

            if (!strcmp(selection, gettext(aszTutor[0]))) {     /* N_("Doubtful") */
                if (TutorSkill != SKILL_DOUBTFUL)
                    UserCommand("set tutor skill doubtful");
            } else if (!strcmp(selection, gettext(aszTutor[1]))) {      /* N_("Bad") */
                if (TutorSkill != SKILL_BAD)
                    UserCommand("set tutor skill bad");
            } else if (!strcmp(selection, gettext(aszTutor[2]))) {      /* N_("Very Bad") */
                if (TutorSkill != SKILL_VERYBAD)
                    UserCommand("set tutor skill very bad");
            } else {
                /* g_assert(FALSE); Unknown Selection, defaulting */
                if (TutorSkill != SKILL_DOUBTFUL)
                    UserCommand("set tutor skill doubtful");
            }
            g_free(selection);
        }
    }

    CHECKUPDATE(pow->pwQuiz, fUseQuiz, "set quiz allow %s");
    CHECKUPDATE(pow->pwQuizAutoAdd, fQuizAutoAdd, "set quiz autoadd %s");
    CHECKUPDATE(pow->pwQuizOnePlayer, fQuizOnePlayer, "set quiz oneplayer %s");
    CHECKUPDATE(pow->pwQuizAtMoney, fQuizAtMoney, "set quiz atmoney %s");

    {
        GtkTreeModel *model;
        GtkTreeIter iter;

        model = gtk_combo_box_get_model(GTK_COMBO_BOX(pow->pwQuizSkill));

        if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(pow->pwQuizSkill), &iter)) {
            gchar *selection;
            gtk_tree_model_get(model, &iter, 0, &selection, -1);

            if (!strcmp(selection, gettext(aszQuiz[0]))) {     /* N_("Doubtful") */
                if (QuizSkill != SKILL_DOUBTFUL)
                    UserCommand("set quiz skill doubtful");
            } else if (!strcmp(selection, gettext(aszQuiz[1]))) {      /* N_("Bad") */
                if (QuizSkill != SKILL_BAD)
                    UserCommand("set quiz skill bad");
            } else if (!strcmp(selection, gettext(aszQuiz[2]))) {      /* N_("Very Bad") */
                if (QuizSkill != SKILL_VERYBAD)
                    UserCommand("set quiz skill very bad");
            } else {
                /* g_assert(FALSE); Unknown Selection, defaulting */
                if (QuizSkill != SKILL_VERYBAD)
                    UserCommand("set quiz skill very bad");
            }
            g_free(selection);
        }
    }


    CHECKUPDATE(pow->pwCubeUsecube, fCubeUse, "set cube use %s");
    CHECKUPDATE(pow->pwCubeJacoby, fJacoby, "set jacoby %s");
    CHECKUPDATE(pow->pwCubeInvert, fInvertMET, "set invert met %s");

    CHECKUPDATE(pow->pwGameClockwise, fClockwise, "set clockwise %s");
    // CHECKUPDATE(pow->pwKeyName, fUseKeyNames, "set usekeynames %s");

    for (i = 0; i < NUM_VARIATIONS; ++i)
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pow->apwVariations[i])) && bgvDefault != (bgvariation) i) {
            sprintf(sz, "set variation %s", aszVariationCommands[i]);
            UserCommand(sz);
            break;
        }

    CHECKUPDATE(pow->pwOutputMWC, fOutputMWC, "set output mwc %s");
    CHECKUPDATE(pow->pwOutputGWC, fOutputWinPC, "set output winpc %s");
    CHECKUPDATE(pow->pwOutputMWCpst, fOutputMatchPC, "set output matchpc %s");

    if ((n = (int) gtk_adjustment_get_value(pow->padjDigits)) != fOutputDigits) {
        sprintf(sz, "set output digits %d", n);
        UserCommand(sz);
    }

    /* ... */

    CHECKUPDATE(pow->pwConfStart, fConfirmNew, "set confirm new %s");
    CHECKUPDATE(pow->pwConfOverwrite, fConfirmSave, "set confirm save %s");

    if ((u = (unsigned int) gtk_adjustment_get_value(pow->padjCubeAutomatic)) != cAutoDoubles) {
        sprintf(sz, "set automatic doubles %u", u);
        UserCommand(sz);
    }

    if ((u = (unsigned int) gtk_adjustment_get_value(pow->padjCubeBeaver)) != nBeavers) {
        sprintf(sz, "set beavers %u", u);
        UserCommand(sz);
    }

    if ((u = (unsigned int) gtk_adjustment_get_value(pow->padjLength)) != nDefaultLength) {
        sprintf(sz, "set matchlength %u", u);
        UserCommand(sz);
    }

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pow->apwDice[0]))) {     /* rng selected */
        char *selRNG = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(pow->pwRngComboBox));
        for (i = 0; i < NUM_RNGS - 3; i++) {
            if (!strcmp(selRNG, aszRNG[i]))
                break;
        }
        g_free(selRNG);
    } else {
        for (i = NUM_RNGS - 3; i < NUM_RNGS; i++) {
            if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pow->apwDice[i])))
                break;
        }
    }
    if (i < RNG_FILE && i != (unsigned int) rngCurrent) {

        static const char *set_rng_cmds[NUM_RNGS] = {
            "set rng bbs",
            "set rng isaac",
            "set rng md5",
            "set rng mersenne",
            "set rng manual",
            "set rng random.org",
            NULL,
        };

        UserCommand(set_rng_cmds[i]);

    } else if (i == RNG_FILE && i != (unsigned int) rngCurrent) {
        filename = GTKFileSelect(_("Select file with dice"), NULL, NULL, NULL, GTK_FILE_CHOOSER_ACTION_OPEN);
        if (filename) {
            command = g_strconcat("set rng file \"", filename, "\"", NULL);
            UserCommand(command);
            g_free(command);
            g_free(filename);
        }
    }

    CHECKUPDATE(pow->pwRecordGames, fRecord, "set record %s");
    CHECKUPDATE(pow->pwDisplay, fDisplay, "set display %s");

    if ((u = (unsigned int) gtk_adjustment_get_value(pow->padjCache)) != GetEvalCacheSize()) {
        SetEvalCacheSize(u);
    }
#if defined(USE_MULTITHREAD)
    if ((u = (unsigned int) gtk_adjustment_get_value(pow->padjThreads)) != MT_GetNumThreads()) {
        sprintf(sz, "set threads %u", u);
        UserCommand(sz);
    }
#endif

    if (gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(pow->pwAutoSaveTime)) != nAutoSaveTime) {
        sprintf(sz, "set autosave time %d", gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(pow->pwAutoSaveTime)));
        UserCommand(sz);
    }

    CHECKUPDATE(pow->pwAutoSaveAnalysis, fAutoSaveAnalysis, "set autosave analysis %s");
    CHECKUPDATE(pow->pwAutoSaveRollout, fAutoSaveRollout, "set autosave rollout %s");
    CHECKUPDATE(pow->pwAutoSaveConfirmDelete, fAutoSaveConfirmDelete, "set autosave confirm %s");
    CHECKUPDATE(pow->pwCheckUpdates, fCheckUpdates, "set checkupdates %s");

    if ((u = (unsigned int) gtk_adjustment_get_value(pow->padjDelay)) != nDelay) {
        sprintf(sz, "set delay %u", u);
        UserCommand(sz);
    }

    if (pow->fChanged == 1) {
        sprintf(sz, "set seed %-20.0f", gtk_adjustment_get_value(pow->padjSeed));
        UserCommand(sz);
    }

    SetSoundSettings();

    CHECKUPDATE(pow->pwIllegal, fGUIIllegal, "set gui illegal %s");
    CHECKUPDATE(pow->pwUseDiceIcon, bd->rd->fDiceArea, "set gui dicearea %s");
    CHECKUPDATE(pow->pwShowIDs, fShowIDs, "set gui showids %s");
    CHECKUPDATE(pow->pwHigherDieFirst, fGUIHighDieFirst, "set gui highdiefirst %s");
    CHECKUPDATE(pow->pwSetWindowPos, fGUISetWindowPos, "set gui windowpositions %s");
    CHECKUPDATE(pow->pwGrayEdit, fGUIGrayEdit, "set gui grayedit %s");
    CHECKUPDATE(pow->pwDragTargetHelp, fGUIDragTargetHelp, "set gui dragtargethelp %s");
    {
#if defined(USE_BOARD3D)
        if (display_is_3d(bd->rd))
            updateDiceOccPos(bd, bd->bd3d);
        else
#endif
        if (gtk_widget_get_realized(pwBoard)) {
            board_free_pixmaps(bd);
            board_create_pixmaps(pwBoard, bd);
            gtk_widget_queue_draw(bd->drawing_area);
        }
    }
    switch (gtk_combo_box_get_active(GTK_COMBO_BOX(pow->pwShowPips))) {
    case GUI_SHOW_PIPS_NONE:
        if (gui_show_pips != GUI_SHOW_PIPS_NONE)
            UserCommand("set gui showpip none");
        break;
    case GUI_SHOW_PIPS_PIPS:
        if (gui_show_pips != GUI_SHOW_PIPS_PIPS)
            UserCommand("set gui showpip pips");
        break;
    case GUI_SHOW_PIPS_EPC:
        if (gui_show_pips != GUI_SHOW_PIPS_EPC)
            UserCommand("set gui showpip epc");
        break;
    case GUI_SHOW_PIPS_WASTAGE:
        if (gui_show_pips != GUI_SHOW_PIPS_WASTAGE)
            UserCommand("set gui showpip wastage");
        break;
    default:
        g_assert_not_reached();
    }


    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pow->pwAnimateNone))
        && animGUI != ANIMATE_NONE)
        UserCommand("set gui animation none");
    else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pow->pwAnimateBlink))
             && animGUI != ANIMATE_BLINK)
        UserCommand("set gui animation blink");
    else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pow->pwAnimateSlide))
             && animGUI != ANIMATE_SLIDE)
        UserCommand("set gui animation slide");

    if ((u = (unsigned int) gtk_adjustment_get_value(pow->padjSpeed)) != nGUIAnimSpeed) {
        sprintf(sz, "set gui animation speed %u", u);
        UserCommand(sz);
    }

    /* dice manipulation */

    n = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pow->pwCheat));

    if (n != fCheat) {
        sprintf(sz, "set cheat enable %s", n ? "on" : "off");
        UserCommand(sz);
    }

    for (i = 0; i < 2; ++i) {
        u = gtk_combo_box_get_active(GTK_COMBO_BOX(pow->apwCheatRoll[i]));
        if (u != afCheatRoll[i]) {
            sprintf(sz, "set cheat player %u roll %u", i, u + 1);
            UserCommand(sz);
        }
    }

    CHECKUPDATE(pow->pwGotoFirstGame, fGotoFirstGame, "set gotofirstgame %s");
    CHECKUPDATE(pow->pwGameListStyles, fStyledGamelist, "set styledgamelist %s");
    CHECKUPDATE(pow->pwMarkedSamePlayer, fMarkedSamePlayer, "set markedsameplayer %s");
    

    newfolder = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(pow->pwDefaultSGFFolder));
    if (newfolder && (!default_sgf_folder || strcmp(newfolder, default_sgf_folder))) {
        tmp = g_strdup_printf("set sgf folder \"%s\"", newfolder);
        UserCommand(tmp);
        g_free(tmp);
    }
    g_free(newfolder);

    newfolder = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(pow->pwDefaultImportFolder));
    if (newfolder && (!default_import_folder || strcmp(newfolder, default_import_folder))) {
        tmp = g_strdup_printf("set import folder \"%s\"", newfolder);
        UserCommand(tmp);
        g_free(tmp);
    }
    g_free(newfolder);

    newfolder = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(pow->pwDefaultExportFolder));
    if (newfolder && (!default_export_folder || strcmp(newfolder, default_export_folder))) {
        tmp = g_strdup_printf("set export folder \"%s\"", newfolder);
        UserCommand(tmp);
        g_free(tmp);
    }
    g_free(newfolder);

    new_browser = gtk_entry_get_text(GTK_ENTRY(pow->pwWebBrowser));
    if (new_browser && (!get_web_browser() || strcmp(new_browser, get_web_browser()))) {
        tmp = g_strdup_printf("set browser \"%s\"", new_browser);
        UserCommand(tmp);
        g_free(tmp);
    }

    if (relPageActivated)
        RelationalSaveOptions();

    UserCommand("save settings");
    /* Destroy widget on exit */
    gtk_widget_destroy(gtk_widget_get_toplevel(pw));
}

static void
OptionsSet(optionswidget * pow)
{
    unsigned int i;
    int f;

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwAutoBearoff), fAutoBearoff);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwAutoCrawford), fAutoCrawford);
    f = GetWarningEnabled(WARN_ENDGAME);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwAutoEndGame), f);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwAutoGame), fAutoGame);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwAutoMove), fAutoMove);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwAutoRoll), fAutoRoll);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwTutor), fTutor);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwTutorCube), fTutorCube);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwTutorChequer), fTutorChequer);

    gtk_combo_box_set_active(GTK_COMBO_BOX(pow->pwTutorSkill), nTutorSkillCurrent);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwQuiz), fUseQuiz);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwQuizAutoAdd), fQuizAutoAdd);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwQuizOnePlayer), fQuizOnePlayer);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwQuizAtMoney), fQuizAtMoney);
    gtk_combo_box_set_active(GTK_COMBO_BOX(pow->pwQuizSkill), nQuizSkillCurrent);


    gtk_adjustment_set_value(pow->padjCubeBeaver, nBeavers);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwCubeUsecube), fCubeUse);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwCubeJacoby), fJacoby);
    gtk_adjustment_set_value(pow->padjCubeAutomatic, cAutoDoubles);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwCubeInvert), fInvertMET);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwGameClockwise), fClockwise);
    // gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwKeyName), fUseKeyNames ); 

    for (i = 0; i < NUM_VARIATIONS; ++i)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->apwVariations[i]), bgvDefault == (bgvariation) i);

    if (rngCurrent >= NUM_RNGS - 3)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->apwDice[rngCurrent]), TRUE);

    /* dice manipulation */

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwCheat), fCheat);

    for (i = 0; i < 2; ++i)
        gtk_combo_box_set_active(GTK_COMBO_BOX(pow->apwCheatRoll[i]), afCheatRoll[i]);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwOutputMWC), fOutputMWC);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwOutputGWC), fOutputWinPC);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwOutputMWCpst), fOutputMatchPC);

    gtk_adjustment_set_value(pow->padjDigits, fOutputDigits);


    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwConfStart), fConfirmNew);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwConfOverwrite), fConfirmSave);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwGotoFirstGame), fGotoFirstGame);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwGameListStyles), fStyledGamelist);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pow->pwMarkedSamePlayer), fMarkedSamePlayer);  
}

static void
OptionsPageChange(GtkNotebook * UNUSED(notebook), gpointer * UNUSED(page), gint tabNumber, gpointer UNUSED(data))
{
    if (tabNumber == relPage && !relPageActivated) {
        RelationalOptionsShown();
        relPageActivated = TRUE;
    }
}

extern void
GTKSetOptions(void)
{
    GtkWidget *pwDialog, *pwOptions;
    optionswidget ow;

    pwDialog = GTKCreateDialog(_("GNU Backgammon - General options"), DT_QUESTION,
                               NULL, DIALOG_FLAG_MODAL, G_CALLBACK(OptionsOK), &ow);
    gtk_container_add(GTK_CONTAINER(DialogArea(pwDialog, DA_MAIN)), pwOptions = OptionsPages(&ow));
    g_signal_connect(G_OBJECT(pwOptions), "switch-page", G_CALLBACK(OptionsPageChange), NULL);

    OptionsSet(&ow);

    GTKRunDialog(pwDialog);
}
