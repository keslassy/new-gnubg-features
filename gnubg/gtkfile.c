/*
 * Copyright (C) 2005 Ingo Macherius
 * Copyright (C) 2005-2019 the AUTHORS
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
 * $Id: gtkfile.c,v 1.78 2022/12/15 22:23:00 plm Exp $
 */

#include "config.h"
#include "backgammon.h"
#include "gtklocdefs.h"

#include <stdlib.h>
#include <string.h>
// #include <conio.h> /*opening file*/
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include "gtkfile.h"
#include "gtkgame.h"
#include "gtktoolbar.h"
#include "gtkwindows.h"
#include "file.h"
#include "util.h"

/*for picking the first file in folder...*/
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>

#define MAX_LEN 1024


static void
FilterAdd(const char *fn, const char *pt, GtkFileChooser * fc)
{
    GtkFileFilter *aff = gtk_file_filter_new();
    gtk_file_filter_set_name(aff, fn);
    gtk_file_filter_add_pattern(aff, pt);
    gtk_file_filter_add_pattern(aff, g_ascii_strup(pt, -1));
    gtk_file_chooser_add_filter(fc, aff);
}

static GtkWidget *
GnuBGFileDialog(const gchar * prompt, const gchar * folder, const gchar * name, GtkFileChooserAction action)
{
#if defined( WIN32)
    char *programdir, *pc;
#endif
    GtkWidget *fc;
    switch (action) {
    case GTK_FILE_CHOOSER_ACTION_OPEN:
        fc = gtk_file_chooser_dialog_new(prompt, NULL,
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
#if GTK_CHECK_VERSION(3,0,0)
                                         "_Open",
#else
                                         GTK_STOCK_OPEN,
#endif
                                         GTK_RESPONSE_ACCEPT,
#if GTK_CHECK_VERSION(3,0,0)
                                         "_Cancel",
#else
                                         GTK_STOCK_CANCEL,
#endif
                                         GTK_RESPONSE_CANCEL, NULL);
        break;
    case GTK_FILE_CHOOSER_ACTION_SAVE:
        fc = gtk_file_chooser_dialog_new(prompt, NULL,
                                         GTK_FILE_CHOOSER_ACTION_SAVE,
#if GTK_CHECK_VERSION(3,0,0)
                                         "_Save",
#else
                                         GTK_STOCK_SAVE,
#endif
                                         GTK_RESPONSE_ACCEPT,
#if GTK_CHECK_VERSION(3,0,0)
                                         "_Cancel",
#else
                                         GTK_STOCK_CANCEL,
#endif
                                         GTK_RESPONSE_CANCEL, NULL);
        break;
    default:
        return NULL;
    }
    gtk_window_set_modal(GTK_WINDOW(fc), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(fc), GTK_WINDOW(pwMain));

    if (folder && *folder && g_file_test(folder, G_FILE_TEST_IS_DIR))
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fc), folder);
    if (name && *name) {
        if (action == GTK_FILE_CHOOSER_ACTION_OPEN)
            gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(fc), name);
        else
            gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(fc), name);
    }

#if defined(WIN32)
    programdir = g_strdup(getDataDir());
    if ((pc = strrchr(programdir, G_DIR_SEPARATOR)) != NULL) {
        char *tmp;

        *pc = '\0';

        tmp = g_build_filename(programdir, "GridGammon", "SaveGame", NULL);
        gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(fc), tmp, NULL);
        g_free(tmp);

        tmp = g_build_filename(programdir, "GammonEmpire", "savedgames", NULL);
        gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(fc), tmp, NULL);
        g_free(tmp);

        tmp = g_build_filename(programdir, "PartyGaming", "PartyGammon", "SavedGames", NULL);
        gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(fc), tmp, NULL);
        g_free(tmp);

        tmp = g_build_filename(programdir, "Play65", "savedgames", NULL);
        gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(fc), tmp, NULL);
        g_free(tmp);

        tmp = g_build_filename(programdir, "TrueMoneyGames", "SavedGames", NULL);
        gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(fc), tmp, NULL);
        g_free(tmp);
    }
    g_free(programdir);
#endif
    return fc;
}

extern char *
GTKFileSelect(const gchar * prompt, const gchar * extension, const gchar * folder,
              const gchar * name, GtkFileChooserAction action)
{
    gchar *filename = NULL;
    GtkWidget *fc = GnuBGFileDialog(prompt, folder, name, action);

    if (extension && *extension) {
        gchar *sz = g_strdup_printf(_("Supported files (%s)"), extension);

        FilterAdd(sz, extension, GTK_FILE_CHOOSER(fc));
        FilterAdd(_("All Files"), "*", GTK_FILE_CHOOSER(fc));
        g_free(sz);
    }

    if (gtk_dialog_run(GTK_DIALOG(fc)) == GTK_RESPONSE_ACCEPT)
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fc));

    gtk_widget_destroy(fc);

    return filename;
}

typedef struct {
    GtkWidget *fc, *description, *mgp, *upext;
} SaveOptions;

static void
SaveOptionsCallBack(GtkWidget * UNUSED(pw), SaveOptions * pso)
{
    gint type, mgp;

    type = gtk_combo_box_get_active(GTK_COMBO_BOX(pso->description));
    mgp = gtk_combo_box_get_active(GTK_COMBO_BOX(pso->mgp));

    gtk_dialog_set_response_sensitive(GTK_DIALOG(pso->fc), GTK_RESPONSE_ACCEPT, export_format[type].exports[mgp]);

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pso->upext))) {
        gchar *fn;

        if ((fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(pso->fc))) != NULL) {
            gchar *fnname, *fndir;

            DisectPath(fn, export_format[type].extension, &fnname, &fndir);
            gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(pso->fc), fndir);
            gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(pso->fc), fnname);

            g_free(fnname);
            g_free(fndir);

            g_free(fn);
        }
    }
}

static void
SaveCommon(guint f, gchar * prompt)
{
    GtkWidget *hbox;
    guint i;
    gint j, type;
    SaveOptions so;
    static ExportType last_export_type = EXPORT_SGF;
    static gint last_export_mgp = 0;
    static gchar *last_save_folder = NULL;
    static gchar *last_export_folder = NULL;
    gchar *fn = GetFilename(TRUE, (f == 1) ? EXPORT_SGF : last_export_type);
    gchar *folder = NULL;
    const gchar *mgp_text[3] = { "match", "game", "position" };

    if (f == 1)
        folder = last_save_folder ? last_save_folder : default_sgf_folder;
    else
        folder = last_export_folder ? last_export_folder : default_export_folder;

    so.fc = GnuBGFileDialog(prompt, folder, fn, GTK_FILE_CHOOSER_ACTION_SAVE);
    g_free(fn);

    so.description = gtk_combo_box_text_new();
    for (j = i = 0; i < f; ++i) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(so.description), export_format[i].description);
        if (i == last_export_type)
            gtk_combo_box_set_active(GTK_COMBO_BOX(so.description), j);
        j++;
    }
    if (f == 1)
        gtk_combo_box_set_active(GTK_COMBO_BOX(so.description), 0);

    so.mgp = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(so.mgp), _("match"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(so.mgp), _("game"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(so.mgp), _("position"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(so.mgp), last_export_mgp);

    so.upext = gtk_check_button_new_with_label(_("Update extension"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(so.upext), TRUE);

#if GTK_CHECK_VERSION(3,0,0)
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
#else
    hbox = gtk_hbox_new(FALSE, 10);
#endif
    gtk_box_pack_start(GTK_BOX(hbox), so.mgp, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), so.description, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), so.upext, TRUE, TRUE, 0);
    gtk_widget_show_all(hbox);
    gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(so.fc), hbox);

    g_signal_connect(G_OBJECT(so.description), "changed", G_CALLBACK(SaveOptionsCallBack), &so);
    g_signal_connect(G_OBJECT(so.mgp), "changed", G_CALLBACK(SaveOptionsCallBack), &so);

    SaveOptionsCallBack(so.fc, &so);

    if (gtk_dialog_run(GTK_DIALOG(so.fc)) == GTK_RESPONSE_ACCEPT) {
        SaveOptionsCallBack(so.fc, &so);
        fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(so.fc));
        if (fn) {
            const gchar *et = mgp_text[gtk_combo_box_get_active(GTK_COMBO_BOX(so.mgp))];
            gchar *cmd = NULL;
            type = gtk_combo_box_get_active(GTK_COMBO_BOX(so.description));
            if (type == EXPORT_SGF)
                cmd = g_strdup_printf("save %s \"%s\"", et, fn);
            else
                cmd = g_strdup_printf("export %s %s \"%s\"", et, export_format[type].clname, fn);
            last_export_type = (ExportType) type;
            last_export_mgp = gtk_combo_box_get_active(GTK_COMBO_BOX(so.mgp));
            g_free(last_export_folder);
            last_export_folder = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(so.fc));
            UserCommand(cmd);
            UserCommand("save settings");
            g_free(cmd);
        }
        g_free(fn);
    }
    gtk_widget_destroy(so.fc);
}

static ImportType lastOpenType;
static GtkWidget *openButton, *selFileType;
static int autoOpen;

static void
selection_changed_cb(GtkFileChooser * file_chooser, void *UNUSED(notused))
{
    const char *label;
    char *buf;
    char *filename = gtk_file_chooser_get_filename(file_chooser);
    FilePreviewData *fpd = ReadFilePreview(filename);
    int openable = FALSE;
    if (!fpd) {
        lastOpenType = N_IMPORT_TYPES;
        label = "";
    } else {
        lastOpenType = fpd->type;
        label = gettext((import_format[lastOpenType]).description);
        g_free(fpd);
        if (lastOpenType != N_IMPORT_TYPES || !autoOpen)
            openable = TRUE;
    }
    buf = g_strdup_printf("<b>%s</b>", label);
    gtk_label_set_markup(GTK_LABEL(selFileType), buf);
    g_free(buf);

    gtk_widget_set_sensitive(openButton, openable);
}

static void
add_import_filters(GtkFileChooser * fc)
{
    GtkFileFilter *aff = gtk_file_filter_new();
    gint i;
    gchar *sg;

    gtk_file_filter_set_name(aff, _("Supported files"));
    for (i = 0; i < N_IMPORT_TYPES; ++i) {
        sg = g_strdup_printf("*%s", import_format[i].extension);
        gtk_file_filter_add_pattern(aff, sg);
        gtk_file_filter_add_pattern(aff, g_ascii_strup(sg, -1));
        g_free(sg);
    }
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(fc), aff);

    FilterAdd(_("All Files"), "*", GTK_FILE_CHOOSER(fc));

    for (i = 0; i < N_IMPORT_TYPES; ++i) {
        sg = g_strdup_printf("*%s", import_format[i].extension);
        FilterAdd(import_format[i].description, sg, GTK_FILE_CHOOSER(fc));
        g_free(sg);
    }
}

static void
OpenTypeChanged(GtkComboBox * widget, gpointer fc)
{
    autoOpen = (gtk_combo_box_get_active(widget) == 0);
    selection_changed_cb(fc, NULL);
}

static GtkWidget *
import_types_combo(void)
{
    gint i;
    GtkWidget *type_combo = gtk_combo_box_text_new();

    /* Default option 'automatic' */
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), _("Automatic"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(type_combo), 0);

    for (i = 0; i < N_IMPORT_TYPES; ++i)
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), import_format[i].description);

    /* Extra option 'command file' */
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), _("GNUbg Command file"));

    return type_combo;
}

static void
do_import_file(gint import_type, gchar * fn)
{
    gchar *cmd = NULL;

    if (!fn)
        return;
    if (import_type == N_IMPORT_TYPES)
        outputerrf(_("Unable to import. Unrecognized file type"));
    else if (import_type == IMPORT_SGF) {
        cmd = g_strdup_printf("load match \"%s\"", fn);
    } else {
        cmd = g_strdup_printf("import %s \"%s\"", import_format[import_type].clname, fn);
    }
    if (cmd) {
        if (ToolbarIsEditing(NULL))
            click_edit();
        UserCommand(cmd);
    }
    g_free(cmd);
}

extern void
GTKOpen(gpointer UNUSED(p), guint UNUSED(n), GtkWidget * UNUSED(pw))
{
    GtkWidget *fc;
    GtkWidget *type_combo, *box, *box2;
    gchar *folder = NULL;
    gint import_type;
    static gchar *last_import_folder = NULL;
    static gchar *last_import_file = NULL;

    folder = last_import_folder ? last_import_folder : default_import_folder;

    fc = GnuBGFileDialog(_("Open backgammon file"), folder, last_import_file, GTK_FILE_CHOOSER_ACTION_OPEN);

    type_combo = import_types_combo();
    g_signal_connect(type_combo, "changed", G_CALLBACK(OpenTypeChanged), fc);

#if GTK_CHECK_VERSION(3,0,0)
    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    box2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    box = gtk_hbox_new(FALSE, 0);
    box2 = gtk_hbox_new(FALSE, 0);
#endif
    gtk_box_pack_start(GTK_BOX(box), box2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box2), gtk_label_new(_("Open as:")), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box2), type_combo, FALSE, FALSE, 0);

#if GTK_CHECK_VERSION(3,0,0)
    box2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    box2 = gtk_hbox_new(FALSE, 0);
#endif
    gtk_box_pack_start(GTK_BOX(box), box2, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(box2), gtk_label_new(_("Selected file type: ")), FALSE, FALSE, 0);
    selFileType = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(box2), selFileType, FALSE, FALSE, 0);
    gtk_widget_show_all(box);

    g_signal_connect(GTK_FILE_CHOOSER(fc), "selection-changed", G_CALLBACK(selection_changed_cb), NULL);

    gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(fc), box);

    add_import_filters(GTK_FILE_CHOOSER(fc));
    openButton = DialogArea(fc, DA_OK);
    autoOpen = TRUE;

    if (gtk_dialog_run(GTK_DIALOG(fc)) == GTK_RESPONSE_ACCEPT) {
        gchar *fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fc));
	
        import_type = gtk_combo_box_get_active(GTK_COMBO_BOX(type_combo));
        if (import_type == 0) { /* Type automatically based on file */
            do_import_file((gint)lastOpenType, fn);
	    g_free(last_import_file);
	    last_import_file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fc));
        } else {
            import_type--;      /* Ignore auto option */
            if (import_type == N_IMPORT_TYPES) {        /* Load command file */
                gchar *cmd = NULL;

                cmd = g_strdup_printf("load commands \"%s\"", fn);
                if (cmd) {
                    UserCommand(cmd);
                    UserCommand("save settings");
                }
                g_free(cmd);
            } else {            /* Import as specific type */
                do_import_file(import_type, fn);
		g_free(last_import_file);
		last_import_file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fc));
            }
        }

        g_free(last_import_folder);
        last_import_folder = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(fc));
        g_free(fn);
    }
    gtk_widget_destroy(fc);
}

extern void
GTKCommandsOpen(gpointer UNUSED(p), guint UNUSED(n), GtkWidget * UNUSED(pw))
{
    gchar *filename = NULL, *cmd = NULL;
    GtkWidget *fc = GnuBGFileDialog(_("Open Commands file"), NULL, NULL, GTK_FILE_CHOOSER_ACTION_OPEN);

    if (gtk_dialog_run(GTK_DIALOG(fc)) == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fc));
        cmd = g_strdup_printf("load commands \"%s\"", filename);
        if (cmd) {
            UserCommand(cmd);
            UserCommand("save settings");
        }
        g_free(cmd);
        g_free(filename);
    }
    gtk_widget_destroy(fc);
}

extern void
GTKSave(gpointer UNUSED(p), guint UNUSED(n), GtkWidget * UNUSED(pw))
{
    SaveCommon(N_EXPORT_TYPES, _("Save or export to file"));
}

enum {
    COL_RESULT = 0,
    COL_DESC,
    COL_FILE,
    COL_PATH,
    NUM_COLS
};

static gboolean
batch_create_save(gchar * filename, gchar ** save, char **result)
{
    gchar *file;
    gchar *folder;
    gchar *dir;
    DisectPath(filename, NULL, &file, &folder);
    dir = g_build_filename(folder, "analysed", NULL);
    g_free(folder);

    if (!g_file_test(dir, G_FILE_TEST_EXISTS))
        g_mkdir(dir, 0700);

    if (!g_file_test(dir, G_FILE_TEST_IS_DIR)) {
        g_free(dir);
        if (result)
            *result = _("Failed to make directory");
        return FALSE;
    }

    *save = g_strconcat(dir, G_DIR_SEPARATOR_S, file, ".sgf", NULL);
    g_free(file);
    g_free(dir);
    return TRUE;
}

static gboolean
batch_analyse(gchar * filename, char **result, gboolean add_to_db, gboolean add_incdata_to_db)
{
    gchar *cmd;
    gchar *save = NULL;
    gboolean fMatchAnalysed;

    if (!batch_create_save(filename, &save, result))
        return FALSE;

    printf("save %s\n", save);

    if (g_file_test((save), G_FILE_TEST_EXISTS)) {
        *result = _("Pre-existing");
        g_free(save);
        return TRUE;
    }


    g_free(szCurrentFileName);
    szCurrentFileName = NULL;
    cmd = g_strdup_printf("import auto \"%s\"", filename);
    UserCommand(cmd);
    g_free(cmd);
    if (!szCurrentFileName) {
        *result = _("Failed import");
        g_free(save);
        return FALSE;
    }

    UserCommand("analysis clear match");
    UserCommand("analyse match");

    if (fMatchCancelled) {
        *result = _("Cancelled");
        g_free(save);
        fInterrupt = FALSE;
        fMatchCancelled = FALSE;
        return FALSE;
    }

    cmd = g_strdup_printf("save match \"%s\"", save);
    UserCommand(cmd);
    g_free(cmd);
    g_free(save);

    fMatchAnalysed = MatchAnalysed();
    if (add_to_db && ((!fMatchAnalysed && add_incdata_to_db) || fMatchAnalysed)) {
        cmd = g_strdup("relational add match quiet");
        UserCommand(cmd);
        g_free(cmd);
    }
    *result = _("Done");
    return TRUE;
}

static void
batch_do_all(gpointer batch_model, gboolean add_to_db, gboolean add_incdata_to_db)
{
    gchar *result;
    GtkTreeIter iter;
    gboolean valid;

    fMatchCancelled = FALSE;

    g_return_if_fail(batch_model != NULL);

    valid = gtk_tree_model_get_iter_first(batch_model, &iter);

    while (valid) {
        gchar *filename;
        gint cancelled;
        gtk_tree_model_get(batch_model, &iter, COL_PATH, &filename, -1);
        //outputerrf("filename=%s\n", filename);
        batch_analyse(filename, &result, add_to_db, add_incdata_to_db);
        gtk_list_store_set(batch_model, &iter, COL_RESULT, result, -1);

        cancelled = GPOINTER_TO_INT(g_object_get_data(batch_model, "cancelled"));
        if (cancelled)
            break;
        valid = gtk_tree_model_iter_next(batch_model, &iter);
    }
}

static void
batch_cancel(GtkWidget * UNUSED(pw), gpointer UNUSED(model))
{
    pwGrab = pwOldGrab;
    fInterrupt = TRUE;
    fMatchCancelled = TRUE;
}


static void
batch_stop(GtkWidget * UNUSED(pw), gpointer p)
{
    fMatchCancelled = TRUE;
    fInterrupt = TRUE;
    g_object_set_data(G_OBJECT(p), "cancelled", GINT_TO_POINTER(1));
}

static void
batch_skip_file(GtkWidget * UNUSED(pw), gpointer UNUSED(p))
{
    fMatchCancelled = TRUE;
    fInterrupt = TRUE;
}

static GtkTreeModel *
batch_create_model(GSList * filenames)
{
    GtkListStore *store;
    GtkTreeIter tree_iter;
    GSList *iter;

    store = gtk_list_store_new(NUM_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    for (iter = filenames; iter != NULL; iter = iter->next) {
        char *desc;
        char *folder;
        char *file;
        char *filename = (char *) iter->data;
        FilePreviewData *fpd;

        gtk_list_store_append(store, &tree_iter);

        fpd = ReadFilePreview(filename);
        if (fpd)
            desc = g_strdup(import_format[fpd->type].description);
        else
            desc = g_strdup(import_format[N_IMPORT_TYPES].description);
        g_free(fpd);
        gtk_list_store_set(store, &tree_iter, COL_DESC, desc, -1);
        g_free(desc);

        DisectPath(filename, NULL, &file, &folder);
        gtk_list_store_set(store, &tree_iter, COL_FILE, file, -1);
        g_free(file);
        g_free(folder);

        gtk_list_store_set(store, &tree_iter, COL_PATH, filename, -1);

    }
    return GTK_TREE_MODEL(store);
}

static GtkWidget *
batch_create_view(GSList * filenames)
{
    GtkCellRenderer *renderer;
    GtkTreeModel *model;
    GtkWidget *view;

    view = gtk_tree_view_new();

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view),
                                                -1, _("Result"), renderer, "text", COL_RESULT, NULL);

    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, Q_("fileType|Type"), renderer, "text", COL_DESC, NULL);

    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, _("File"), renderer, "text", COL_FILE, NULL);
    model = batch_create_model(filenames);

    gtk_tree_view_set_model(GTK_TREE_VIEW(view), model);

    g_object_unref(model);
    return view;
}

static void
batch_open_selected_file(GtkTreeView * view)
{
    gchar *save;
    gchar *file;
    gchar *cmd;
    GtkTreeModel *model;
    GtkTreeIter selected_iter;
    GtkTreeSelection *sel = gtk_tree_view_get_selection(view);

    if (gtk_tree_selection_count_selected_rows(sel) != 1)
        return;

    model = gtk_tree_view_get_model(view);
    gtk_tree_selection_get_selected(sel, &model, &selected_iter);
    gtk_tree_model_get(model, &selected_iter, COL_PATH, &file, -1);

    if (!batch_create_save(file, &save, NULL))
        return;

    cmd = g_strdup_printf("load match \"%s\"", save);
    UserCommand(cmd);
    g_free(save);
    g_free(cmd);
}

static void
batch_row_activate(GtkTreeView * view,
                   GtkTreePath * UNUSED(path), GtkTreeViewColumn * UNUSED(col), gpointer UNUSED(userdata))
{
    batch_open_selected_file(view);
}

static void
batch_row_open(GtkWidget * UNUSED(widget), GtkTreeView * view)
{
    batch_open_selected_file(view);
}

static void
batch_create_dialog_and_run(GSList * filenames, gboolean add_to_db)
{
    GtkWidget *dialog;
    GtkWidget *buttons;
    GtkWidget *view;
    GtkWidget *ok_button;
    GtkWidget *skip_button;
    GtkWidget *stop_button;
    GtkWidget *open_button;
    GtkTreeModel *model;
    GtkWidget *sw;
    gboolean add_incdata_to_db = FALSE;

    if (add_to_db && (!fAnalyseMove || !fAnalyseCube || !fAnalyseDice || !afAnalysePlayers[0] || !afAnalysePlayers[1])) {
        add_incdata_to_db = GetInputYN(_("Your current analysis settings will produce incomplete statistics.\n"
                                         "Do you still want to add these matches to the database?"));
    }

    view = batch_create_view(filenames);
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
    g_object_set_data(G_OBJECT(model), "cancelled", GINT_TO_POINTER(0));

    // if(!fBackgroundAnalysis) {
        dialog = GTKCreateDialog(_("Batch analyse files"), DT_INFO, NULL,
                             DIALOG_FLAG_MODAL | DIALOG_FLAG_MINMAXBUTTONS | DIALOG_FLAG_NOTIDY, NULL, NULL);
    // } else {
    //     dialog = GTKCreateDialog(_("Batch analyse files"), 
    //                          DT_INFO, NULL, DIALOG_FLAG_MINMAXBUTTONS | DIALOG_FLAG_NOTIDY, NULL, NULL);
    //     // dialog = GTKCreateDialog(_("Batch analyse files"), 
    //     //                      DT_INFO, NULL, DIALOG_FLAG_NONE, NULL, NULL);
    // }

    gtk_window_set_default_size(GTK_WINDOW(dialog), -1, 400);

    pwOldGrab = pwGrab;
    pwGrab = dialog;

    g_signal_connect(G_OBJECT(dialog), "destroy", G_CALLBACK(batch_cancel), model);


    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(sw), view);
    gtk_container_add(GTK_CONTAINER(DialogArea(dialog, DA_MAIN)), sw);

    buttons = DialogArea(dialog, DA_BUTTONS);

    ok_button = DialogArea(dialog, DA_OK);
    gtk_widget_set_sensitive(ok_button, FALSE);

    skip_button = gtk_button_new_with_label(_("Skip"));
    stop_button = gtk_button_new_with_label(_("Stop"));
    open_button = gtk_button_new_with_label(_("Open"));

    gtk_container_add(GTK_CONTAINER(buttons), skip_button);
    gtk_container_add(GTK_CONTAINER(buttons), stop_button);
    gtk_container_add(GTK_CONTAINER(buttons), open_button);

    g_signal_connect(G_OBJECT(skip_button), "clicked", G_CALLBACK(batch_skip_file), model);
    g_signal_connect(G_OBJECT(stop_button), "clicked", G_CALLBACK(batch_stop), model);
    gtk_widget_show_all(dialog);

    batch_do_all(model, add_to_db, add_incdata_to_db);

    g_signal_connect(open_button, "clicked", G_CALLBACK(batch_row_open), view);
    g_signal_connect(view, "row-activated", G_CALLBACK(batch_row_activate), view);

    gtk_widget_set_sensitive(ok_button, TRUE);
    gtk_widget_set_sensitive(open_button, TRUE);
    gtk_widget_set_sensitive(skip_button, FALSE);
    gtk_widget_set_sensitive(stop_button, FALSE);

    gtk_window_set_modal(GTK_WINDOW(dialog), FALSE);
}

/* functions to find latest file in folder */


void recentByModification(const char* path, char* recent){
    char buffer[MAX_LEN];
    FilePreviewData *fdp;
    struct dirent* entry;
    time_t recenttime = 0;
    struct stat statbuf;
    DIR* dir  = opendir(path);

    if (dir) {
        while (NULL != (entry = readdir(dir))) {
           // outputerrf(_("`%s' ... looking at file....: %s, time: %lld"), entry->d_name, recent, (long long)recenttime);

            /* we first check that it's a file*/
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strncmp(entry->d_name, "/", 1) == 0 || strncmp(entry->d_name, "\\", 1) == 0)
                continue;
                /* check file type when DIRENT is defined; could use stat if not*/
#ifdef _DIRENT_HAVE_D_TYPE
            if (entry->d_type == DT_REG) 
// #else
//             DIR* dir2 = opendir(entry);
//             if(dir2==NULL) {
#endif 
            {    
                /* we then check that it's more recent than what we've seen so far*/
                sprintf(buffer, "%s/%s", path, entry->d_name);
                stat(buffer, &statbuf);            
                if (statbuf.st_mtime > recenttime) {
                    /* next we check that it's a correct file format*/
                    fdp = ReadFilePreview(buffer);
                    if (!fdp) {
                        //outputerrf(_("`%s' is not a backgammon file (especially %s)... looking at file....: %s, time: %lld"), buffer,entry->d_name, recent, (long long) recenttime);
                        g_free(fdp);
                        continue;
                    } else if (fdp->type == N_IMPORT_TYPES) {
                        //outputerrf(_("The format of '%s' is not recognized  (especially %s)"), buffer,entry->d_name);
                        g_free(fdp);
                        continue;
                    } else {
                        /* finally it passed all the checks, we keep it for now in the 
                        char * recent 
                        */
                        strncpy(recent, buffer, MAX_LEN);
                        // strncpy(recent, entry->d_name, MAX_LEN);
                        recenttime = statbuf.st_mtime;
                        // g_message("looking at file....: %s, time: %lld\n", recent, (long long) recenttime);
                        g_free(fdp);
                    }
                }
            }
        }
        closedir(dir);
    } else {
        outputerrf(_("Unable to read the directory"));
    }
}

extern void
GTKAnalyzeCurrent(void)
{
    /*analyze match*/
    UserCommand("analyse match");
    if(fAutoDB) {
        /*add match to db*/
        CommandRelationalAddMatch(NULL);
    }
    /*show stats panel*/
    UserCommand("show statistics match");
    return;
}

extern void
AnalyzeSingleFile(void)
{
    gchar *folder = NULL;
    gchar *filename = NULL;
    GtkWidget *fc;
    static gchar *last_folder = NULL;

    folder = last_folder ? last_folder : default_import_folder;

    /* now select a file; could also use GTKFileSelect() */
    fc = GnuBGFileDialog(_("Select file to analyse"), folder, NULL, GTK_FILE_CHOOSER_ACTION_OPEN);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(fc), FALSE);
    add_import_filters(GTK_FILE_CHOOSER(fc));

    if (gtk_dialog_run(GTK_DIALOG(fc)) == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fc));
    }
    if (filename) {
        last_folder = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(fc));
        gtk_widget_destroy(fc);

        // char buffer[MAX_LEN];
        // sprintf(buffer, "%s/%s", last_folder, filename);
        // // if (!get_input_discard())
        // //     return;

        /*open this file*/
        gchar* cmd;
        cmd = g_strdup_printf("import auto \"%s\"", filename);
        UserCommand(cmd);
        g_free(cmd);
        ////outputerrf("filename=%s\n", filename);
        //CommandImportAuto(filename);
        g_free(filename);

        /*analyze match*/
        UserCommand("analyse match");
        if(fAutoDB) {
            /*add match to db*/
            CommandRelationalAddMatch(NULL);
        }
        /*show stats panel*/
        UserCommand("show statistics match");
        return;

    } else
        gtk_widget_destroy(fc);
}


void
SmartAnalyze(void)
{
    gchar *folder = NULL;
    char recent[MAX_LEN];

    folder = default_import_folder ? default_import_folder : ".";
    // g_message("folder=%s\n", folder);

    /* find most recent file in the folder and write its name (in char recent[])*/
    recentByModification(folder, recent);

    /*open this file*/
    gchar* cmd;
    cmd = g_strdup_printf("import auto \"%s\"", recent);
    UserCommand(cmd);
    g_free(cmd);

    //outputerrf("recent=%s\n", recent);
    //CommandImportAuto(recent);

    /*analyze match*/
    UserCommand2("analyse match");
    if(fAutoDB) {
        /*add match to db*/
        CommandRelationalAddMatch(NULL);
    }
    /*show stats panel*/
    UserCommand2("show statistics match");
    return;
}

/* ************ QUIZ ************************** */
/*
The quiz mode defines a full loop, from picking the position category to play with,
to playing, to getting the hint screen, to starting again. Formally, here are the 
functions in the loop:
QuizConsole->StartQuiz->OpenQuizPositionsFile, LoadPositionAndStart
    ->CommandSetGNUBgID->(play)->MoveListUpdate->qUpdate(play error)
    ->SaveFullPositionFile [screen: could play again] -> HintOK->BackFromHint
    ->either LoadPositionAndStart or QuizConsole
*/

 /*extern:right-click menu, to be updated in quiz mode; used in gtkgamelist.c*/
GtkWidget *pQuizMenu;
GtkWidget *pwQuiz = NULL;
// static void QuizConsole(void);

#define WIDTH   640
#define HEIGHT  480
#define MAX_ROWS 3000
#define MAX_ROW_CHARS 1024
#define ERROR_DECAY 0.6
#define SMALL_ERROR 0.2
// #define BGCOLOR "#EDF5FF"

// static char positionsFolder [200];
// char * tempPath= g_build_filename(szHomeDirectory, "quiz", NULL);
// strcpy(positionsFolder,tempPath);
    // 
    // if (g_file_test(path, G_FILE_TEST_IS_REGULAR)) {
// "/home/isaac/g/new-gnubg-features/gnubg/quiz";
// static const char positionsFolder2 []="~/g/new-gnubg-features/gnubg/quiz/";

//static char positionsFileFullPath []="./quiz/positions.csv";
// static char * positionsFileFullPath;
// static char * currentCategory;
int currentCategoryIndex=-1; /*extern*/
static quiz q[MAX_ROWS]; 
static int qLength=0;
static int iOpt=-1; 
static int iOptCounter=0; 
// quiz qNow={"\0",0,0,0.0}; /*extern*/
int counterForFile=0; /*extern*/
float latestErrorInQuiz=-1.0; /*extern*/
// char * name0BeforeQuiz=NULL; //[MAX_NAME_LEN+1];
// char * name1BeforeQuiz=NULL; //[MAX_NAME_LEN+1];
char name0BeforeQuiz[MAX_NAME_LEN];
char name1BeforeQuiz[MAX_NAME_LEN];
playertype type0BeforeQuiz;

// /*initialization: maybe not needed, using GetCategory->InitCategoryArray */
// static const categorytype emptyCategory={NULL,0,NULL,NULL};
categorytype categories[MAX_POS_CATEGORIES]; //={""};
int numCategories;//=0
// int numPositionsInCategory[MAX_POS_CATEGORIES]={0}; /* array with #positions per category file*/

enum {COLUMN_INDEX, COLUMN_STRING, COLUMN_INT, N_COLUMNS};

/* the following function:
- updates positionsFileFullPath,
- opens the corrsponding file, 
- and saves the parsing results in the categories static array 
*/
int OpenQuizPositionsFile(const int index)
{
    char row[MAX_ROW_CHARS];
    int i = -2;
    int column = 0;

    // positionsFileFullPath = g_strconcat(positionsFolder, G_DIR_SEPARATOR_S, category, ".csv", NULL);
    // g_message("fullPath=%s",positionsFileFullPath);

	FILE* fp = fopen(categories[index].path, "r");

    if(fp==NULL) {
        char buf[100];
        sprintf(buf,_("Error: Problem reading file %s"),categories[index].path);
        GTKMessage(_(buf), DT_INFO);
        return FALSE;
    }

    /* based on standard csv program from geeksforgeeks*/
    while (fgets(row, MAX_ROW_CHARS, fp)) {
        column = 0;
        i++;

        // To avoid printing of column
        // names in file can be changed
        // according to need
        if (i == -1)
            continue;

        // Splitting the data
        char* token = strtok(row, ", ");
        // g_message("i:%d, token:%s",i,token);

        while (token) {
            // // Column 1
            // if (column == 0) {
            //     printf("Position:");
            // }
            // // Column 2
            // if (column == 1) {
            //     printf("\tError:");
            // }
            // // Column 3
            // if (column == 2) {
            //     printf("\tTime:");
            // }
            // printf("%s", token);
            if (column == 0) {
                // g_message("i=%d",i);
                // q[i].position = malloc(50 * sizeof(char));
                strcpy(q[i].position,token); 
        // g_message("read new line %d: %s\n", i, q[i].position);
            } else if (column == 1) {
                q[i].player=strtol(token, NULL, 10); //atof(token);
            } else if (column == 2) {
                q[i].ewmaError=atof(token);
        // g_message("read new line %d: %s, %.3f\n", i, q[i].position, q[i].ewmaError);
            } else if (column == 3) {
                q[i].lastSeen=strtol(token, NULL, 10);
                // q[i].lastSeen=strtoimax(sz, NULL, 10);
        // g_message("read new line %d: %s, %.3f, %ld\n", i, q[i].position, q[i].ewmaError, q[i].lastSeen);
            } 
            // printf("%s", token);


            // g_message("i:%d,col=%d, token:%s",i,column,token);
            token = strtok(NULL, ", ");
            column++;
        }
        // printf("\n");
        //    intmax_t seconds = strtoimax(sz, NULL, 10);
        // g_message("read new line %d: %s, %.5f, %ld\n", i, q[i].position, q[i].ewmaError, q[i].lastSeen);

    }
    qLength=i+1;
    // g_message("qLength=%d ==??? categories[index].number=%d ",qLength,categories[index].number);
    g_assert(qLength==categories[index].number);
    // Close the file
    fclose(fp);
    if(qLength<1) {
        GTKMessage(_("Error: no positions in file?"), DT_INFO);
        // return FALSE;
    } 
	return qLength;
}
/* in backgammon.h, we define: */
// typedef struct {
//     char * position; 
//     double ewmaError; 
//     long int lastSeen; 
// } quiz;


static void writeQuizHeader (FILE* fp) {
        fprintf(fp, "position, player, ewmaError, lastSeen\n");
}
static void writeQuizLine (quiz q, FILE* fp) {
        fprintf(fp, "%s, %d, %.5f, %ld\n", q.position, q.player, q.ewmaError, q.lastSeen);
}
extern int writeQuizLineFull (quiz q, char * file, int quiet) {
    /*test that file exists, else write header*/
    if(!g_file_test(file, G_FILE_TEST_EXISTS )){
        g_message("writeQuizLineFull file doesn't exist: %s",file);
        	FILE* fp0 = fopen(file, "w");        
            if(!fp0){
                g_message("writeQuizLineFull file had a pointer problem: fp0-> %s",file);
                if(!quiet) {
                    char buf[100];
                    sprintf(buf,_("Error: cannot write into file %s"),file);
                    GTKMessage(_(buf), DT_INFO);
                }
                return FALSE;
            }
            writeQuizHeader(fp0);
            fclose(fp0);
    }
    if(!g_file_test(file, G_FILE_TEST_IS_REGULAR)){
        g_message("writeQuizLineFull problem with file %s",file);
        if(!quiet) {
            char buf[100];
            sprintf(buf,_("Error: problem with file %s, not a regular file?"),file);
            GTKMessage(_(buf), DT_INFO);
        }
        return FALSE;
    }
    FILE* fp = fopen(file, "r+");
    if(!fp){
        g_message("writeQuizLineFull cannot read/write: fp-> %s",file);
        if(!quiet) {
            char buf[100];
            sprintf(buf,_("Error: cannot read/write file %s"),file);
            GTKMessage(_(buf), DT_INFO);
        }
        return FALSE;
    }
    char line[MAX_ROW_CHARS];
    int lineCounter=-2;
    while (fgets(line, sizeof(line), fp)){
        lineCounter++;
        // g_message("%s->%s?",line, q.position);
        if (strstr(line, q.position)!=NULL) {
            // g_message("writeQuizLineFull found a match for position");
            if(!quiet) {
                char buf[100];
                sprintf(buf,_("Error: the position is already in file %s"),file);
                GTKMessage(_(buf), DT_INFO);
            }
            fclose(fp);
            return FALSE;
        }
    }
    g_message("lineCounter=%d",lineCounter);
    if(lineCounter>MAX_ROWS){
        g_message("writeQuizLineFull full file-> %s",file);
        if(!quiet) {
            char buf[100];
            sprintf(buf,_("Error: file %s has reached the maximum allowed number %d of positions"),
                file,MAX_ROWS);
            GTKMessage(_(buf), DT_INFO);
        }
        fclose(fp);
        return FALSE;
    }    
    writeQuizLine (q, fp);
    g_message("Added a line");
    fclose(fp);
    return TRUE;
}

// extern int AddQuizPosition(quiz qRow, categorytype * pcategory)
// {
//     writeQuizLineFull (qRow, pcategory->path, FALSE);

// 	// FILE* fp = fopen(pcategory->path, "a");

// 	// if (!fp){
//     //     GTKMessage(_("Error: problem saving quiz position, cannot open file"), DT_INFO);
// 	// 	// printf("Can't open file\n");
//     //     return FALSE;
//     // } 
//     // // Saving data in file
//     // // fprintf(fp, "%s, %d, %.5f, %ld\n", qRow.position, qRow.player, qRow.ewmaError, qRow.lastSeen);
//     // writeQuizLine (qRow, fp);
//     // g_message("Added a line");
//     // fclose(fp);
//     // return TRUE;
// }

extern int AutoAddQuizPosition(quiz q, quizdecision qdec) {

    char * file;
    // char * file = g_build_filename(szHomeDirectory, "quiz", "autoadd.csv", NULL);

    // if (!g_file_test(file, G_FILE_TEST_IS_REGULAR)) {
    //     g_message("autoadd file had a problem: %s",file);
    //     return FALSE;
    // }

    if(qdec==QUIZ_MOVE)
        file = g_build_filename(szHomeDirectory, "quiz", "MOVE-autoadded.csv", NULL);
    else if(qdec==QUIZ_DOUBLE || qdec==QUIZ_NODOUBLE)
        file = g_build_filename(szHomeDirectory, "quiz", "DOUBLE-NO_DOUBLE-autoadded.csv", NULL);
    else if(qdec==QUIZ_PASS || qdec==QUIZ_TAKE)
        file = g_build_filename(szHomeDirectory, "quiz", "PASS-TAKE-autoadded.csv", NULL);
    else
        return FALSE;

    return (writeQuizLineFull (q, file, TRUE));//TRUE for quiet

    // if(!g_file_test(file, G_FILE_TEST_EXISTS )){
    //     g_message("autoadd file doesn't exist: %s",file);
    //     	FILE* fp0 = fopen(file, "w");        
    //         if(!fp0){
    //             g_message("autoadd file had a pointer problem:fp0-> %s",file);
    //             return FALSE;
    //         }
    //         writeQuizHeader (fp0);
    //         fclose(fp0);
    // }
    // FILE* fp = fopen(file, "a");
    // if(!fp){
    //     g_message("autoadd file had a pointer problem: %s",file);
    //     return FALSE;
    // }
    // writeQuizLine (q, fp);
    // g_message("Added a line");
    // fclose(fp);
    // return TRUE;
}

// extern int AutoAddQuizPosition(-rSkill,QUIZ_NODOUBLE)(float error, quizdecision qdec) {
//     g_message("Adding position: %s to category %s, error: %f",
//         qNow.position,pcategory->name,qNow.ewmaError);
//     qNow.lastSeen=(long int) (time(NULL));
//     return (AddQuizPosition(qNow,pcategory));
// }


/* Here we save a full file with all positions. We assume that they were already checked to 
be distinct. */
static int SaveFullPositionFile(void)
{
        // szFile = g_build_filename(szHomeDirectory, "gnubgautorc", NULL);

    FILE* fp = fopen(categories[currentCategoryIndex].path, "w");

	if (!fp){
        GTKMessage(_("Error: problem saving quiz position, cannot open file"), DT_INFO);
        return FALSE;
    } 
    /*header*/
    // fprintf(fp, "position, player, ewmaError, lastSeen\n");
    writeQuizHeader (fp);
    for (int i = 0; i < qLength; ++i) {
        // Saving data in file
        // fprintf(fp, "%s, %d, %.5f, %ld\n", q[i].position, q[i].player, q[i].ewmaError, q[i].lastSeen);
        writeQuizLine (q[i], fp);
        // writeQuizLineFull (quiz q, categories[currentCategoryIndex].path, int quiet);
    }
    // g_message("Saved q");
    fclose(fp);
    return TRUE;
}

extern void DeletePosition(void) {
    if (!GTKGetInputYN(_("Are you sure you want to delete this position?")))
        return;
    q[iOpt]=q[qLength-1];
    qLength-=1;
    SaveFullPositionFile();
    GTKMessage(_("Position deleted."), DT_INFO);
    if (qLength==0) {
        StopLoopClicked(NULL,NULL);
        return;
    }
    else {
        return;
    }
}


extern void qUpdate(float error) {
    /* if someone makes a mistake, then plays again the same position after seeing the 
    answer, only the first time should count => we use iOptCounter 
    */

    if(iOptCounter==0) {
        g_message("updating in qUpdate with error=%f",error);
        // g_message("q[iOpt].ewmaError=%f, error=%f => ?",q[iOpt].ewmaError,error);//        2/3*q[iOpt].ewmaError+1/3*error);
        q[iOpt].ewmaError=ERROR_DECAY*(q[iOpt].ewmaError)+(1-ERROR_DECAY)*error;
        // g_message("result: q[iOpt].ewmaError=%f", q[iOpt].ewmaError);  
        q[iOpt].lastSeen=(long int) (time(NULL));
        /*Here we save the whole file again. If it gets slow, alternative=keep line
        number and only update this line.*/
        SaveFullPositionFile();
        latestErrorInQuiz=error;
        iOptCounter=1;
        counterForFile++;
    } else  
        g_message("NOT updating in qUpdate! 2nd update or more...");

}
// #if(0) /***********/
static void DisplayCategories(void)
{
    for(int i=0;i < numCategories; i++) {
        g_message("in DisplayCategories: %d->%s,%d,%s", 
            i,categories[i].name,categories[i].number,categories[i].path);
    }
        g_message("numCategories=%d",numCategories);
}

// int NameIsCategory (const char sz[]) {
//     for(int i=0;i < numCategories; i++) {
//         if (!strcmp(sz, categories[i])) {
//             // g_message("NameIsKey: EXISTS! %s=%s at i=%d", sz,categories[i],i);
//             return 1;
//         }
//     }
//     return 0;
// }

static void InitCategoryArray(void) {
    const categorytype emptyCategory={"\0",0,"\0"};
    for(int i=0;i < MAX_POS_CATEGORIES; i++) {
        categories[i]=emptyCategory;
        // categories[i][0]='\0';
        // numCategories=0;
    }
    numCategories=0;
}


// /* from ad absurdum, stackoverflow*/
// void strip_ext(char *fname)
// {
//     char *end = fname + strlen(fname);

//     while (end > fname && *end != '.' && *end != '\\' && *end != '/') {
//         --end;
//     }
//     if ((end > fname && *end == '.') &&
//         (*(end - 1) != '\\' && *(end - 1) != '/')) {
//         *end = '\0';
//     }  
// }


static int CountPositions(categorytype category) {

    //char * fullPath = g_strconcat(positionsFolder, G_DIR_SEPARATOR_S, category, ".csv", NULL);
    // g_message("fullPath=%s",fullPath);


	FILE* fp = fopen(category.path, "r");
    if (fp == NULL) {
        g_message("error: opening file: %s ", category.path);
        return 0; //category.number=0;
    }

    int c;    // this must be an int
    int count = -1; //don't count the title

    for (c = getc(fp); c != EOF; c = getc(fp)) {
        if (c == '\n') // Increment count if this character is newline 
            count = count + 1;
    }

    fclose(fp);
    // g_message("count=%d",count);
    return count; //category.number=count;
}

// /*case-insensitive string comparison, inspired by minilibc code from JL2210 */

// int strcasecmp(const char *restrict str1, const char *restrict str2)
// {
//     const unsigned char *s1 = (const unsigned char *)str1,
//                         *s2 = (const unsigned char *)str2;

//     if(str1 == str2)
//         return 0;

//     while( *s1 && toupper(*s1) == toupper(*s2) )
//     {
//         s1++;
//         s2++;
//     }

//     return *s1 - *s2;
// }

/*case-insensitive string comparison*/
// int string_cmp (const categorytype * p1, const categorytype * p2) {
int string_cmp (const void * p1, const void * p2) {
    // const char * pa = (const char *) a;
    // const char * pb = (const char *) b;
    // return strcmp(pa,pb);
    // return strcmp(((const categorytype *)p1)->name, ((const categorytype *)p2)->name);
    const unsigned char *s1 = (const unsigned char *)((const categorytype *)p1)->name,
                        *s2 = (const unsigned char *)((const categorytype *)p2)->name;
    if(s1 == s2)
        return 0;
    while( *s1 && toupper(*s1) == toupper(*s2) )
    {
        s1++;
        s2++;
    }
    // g_message("string comp: %s vs %s, %c vs %c=>%d|%d",s1,s2,(toupper(*s1)),(toupper(*s2)),*s1 - *s2,toupper(*s1) - toupper(*s2));
    return toupper(*s1) - toupper(*s2);
}

static void populateCategory(const int index, const char * name, const int updateNumber) {
    /* add name*/
    strcpy(categories[index].name, name);
    /* add file path */
    // strcpy(categories[index].path,g_strconcat(positionsFolder, 
    //     G_DIR_SEPARATOR_S, categories[index].name, ".csv", NULL));
    gchar *file = g_strdup_printf("%s.csv", name);
    gchar *path = g_build_filename(szHomeDirectory, "quiz", file, NULL);
    strcpy(categories[index].path,path);

    /*compute and add number of positions*/
    if (updateNumber)
        categories[index].number=CountPositions(categories[index]);
    // g_message("at index %d with name=%s, number=%d",index,name,categories[index].number);
}

extern void GetPositionCategories(void) {
    //     char buffer[MAX_LEN];
    // FilePreviewData *fdp;
    struct dirent* entry;
    // time_t recenttime = 0;
    // struct stat statbuf;
    InitCategoryArray();

    DIR* dir  = opendir(g_build_filename(szHomeDirectory, "quiz", NULL));
    while (NULL != (entry = readdir(dir))) {
        //g_message("entry->d_name=%s",entry->d_name);
        const char *dot = strrchr(entry->d_name, '.');
        if(dot && dot != entry->d_name && (strcmp(dot+1,"csv") == 0)) {
            int offset = dot - entry->d_name;
            entry->d_name[offset] = '\0';
            populateCategory(numCategories, entry->d_name,TRUE);
            // strcpy(categories[numCategories].name, entry->d_name);
            // // g_message("stripped entry->d_name=%s=%s",entry->d_name,
            //     // categories[numCategories]);
            // /*computes number of positions*/
            // CountPositions(&(categories[numCategories]));
            // // categories[numCategories].number=CountPositions(categories[numCategories]);
            // categories[numCategories].path=g_strconcat(positionsFolder, 
            //     G_DIR_SEPARATOR_S, categories[numCategories].name, ".csv", NULL);
            
            numCategories++;
        }
    }
    /* sorting alphabetically */
    qsort(categories, numCategories, sizeof(categorytype), string_cmp );

    closedir(dir);
    if(0)
        DisplayCategories();
}

/* for reference: code for deleting several rows, if needed once (Fedor, stackoverflow):
static void
delete_selected_rows (GtkButton * activated, GtkTreeView * tree_view)
{
  GtkTreeSelection * selection = gtk_tree_view_get_selection (tree_view);
  GtkTreeModel *model;
  GList * selected_list = gtk_tree_selection_get_selected_rows (selection, &model);
  GList * cursor = g_list_last (selected_list);
  while (cursor != NULL) {
    GtkTreeIter iter;
    GtkTreePath * path = cursor->data;
    gtk_tree_model_get_iter (model, &iter, path);
    gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
    cursor = cursor->prev;
  }
  g_list_free_full (selected_list, (GDestroyNotify) gtk_tree_path_free);
}
*/

/* another reference: the code below could be shorter with:
  if (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(store), &iter, NULL, n))
    {
      gtk_list_store_remove(store, &iter);
      return TRUE;
    }
*/

/*  delete a position category from the categories array */
static int DeleteCategory(const int categoryIndex) {
    // g_message("in DeleteCategory: %s, length=%zu", sz, strlen(sz));
    // g_message("before loop in DeleteCategory:numCategories=%d, look for %s ",numCategories, sz);
    if(categoryIndex<0 || categoryIndex>=numCategories) {
        GTKMessage(_("Error: you forgot to select a position category"), DT_INFO);
        return 0;
    }
    // for(int i=0;i < numCategories; i++) {
    //     g_message("loop in DeleteCategory: i=%d, numCategories=%d, %s =?= %s ",i,numCategories, sz,categories[i]);
    //     if (!strcmp(sz, categories[i].name)) {
    //         g_message("EXISTS! %s=%s, i=%d, numCategories=%d", sz,categories[i],i,numCategories);
    //         // char *fullPath = g_strconcat(positionsFolder, G_DIR_SEPARATOR_S, sz, ".csv", NULL);
    if (remove(categories[categoryIndex].path)!=0){
        GTKMessage(_("Error: File could not be removed"), DT_INFO);
        return 0;
    }
    // if (numCategories==(categoryIndex+1)) {
    //         g_message("DeleteCategory: last category");
    //         numCategories--;
    //         // UserCommand2("save settings");
    //         // return 1;
    // } else {

    for(int j=categoryIndex+1;j < numCategories; j++) {
        g_message("loop2 in DeleteCategory: j=%d",j);
        categories[j-1]=categories[j]; 
    }
    numCategories--;
        // DisplayCategories();
        // UserCommand2("save settings");
        // return 1;
    g_message("end of deletecategory");
    return 1;
}

static int CheckBadCharacters(const char * name) {
    g_message("checking");
    char bad_chars[] = "!@%^*~|<>:$/?\\\"";
    int invalid_found = FALSE;
    int i;
    for (i = 0; i < (int) strlen(bad_chars); ++i) {
        if (strchr(name, bad_chars[i]) != NULL) {
            invalid_found = TRUE;
            g_message("invalid:%c",bad_chars[i]);
            break;
        }
    }
    return invalid_found;
}

/*  add a new position category to the categories array ... 
and make a corresponding file.
Based on AddkeyName() function (and same for the similar functions above)
Return 1 if success, 0 if problem */
static int
AddCategory(const char *sz)
{
    // g_message("in AddCategory: %s, length=%zu", sz, strlen(sz));
    /* check that the name doesn't contain "\t", "\n" */
    if (strstr(sz, "\t") != NULL || strstr(sz, "\n") != NULL || CheckBadCharacters(sz)) {
        // for(unsigned int j=0;j < strlen(sz); j++) {
        //     g_message("%c", sz[j]);
        // }
            GTKMessage(_("Error: Position category name contains unallowed character"), DT_INFO);
        return 0;
    }

    /* check that the category name is not too long*/
    if(strlen(sz) > MAX_CATEGORY_NAME_LENGTH) {
            GTKMessage(_("Error: Position category name is too long"), DT_INFO);
        return 0;
    }

    /* check that the categories array is not full */
    if(numCategories >= MAX_POS_CATEGORIES) {
            GTKMessage(_("Error: We have reached the maximum allowed number of position categories"), DT_INFO);
        return 0;
    }

    /* check that the position category doesn't already exist */
    for(int i=0;i < numCategories; i++) {
        if (!strcmp(sz, categories[i].name)) {
            GTKMessage(_("Error: This category name already exists"), DT_INFO);
            return 0;
        }
    }

    /* check that file can be added*/
    gchar *file = g_strdup_printf("%s.csv", sz);
    gchar *fullPath = g_build_filename(szHomeDirectory, "quiz", file, NULL);
    // char *fullPath = g_strconcat(positionsFolder, G_DIR_SEPARATOR_S, sz, ".csv", NULL);
    g_message("fullPath=%s",fullPath);

	FILE* fp = fopen(fullPath, "w");

    if(fp==NULL) {
        char buf[100];
        sprintf(buf,_("Error: Problem writing into file %s"),fullPath);
        GTKMessage(_(buf), DT_INFO);
        return 0;
    }
    //  perror("fopen");
    /*header*/
    writeQuizHeader (fp);
    // fprintf(fp, "position, cubedecision, ewmaError, lastSeen\n");
    fclose(fp);

	// FILE* fp2 = fopen(sz, "r");

    // if(fp2==NULL) {
    //     char buf[100];
    //     sprintf(buf,_("Error: Could not create file %s, maybe we do not have writing right?"),
    //         fullPath);
    //     GTKMessage(_(buf), DT_INFO);
    //     return 0;
    // }
    // fclose(fp2);

    populateCategory(numCategories,sz,FALSE);

    // strcpy(categories[numCategories].name,sz); 
    numCategories++;

        /* sorting alphabetically */
    qsort(categories, numCategories, sizeof(categorytype), string_cmp );
    BuildQuizMenu(NULL);

    // DisplayCategories();
    // UserCommand2("save settings");
    return 1;
}

/*  rename a position category in the array, and its corresponding file.
Return the position of the category, -1 if problem */
static int
RenameCategory(const char * szOld, const char * szNew)
{
    if (!strcmp(szOld, szNew)) {
        GTKMessage(_("Error: The old and new category names are the same"), DT_INFO);
        return -1;
    }

    /* check that the new name doesn't contain "\t", "\n" */
    if (strstr(szNew, "\t") != NULL || strstr(szNew, "\n") != NULL
         || CheckBadCharacters(szNew)) {
        GTKMessage(_("Error: Position category name contains unallowed character"), DT_INFO);
        return -1;
    }

    /* check that the new category name is not too long*/
    if(strlen(szNew) > MAX_CATEGORY_NAME_LENGTH) {
        GTKMessage(_("Error: Position category name is too long"), DT_INFO);
        return -1;
    }

    /* check that the old position category exists, and the new one does not! */
    int positionIndex=-1;
    for(int i=0;i < numCategories; i++) {
        if (!strcmp(szNew, categories[i].name)) {
            GTKMessage(_("Error: This category name already exists"), DT_INFO);
            return -1;
        }
        if (!strcmp(szOld, categories[i].name)) {
            positionIndex=i;
        }
    }
    if(positionIndex==-1) {
        GTKMessage(_("Error: Could not find the old position name"), DT_INFO);
        return -1;
    }

    /* replace file*/
    // char *fullOldPath = g_strconcat(positionsFolder, G_DIR_SEPARATOR_S, szOld, ".csv", NULL);
    // char fullOldPath [MAX_CATEGORY_PATH_LENGTH];
    // strcpy(fullOldPath, categories[i].path);
    // char *fullNewPath = g_strconcat(positionsFolder, G_DIR_SEPARATOR_S, szNew, ".csv", NULL);
    gchar *file = g_strdup_printf("%s.csv", szNew);
    gchar *fullNewPath = g_build_filename(szHomeDirectory, "quiz", file, NULL);

    int result = rename(categories[positionIndex].path, fullNewPath);
	
    if(result!=0) {
        char buf[200];
        sprintf(buf,_("Error: Problem renaming file %s as %s"),categories[positionIndex].path,fullNewPath);
        GTKMessage(_(buf), DT_INFO);
        return -1;
    }

    // strcpy(categories[positionIndex].name,szNew); 
    populateCategory(positionIndex,szNew,TRUE);

    // /* sorting alphabetically */
    // qsort(categories, numCategories, sizeof(categorytype), string_cmp );

    DisplayCategories();
    // UserCommand2("save settings");
    return positionIndex;
}

// #endif /***********/


static GtkTreeIter selected_iter;
static GtkListStore *nameStore;

static int
GetSelectedCategoryIndex(GtkTreeView * treeview)
{
    GtkTreeModel *model;
    // char *category = NULL;
    GtkTreeSelection *sel = gtk_tree_view_get_selection(treeview);
    if (gtk_tree_selection_count_selected_rows(sel) != 1) {
        g_message("no index!");
        return (-1);
    }
    GList * selected_list = gtk_tree_selection_get_selected_rows (sel, &model);
    GList * cursor = g_list_first (selected_list);
    GtkTreePath * path = cursor->data;
    gint * a = gtk_tree_path_get_indices (path);
    g_message("index=%d",a[0]);
    return a[0];
}

static char *
GetSelectedCategory(GtkTreeView * treeview)
{
    GtkTreeModel *model;
    char *categoryName = NULL;
    GtkTreeSelection *sel = gtk_tree_view_get_selection(treeview);
    if (gtk_tree_selection_count_selected_rows(sel) != 1)
        return NULL;
    
    /* Sets selected_iter to the currently selected node: */
    gtk_tree_selection_get_selected(sel, &model, &selected_iter);
    
    /* Gets the value of the char* cell (in column COLUMN_STRING) in the row 
        referenced by selected_iter */
    gtk_tree_model_get(model, &selected_iter, COLUMN_STRING, &categoryName, -1);
    // g_message("GetSelectedCategory gives categoryName=%s",categoryName);
    return categoryName;
}

static void
DestroyQuizDialog(gpointer UNUSED(p), GObject * UNUSED(obj))
/* Called by gtk when the window is closed.
Allows garbage collection.
*/
{
    g_message("in destroy");
    // sprintf(name0BeforeQuiz, "%s",ap[0].szName);
    // sprintf(name1BeforeQuiz, "%s",ap[0].szName);
    

    if (pwQuiz) { //i.e. we didn't close it using DestroyQuizDialog()
        gtk_widget_destroy(gtk_widget_get_toplevel(pwQuiz));
        g_message("in destroy loop");
        pwQuiz = NULL;
    }
}

extern void ReloadQuizConsole(void) {
    // GetPositionCategories();
    
    // UpdateGame(FALSE);
    // GTKClearMoveRecord();
            // CreateGameWindow();
            //CreatePanels();
    // UserCommand2("set dockpanels off"); //       DockPanels();


    /*this was needed to apply the change in the gamelist menu, but is not anymore*/
    //UserCommand2("set dockpanels on"); //       DockPanels();

    DestroyQuizDialog(NULL,NULL);
    QuizConsole();

}
// extern void
// CommandAddQuizPosition(char *sz)
// {

//     char *pch = NextToken(&sz);
//     int i;

//     if (!pch) {
//         outputl(_("To which quiz category do you want to add this position?"));
//         return;
//     }

//     GetPositionCategories();
//     for(int i=0;i < numCategories; i++) {
//         if (!strcmp(sz, categories[i].name)) {
//             AddPositionToFile(&(categories[i]),NULL);
//             return;
//         }
//     }
//     outputl(_("Error: This category does not exist; please create it first"), DT_INFO);
//     // GTKMessage(_("Error: This category name already exists"), DT_INFO);
// }

static void
AddPositionClicked(GtkButton * UNUSED(button), gpointer treeview)
{
    // GtkTreeIter iter;
    // GetSelectedCategoryIndex(GTK_TREE_VIEW(treeview));
    int categoryIndex = GetSelectedCategoryIndex(GTK_TREE_VIEW(treeview));
    if(categoryIndex<0 || categoryIndex>=numCategories) {
        GTKMessage(_("Error: you forgot to select a position category"), DT_INFO);
        return;
    }
    // g_position("got back with int=%d",categoryIndex);
    if(AddPositionToFile(&(categories[categoryIndex]),NULL)){
        GTKMessage(_("The position was added successfully."), DT_INFO);
        ReloadQuizConsole();
    }
    return;    
}

static void
AddNDPositionClicked(GtkButton * UNUSED(button), gpointer treeview)
{
    if(qNow_NDBeforeMoveError <-0.001) {
        /*should not happen, there is no ND position; maybe user clicked outside 
        the manage window*/
        GTKMessage(_("Error: the selected move seems to have changed, reloading the quiz window"), DT_INFO);
        ReloadQuizConsole();
        return;        
    }
    int categoryIndex = GetSelectedCategoryIndex(GTK_TREE_VIEW(treeview));
    if(categoryIndex<0 || categoryIndex>=numCategories) {
        GTKMessage(_("Error: you forgot to select a position category"), DT_INFO);
        return;
    }
    if(AddNDPositionToFile(&(categories[categoryIndex]),NULL)){
        GTKMessage(_("The double/no-double decision was added successfully."), DT_INFO);
        ReloadQuizConsole();
    }
    return; 
}

static void AddCategoryClicked(GtkButton * UNUSED(button), gpointer UNUSED(treeview))
{
    // GtkTreeIter iter;
    char *categoryName = GTKGetInput(_("Add position category"), 
        _("Position category:"), NULL);
    if(!categoryName) 
        return;
    g_message("add=%s",categoryName);
    if (AddCategory(categoryName)) {
    // AddCategory(category);
        // gtk_list_store_append(GTK_LIST_STORE(nameStore), &iter);
        // gtk_list_store_set(GTK_LIST_STORE(nameStore), &iter, 0, categoryName, -1);
        // gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), &iter);
        // selected_iter=iter;
        ReloadQuizConsole();
    }  
    /* assuming AddCategory already gave an error message*/
    // else {
    //     GTKMessage(_("Error: problem adding this position category"), DT_INFO);
    // }

    /*if we don't refresh the game window, the right-click won't show the newly added
    category*/
    
    // ShowHidePanel(WINDOW_GAME);
    // ShowHidePanel(WINDOW_GAME);
    // GL_Create();

    // GTKRegenerateGames();
    // gtk_list_store_clear(plsGameList);

    // 
    g_free(categoryName);
    return;    
}

static void RenameCategoryClicked(GtkButton * UNUSED(button), gpointer treeview)
{
    GtkTreeIter iter;
    // GetSelectedCategoryIndex(GTK_TREE_VIEW(treeview));
    char * oldCategory = GetSelectedCategory(GTK_TREE_VIEW(treeview));
    if(!oldCategory){
        GTKMessage(_("Error: please select a position category to rename"), DT_INFO);
    }
    char *newCategory = GTKGetInput(_("Add position category"), 
        _("Position category:"), NULL);
    if(!newCategory) {
        g_free(oldCategory);
        return;
    }
    
    int positionIndex = RenameCategory(oldCategory, newCategory);

    gtk_list_store_remove(GTK_LIST_STORE(nameStore), &selected_iter);
    gtk_list_store_insert(GTK_LIST_STORE(nameStore), &iter, positionIndex);
    gtk_list_store_set(GTK_LIST_STORE(nameStore), &iter, 0, newCategory, -1);
    selected_iter=iter;

    ReloadQuizConsole();

    g_free(newCategory);
    g_free(oldCategory);
}

static void
DeleteCategoryClicked(GtkButton * UNUSED(button), gpointer treeview)
{

    int categoryIndex = GetSelectedCategoryIndex(GTK_TREE_VIEW(treeview));
    if(categoryIndex<0 || categoryIndex>=numCategories) {
        GTKMessage(_("Error: you forgot to select a position category"), DT_INFO);
        return;
    }
    if (!GTKGetInputYN(_("Are you sure?"))) { 
        GTKMessage(_("Error: problem deleting this position category"), DT_INFO);
        return;
    }
    /*just to update the selected_iter...*/
    GetSelectedCategory(GTK_TREE_VIEW(treeview));
    // char *categoryName =g_strdup(GetSelectedCategory(GTK_TREE_VIEW(treeview)));
    // g_message("categoryName=%s",categoryName);
    gtk_list_store_remove(GTK_LIST_STORE(nameStore), &selected_iter);

    if (!DeleteCategory(categoryIndex)) {
        return;
    }


    ReloadQuizConsole();
    // DisplayCategories();
    
        return;
}

static void updatePriority(quiz * pq, long int seconds) {
    pq->priority = (pq->ewmaError + SMALL_ERROR) * (float) (seconds-pq->lastSeen);
    // pq->priority = (pq->ewmaError + SMALL_ERROR) * (pq->ewmaError + SMALL_ERROR) * (float) (seconds-pq->lastSeen);
    // return ( (q.ewmaError + SMALL_ERROR) * (q.ewmaError + SMALL_ERROR) * (float) (seconds-q.lastSeen));
}

extern void LoadPositionAndStart (void) {
    // fInQuizMode=TRUE;
    qDecision=QUIZ_UNKNOWN;
    // sprintf(name0BeforeQuiz, "%s", ap[0].szName);
    // sprintf(name1BeforeQuiz, "%s", ap[1].szName);

    /*this should not happen as it was previously checked before calling the function:*/
    if(qLength<0) {
        GTKMessage(_("Error: no positions in this category"), DT_INFO);
        QuizConsole();
        return;
    }

    long int seconds=(long int) (time(NULL));
    // float maxPriority=0;
    iOpt=0;
    updatePriority(&(q[iOpt]),seconds);
    for (int i = 1; i < qLength; ++i) {
        /* heuristic formula, the "secret sauce"...
        1) Very roughly: a 2x bigger typical error should be seen 2x more often -
        and then the error hopefully goes down.
        2) If we add a non-analyzed position, or a position with 0 error, then it will never appear in the loop 
        unless we add something to get a non-null priority 
        */
        updatePriority(&(q[i]),seconds);
        // q[i].priority=(q[i].ewmaError + SMALL_ERROR) * (q[i].ewmaError + SMALL_ERROR) * (float) (seconds-q[i].lastSeen); // /1000.0;
        g_message("priority %.3f <-- %.3f, %.3f, %ld\n", q[i].priority, q[i].ewmaError, (float) (seconds-q[i].lastSeen)/1000.0,q[i].lastSeen);
        if(q[i].priority>q[iOpt].priority) {
            // maxPriority=q[i].priority;
            iOpt=i;
        }
    }
    iOptCounter=0;
    g_message("iOpt=%d: priority %.3f <-- error %.3f, delta t %.3f, %ld\n", 
        iOpt,
        q[iOpt].priority, q[iOpt].ewmaError, (float) (seconds-q[iOpt].lastSeen),
        q[iOpt].lastSeen);

    // qNow=q[iOpt];

    if(!q[iOpt].position){
        char buf[100];
        sprintf(buf, _("Error: wrong position in line %d of file?"), iOpt+1);
        GTKMessage(_(buf), DT_INFO);
        return;
    }

    /* start quiz-mode play! */
    // fInQuizMode=TRUE;
    // g_message("names: %s %s",ap[0].szName,ap[1].szName);
  
    CommandSetGNUBgID(q[iOpt].position); 

    g_message("fDoubled=%d, fMove=%d, fTurn=%d, recorderdplayer=%d",
            ms.fDoubled, ms.fMove, ms.fTurn, q[iOpt].player);
    if(q[iOpt].player==0) {
    // if(ms.fTurn == 0) {
        g_message("swap!");
        // SwapSides(ms.anBoard);
        CommandSwapPlayers(NULL);   
        // sprintf(ap[1].szName, "%s", name1BeforeQuiz);
        // g_message("names: %s %s",ap[0].szName,ap[1].szName);
    }
    // UserCommand2("set player 0 human");
    // g_message("human");
    // UserCommand2("set player 0 name QuizOpponent");
    // g_message("name");
    //     g_message("names: %s %s",ap[0].szName,ap[1].szName);

    char name0InQuiz[MAX_NAME_LEN]="Quiz Opponent";
    // char name1InQuiz[MAX_NAME_LEN];
    if(strcmp(ap[0].szName,name0InQuiz))
        strncpy(ap[0].szName, name0InQuiz, MAX_NAME_LEN);
    if(strcmp(ap[1].szName,name1BeforeQuiz))
        strncpy(ap[1].szName, name1BeforeQuiz, MAX_NAME_LEN);
    g_message("Starting quiz mode: ap names: (%s,%s) vs: (%s,%s)",
        ap[0].szName,ap[1].szName,name0BeforeQuiz,name1BeforeQuiz);        

    // if (ap[ms.fTurn].pt != PLAYER_HUMAN) {
        // g_message("player=%d",q[iOpt].player);

    // g_message("Pre-double: fDoubled=%d, fMove=%d, fTurn=%d, recorderdplayer=%d",
    //     ms.fDoubled, ms.fMove, ms.fTurn, q[iOpt].player);
    // CommandPlay(""); //<-if opponent is computer; but what if its right 
                        //decision is not to double? We need a double
    /* User is player 1.
    When player 1 has a take, we need player 0 to double, else 1 cannot play. 
    In such a case:
    Before doubling: fMove=0,fTurn=0 (and fDoubled=0)
    After doubling: fMove=0,fTurn=1 (and fDoubled=0)
    */
   if(ms.fTurn ==0) { /*T/P*/
        CommandDouble("");
        StatusBarMessage(_("Your turn to play this quiz position: take or pass?"));
   } else if(ms.anDice[0]>0) /*move*/
        StatusBarMessage(_("Your turn to play this quiz position: best move?"));
    else /*T/K*/
        StatusBarMessage(_("Your turn to play this quiz position: double or no-double?"));
    // g_message("double");
     g_message("Post: fDoubled=%d, fMove=%d, fTurn=%d, recorderdplayer=%d",
        ms.fDoubled, ms.fMove, ms.fTurn, q[iOpt].player);
    // gtk_statusbar_push(GTK_STATUSBAR(pwStatus), idOutput, _("Your turn to play this quiz position!"));


}


static GtkWidget * BuildCategoryList(void) {
   // GtkWidget *pwQuiz;
    GtkWidget *treeview;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    // GtkListStore *store;
    GtkTreeIter iter;
    // GtkTreeViewColumn   *col;
 
    // DisplayCategories();

    /* We always check the positions again because the user may have added positions since the
    gnubg start-up */
    // if(!categories[0].name) {
    //     g_message("The categories array was expected to be initialized at start-up...");
    GetPositionCategories();
    // }

    /* create list store */

    nameStore = gtk_list_store_new(N_COLUMNS, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT);

    for(int i=0;i < numCategories; i++) {
        gtk_list_store_append(nameStore, &iter);
        // int numberPositions=CountPositions(categories[i]);
        gtk_list_store_set(nameStore, &iter, 
                            COLUMN_INDEX,i,
                            COLUMN_STRING, categories[i].name,
                            COLUMN_INT, categories[i].number,
                            -1);
        // g_message("in DisplayCategories: %d->%s", i,categories[i]);
    }

    /* create tree view */
    treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(nameStore));

    g_object_unref(nameStore);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_(""), 
        renderer, "text", COLUMN_INDEX, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("Category"), 
        renderer, "text", COLUMN_STRING, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("Num. positions"), 
        renderer, "text", COLUMN_INT, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    return treeview;
}
// static int eventCounter=0;

// static gboolean QuizManageReleased(GtkWidget * UNUSED(widget), GdkEventButton  * event, GtkWidget * myTreeView)
// {
//     switch (event->type) {
//     // case GDK_BUTTON_PRESS:
//     //     fScrollComplete = FALSE;
//     //     break;
//     case GDK_BUTTON_RELEASE:
//         // fScrollComplete = TRUE;
//         // g_signal_emit_by_name(G_OBJECT(prw->pScale), "value-changed", prw);
//         // g_message("released");
//         g_message("released time=%d",event->time);

        
//         currentCategoryIndex = GetSelectedCategoryIndex(GTK_TREE_VIEW(myTreeView));
//         g_message("currentCategoryIndex=%d",currentCategoryIndex);
//         // gtk_widget_destroy(gtk_widget_get_toplevel(pw));
//         if(currentCategoryIndex<0 || currentCategoryIndex>=numCategories) {
//             // GTKMessage(_("Error: you forgot to select a position category"), DT_INFO);
//             // QuizConsole();
//             return FALSE;
//         }
//         break;
//     default:
//         ;
//     }

//     return FALSE;
// }
static void QuizManageClicked(GtkWidget * UNUSED(widget), GdkEventButton  * event, GtkTreeView * treeview) {
    
    g_message("before: currentCategoryIndex=%d",currentCategoryIndex);
    currentCategoryIndex = GetSelectedCategoryIndex(GTK_TREE_VIEW(treeview));
    g_message("after: currentCategoryIndex=%d",currentCategoryIndex);

    /*double-click, e.g. w/ left or middle click, not right*/
    if (event->type == GDK_2BUTTON_PRESS  &&  event->button != 3){
        // g_message("double-click time=%d",event->time);
        // gtk_widget_destroy(gtk_widget_get_toplevel(pw));
        // if(currentCategoryIndex<0 || currentCategoryIndex>=numCategories) {
        //     // GTKMessage(_("Error: you forgot to select a position category"), DT_INFO);
        //     // QuizConsole();
        // }
        if(currentCategoryIndex>=0 && currentCategoryIndex<numCategories) {
            StartQuiz(NULL, treeview);
        }
    }
}
/* We disable / enable buttons depending on whether a category is 
selected. To do so, (1) here, we set these variables static.
Potential alternatives: (2) Refresh the quiz manage window when
a category is selected to enable the buttons, or 
(3) not disable the buttons at all. */
static GtkWidget *delButton;
static GtkWidget *renameButton;
static GtkWidget *startButton;

void on_changed(GtkTreeSelection *selection, gpointer UNUSED(p))
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    // int value;

    if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection), &model, &iter))
    {
        gtk_tree_model_get(model, &iter, COLUMN_INDEX, &currentCategoryIndex,  -1);
        g_print("%d is selected\n", currentCategoryIndex);
        // g_free(value);
        gtk_widget_set_sensitive(startButton, (currentCategoryIndex>=0 && currentCategoryIndex<numCategories));
        gtk_widget_set_sensitive(renameButton, (currentCategoryIndex>=0 && currentCategoryIndex<numCategories));
        gtk_widget_set_sensitive(delButton, (currentCategoryIndex>=0 && currentCategoryIndex<numCategories));

    }
}

// display info on quiz feature
static void ExplanationsClicked(GtkWidget * UNUSED(widget), GtkWidget* pwParent) {
    
    GtkWidget* pwInfoDialog, * pwBox;
    // const char* pch;

    pwInfoDialog = GTKCreateDialog(_("Quiz Info"), DT_INFO, pwParent, DIALOG_FLAG_MODAL, NULL, NULL);
#if GTK_CHECK_VERSION(3,0,0)
    pwBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
    pwBox = gtk_vbox_new(FALSE, 0);
#endif
    gtk_container_set_border_width(GTK_CONTAINER(pwBox), 8);

    gtk_container_add(GTK_CONTAINER(DialogArea(pwInfoDialog, DA_MAIN)), pwBox);

    AddText(pwBox, _("The quiz feature enables you to collect and try again positions where you blundered. \
        \n\n- It is managed through the quiz console. To launch the quiz console, either: \
        \n\t (1) right-click and select it, or \
        \n\t (2) use the menu: \"Game > Quiz \", \
        \n\n- There are two ways to collect positions: \
        \n\t * AUTOMATIC: GNUBG can automatically add blundered positions to three pre-defined files, \
        \n\t\t depending on whether they correspond to (1) move, (2) double/no-double or \
        \n\t\t (3) pass/take decisions. \
        \n\t * MANUAL: You can create a category in the quiz console, and manually add a position \
        \n\t\t to this category: \
        \n\t\t (1) Either by right-clicking on the position in the (top-right) move-list window \
        \n\t\t (2) Or by launching the quiz console, then selecting the category, then clicking the \
        \n\t\t\t add-position button. \
        \n\t\t Note that sometimes, a position in the move-list window corresponds to two consecutive \
        \n\t\t decisions: roll the dice (ND=no-double) instead of doubling, then move. GNUBG enables you \
        \n\t\t to add both decisions to the quiz positions. \
        \n\n- To start playing the quiz with the positions of a given category, open the quiz console, \
        \n\t then either double-click on the category, or click once and select the play-quiz arrow. \
        \n\t GNUBG manages the behind-the-scenes algorithms that present you first the positions \
        \n\t that have bigger blunders and/or that are harder to you and/or that you haven't seen \
        \n\t for a while. \
        \n\n - You can change the quiz options in \"Settings > Options > Quiz\", e.g., to disable the \
        \n\t automatic collection of blundered positions, or to disable the quiz feature completely."));

    GTKRunDialog(pwInfoDialog);
}

extern void TurnOnQuizMode(void){
    fInQuizMode=TRUE;
    if(strcmp(ap[0].szName,name0BeforeQuiz))
        // name0BeforeQuiz=g_strdup(ap[0].szName);
        strncpy(name0BeforeQuiz, ap[0].szName, MAX_NAME_LEN);
    if(strcmp(ap[1].szName,name1BeforeQuiz))
        strncpy(name1BeforeQuiz, ap[1].szName, MAX_NAME_LEN);
    // g_message("After TurnOn: ap names: (%s,%s) vs: (%s,%s)",
    //     ap[0].szName,ap[1].szName,name0BeforeQuiz,name1BeforeQuiz);
    type0BeforeQuiz=ap[0].pt;
    ap[0].pt = PLAYER_HUMAN;
}
extern void TurnOffQuizMode(void){
    fInQuizMode=FALSE;
    g_message("before TurnOff: ap names: (%s,%s) vs: (%s,%s)",
        ap[0].szName,ap[1].szName,name0BeforeQuiz,name1BeforeQuiz);
    if(strcmp(ap[0].szName,name0BeforeQuiz))
        // sprintf(ap[0].szName, "%s",name0BeforeQuiz);
        strncpy(ap[0].szName, name0BeforeQuiz, MAX_NAME_LEN);
    if(strcmp(ap[1].szName,name1BeforeQuiz))
        strncpy(ap[1].szName, name1BeforeQuiz, MAX_NAME_LEN);
    // g_message("TurnOff: ap names: (%s,%s) vs: (%s,%s)",
    //     ap[0].szName,ap[1].szName,name0BeforeQuiz,name1BeforeQuiz);
    // char buf[100];
    // sprintf(buf,"set player 0 name %s",ap[0].szName);
    // UserCommand2(buf);
    // sprintf(buf,"set player 1 name %s",ap[1].szName);
    // UserCommand2(buf);
    // g_message("after TurnOff: ap names: (%s,%s) vs: (%s,%s)",
    // ap[0].szName,ap[1].szName,name0BeforeQuiz,name1BeforeQuiz);
    ap[0].pt = type0BeforeQuiz;
}

/*"Quiz console" = central management window for the quiz feature */

extern void QuizConsole(void) {

    GtkWidget *pwScrolled;
    GtkWidget *pwMainHBox;
    GtkWidget *pwVBox;
    GtkWidget *addButton; /*and more buttons are in static outside the function*/
    GtkWidget *helpButton;
    GtkWidget *pwMainVBox;
    GtkWidget *addPos1Button;
    GtkWidget *addPos2Button;
    
    currentCategoryIndex=-1;
    /* putting true means that we need to end it when we leave by using the close button and
    only for this screen; so we put false by default for now */
    if(fInQuizMode)
        TurnOffQuizMode();

    GtkWidget *treeview = BuildCategoryList();

    pwScrolled = gtk_scrolled_window_new(NULL, NULL);

    if (pwQuiz) { //i.e. we didn't close it using DestroyQuizDialog()
        gtk_widget_destroy(gtk_widget_get_toplevel(pwQuiz));
        pwQuiz = NULL;
    }

#if GTK_CHECK_VERSION(3,0,0)
    pwQuiz = (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size (pwQuiz, WIDTH, HEIGHT);
    gtk_window_set_position     (pwQuiz, GTK_WIN_POS_CENTER);
    gtk_window_set_title        (pwQuiz, "MWC plot");
    g_signal_connect(pwQuiz, "destroy", G_CALLBACK(gtk_widget_destroy), NULL);
    gtk_widget_show_all ((GtkWidget*)pwQuiz);
#else
    pwQuiz = GTKCreateDialog(_("Quiz console"), DT_INFO, NULL, 
        DIALOG_FLAG_NOOK, NULL, NULL);
        // DIALOG_FLAG_NONE, NULL, NULL);
        // DIALOG_FLAG_NONE, (GCallback) StartQuiz, treeview);
        // DIALOG_FLAG_MODAL | DIALOG_FLAG_CLOSEBUTTON,(GCallback) StartQuiz, 
        //     GTK_TREE_VIEW(treeview));
        // DIALOG_FLAG_MODAL | DIALOG_FLAG_CLOSEBUTTON, NULL, NULL);
        // DIALOG_FLAG_MODAL | DIALOG_FLAG_CLOSEBUTTON, NULL, treeview);
    //    DIALOG_FLAG_NONE
    gtk_window_set_default_size (GTK_WINDOW (pwQuiz), WIDTH, 350);
    GdkColor color;
    gdk_color_parse ("#EDF5FF", &color);
    gtk_widget_modify_bg(pwQuiz, GTK_STATE_NORMAL, &color);

    // color.red = 0xcfff;
    // color.green = 0xdfff;
    // color.blue = 0xefff;
    // gdk_color_parse ("red", &color);
    // gdk_color_parse ("black", &color);
    //  gdk_color_parse (BGCOLOR, &color);    
    // gtk_window_set_title(GTK_WINDOW(pwQuiz), "hello");
    // if (gdk_color_parse("#c0deed", &color)) {
    // } else {
    //     gtk_widget_modify_bg(pwQuiz, GTK_STATE_NORMAL, &color);
    // }
    // gtk_widget_modify_bg(pwMain, GTK_STATE_NORMAL, &color);
    // gtk_widget_show_all(pwQuiz);
#endif
#if GTK_CHECK_VERSION(3,0,0)
    pwMainVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
#else
    pwMainVBox = gtk_vbox_new(FALSE, 2);
#endif

    gtk_container_add(GTK_CONTAINER(DialogArea(pwQuiz, DA_MAIN)), pwMainVBox);

   AddText(pwMainVBox, _("\nSelect a category to start playing"));

#if GTK_CHECK_VERSION(3,0,0)
    pwMainHBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
#else
    pwMainHBox = gtk_hbox_new(FALSE, 2);
#endif
    gtk_box_pack_start(GTK_BOX(pwMainVBox), pwMainHBox, FALSE, FALSE, 0);
    // gtk_container_add(GTK_CONTAINER(DialogArea(pwQuiz, DA_MAIN)), pwMainHBox);
    gtk_box_pack_start(GTK_BOX(pwMainHBox), pwScrolled, TRUE, TRUE, 0);
    gtk_widget_set_size_request(pwScrolled, 100, 200);//-1);
#if GTK_CHECK_VERSION(3,0,0)
    pwVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
#else
    pwVBox = gtk_vbox_new(FALSE, 2);
#endif
    gtk_box_pack_start(GTK_BOX(pwMainHBox), pwVBox, FALSE, FALSE, 0);
    // gtk_container_set_border_width(GTK_CONTAINER(pwVBox), 8);

#if GTK_CHECK_VERSION(3, 8, 0)
    gtk_container_add(GTK_CONTAINER(pwScrolled), treeview);
#else
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(pwScrolled), treeview);
#endif
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(pwScrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    g_signal_connect(selection, "changed", G_CALLBACK(on_changed), NULL);

// #if GTK_CHECK_VERSION(3,0,0)
//     hb1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
// #else
//     hb1 = gtk_hbox_new(FALSE, 0);
// #endif
    // gtk_box_pack_start(GTK_BOX(pwVBox), hb1, FALSE, FALSE, 0);


    if(numCategories>0) {
        /*first, if single position to add from current board*/
        if(qNow_NDBeforeMoveError <-0.001) {
            addPos1Button = gtk_button_new_with_label(_("Add position to category"));
            g_signal_connect(addPos1Button, "clicked", G_CALLBACK(AddPositionClicked), GTK_TREE_VIEW(treeview));
            gtk_box_pack_start(GTK_BOX(pwVBox), addPos1Button, FALSE, FALSE, 0);
            gtk_widget_set_sensitive(addPos1Button, (currentCategoryIndex>=0 && currentCategoryIndex<numCategories));
        } else {
            /*now it means there is a no-double event before a move event,
            so we need to allow users to add both*/
            addPos1Button = gtk_button_new_with_label(_("Add move decision to category"));
            g_signal_connect(addPos1Button, "clicked", G_CALLBACK(AddPositionClicked), GTK_TREE_VIEW(treeview));
            gtk_box_pack_start(GTK_BOX(pwVBox), addPos1Button, FALSE, FALSE, 0);
            gtk_widget_set_sensitive(addPos1Button, (currentCategoryIndex>=0 && currentCategoryIndex<numCategories));

            addPos2Button = gtk_button_new_with_label(_("Add double/no-double decision to category"));
            g_signal_connect(addPos2Button, "clicked", G_CALLBACK(AddNDPositionClicked), GTK_TREE_VIEW(treeview));
            gtk_box_pack_start(GTK_BOX(pwVBox), addPos2Button, FALSE, FALSE, 0);
            gtk_widget_set_sensitive(addPos2Button, (currentCategoryIndex>=0 && currentCategoryIndex<numCategories));
        }
    }

#if GTK_CHECK_VERSION(3,0,0)
    gtk_box_pack_start(GTK_BOX(pwVBox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 2);
#else
    gtk_box_pack_start(GTK_BOX(pwVBox), gtk_hseparator_new(), FALSE, FALSE, 2);
#endif

    addButton = gtk_button_new_with_label(_("Add category"));
    g_signal_connect(addButton, "clicked", G_CALLBACK(AddCategoryClicked), GTK_TREE_VIEW(treeview));
    gtk_box_pack_start(GTK_BOX(pwVBox), addButton, FALSE, FALSE, 0);
 
    renameButton = gtk_button_new_with_label(_("Rename category"));
    g_signal_connect(renameButton, "clicked", G_CALLBACK(RenameCategoryClicked), GTK_TREE_VIEW(treeview));
    gtk_box_pack_start(GTK_BOX(pwVBox), renameButton, FALSE, FALSE, 0);
 
    delButton = gtk_button_new_with_label(_("Delete category"));
    g_signal_connect(delButton, "clicked", G_CALLBACK(DeleteCategoryClicked), GTK_TREE_VIEW(treeview));
    gtk_box_pack_start(GTK_BOX(pwVBox), delButton, FALSE, FALSE, 4);

#if GTK_CHECK_VERSION(3,0,0)
    gtk_box_pack_start(GTK_BOX(pwVBox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 2);
#else
    gtk_box_pack_start(GTK_BOX(pwVBox), gtk_hseparator_new(), FALSE, FALSE, 2);
#endif

    helpButton = gtk_button_new_with_label(_("Explanations"));
    g_signal_connect(helpButton, "clicked", G_CALLBACK(ExplanationsClicked), pwQuiz);
    gtk_box_pack_start(GTK_BOX(pwVBox), helpButton, FALSE, FALSE, 4);
    gtk_widget_set_tooltip_text(helpButton, _("Click to obtain more explanations on the quiz mode")); 

    // startButton = gtk_button_new_from_stock(GTK_STOCK_GO_FORWARD);
    startButton = gtk_button_new(); 
    //gtk_button_new_with_label(_("Start quiz!"));
    // button = gtk_button_new();
    // GtkWidget *image = gtk_image_new_from_stock(GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_DIALOG);
    GtkWidget *image = gtk_image_new_from_stock(GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_BUTTON);
    // gtk_button_set_use_stock (GTK_BUTTON(startButton), FALSE);
    gtk_button_set_image(GTK_BUTTON(startButton), image);
    // gtk_button_set_label(GTK_BUTTON(startButton), _("Start quiz!"));
    // gtk_button_set_always_show_image (GTK_BUTTON (button), TRUE);
    // gtk_widget_set_sensitive(startButton, (&selected_iter!=NULL));

    g_message("in window: currentCategoryIndex=%d",currentCategoryIndex);
    
    gtk_widget_set_sensitive(startButton, (currentCategoryIndex>=0 && currentCategoryIndex<numCategories));
    gtk_widget_set_sensitive(renameButton, (currentCategoryIndex>=0 && currentCategoryIndex<numCategories));
    gtk_widget_set_sensitive(delButton, (currentCategoryIndex>=0 && currentCategoryIndex<numCategories));
    // gtk_button_set_relief(GTK_BUTTON(startButton), GTK_RELIEF_NONE);

    g_signal_connect(startButton, "clicked", G_CALLBACK(StartQuiz), GTK_TREE_VIEW(treeview));
    gtk_box_pack_start(GTK_BOX(pwMainVBox), startButton, TRUE, TRUE, 10);
    // gtk_dialog_add_button(GTK_DIALOG(pwQuiz), _("Start quiz!"),
    //                           GTK_RESPONSE_YES);
    // gtk_dialog_set_default_response(GTK_DIALOG(pwQuiz), GTK_RESPONSE_YES);

    g_signal_connect(G_OBJECT(treeview), "button-press-event", G_CALLBACK(QuizManageClicked), GTK_TREE_VIEW(treeview));
    // g_signal_connect(G_OBJECT(treeview), "button-release-event", G_CALLBACK(QuizManageReleased), treeview);
    

    g_object_weak_ref(G_OBJECT(pwQuiz), DestroyQuizDialog, NULL);

    GTKRunDialog(pwQuiz);
}

// /* Now we are back from the hint function within quiz mode. 
// - The position category is well known. 
// - The values have already been updated and saved in a file 
//         (in MoveListUpdate->qUpdate->SaveFullPositionFile)
// We need to ask the player whether to play again within this category.
//     (YES) We want to pick a new position in the category and start
//     (NO) We get back to the main panel in case the user wants to 
//             change category.
// */
// extern void BackFromHint (void) {
//     char buf[200];
//     // counterForFile++;
//     sprintf(buf,_("%d positions played in category %s (which has %d positions)."
//         "\n\n Play another position?"), counterForFile,categories[currentCategoryIndex].name,
//         categories[currentCategoryIndex].number);

//     if (GTKGetInputYN(buf)) {
//         LoadPositionAndStart();
//     } else {
//         fInQuizMode=FALSE;
//         QuizConsole();
//     }
// }

extern void StartQuiz(GtkWidget * UNUSED(pw), GtkTreeView * treeview) {
    g_message("in StartQuiz");

    // currentCategory = GetSelectedCategory(treeview);
    // if(!currentCategory) {
    //     GTKMessage(_("Error: you forgot to select a position category"), DT_INFO);
    //     QuizConsole();
    //     return;
    // }

    currentCategoryIndex = GetSelectedCategoryIndex(GTK_TREE_VIEW(treeview));
    g_message("currentCategoryIndex=%d",currentCategoryIndex);
    // gtk_widget_destroy(gtk_widget_get_toplevel(pw));
    if(currentCategoryIndex<0 || currentCategoryIndex>=numCategories) {
        GTKMessage(_("Error: you forgot to select a position category"), DT_INFO);
        // QuizConsole();
        return;
    }

    /* the following function:
    - updates positionsFileFullPath,
    - opens the corrsponding file, 
    - and saves the parsing results in the categories static array */
    int result= OpenQuizPositionsFile(currentCategoryIndex);
    // g_free(currentCategory); //when should we free a static var that is to be reused?

    /*empty file, no positions*/
    if(result==0) {
        GTKMessage(_("Error: problem with the file of this category"), DT_INFO);
        // QuizConsole();
        return;
    }

    /*start!*/
    DestroyQuizDialog(NULL,NULL);
    if(!fInQuizMode)
        TurnOnQuizMode();
    // // sprintf(name0BeforeQuiz, "%s", ap[0].szName);
    // // sprintf(name1BeforeQuiz, "%s", ap[1].szName);
    // name0BeforeQuiz=g_strdup(ap[0].szName);
    // name1BeforeQuiz=g_strdup(ap[1].szName);

    counterForFile=0; /*first one for this category in this round*/
    LoadPositionAndStart();
}

extern void
CommandQuiz(char *UNUSED(sz))
{
    QuizConsole();
    // GdkColor color;
    // color.red = 0xffff;
    // color.green = 0xffff;
    // color.blue = 0;
    // gtk_window_set_title(GTK_WINDOW(pwMain), "hello");
    // if (gdk_color_parse("#c0deed", &color)) {
    //     gtk_widget_modify_bg(pwMain, GTK_STATE_NORMAL, &color);
    // } else {
    //     gtk_widget_modify_bg(pwMain, GTK_STATE_NORMAL, &color);
    // }
    // // gtk_widget_modify_bg(pwMain, GTK_STATE_NORMAL, &color);
    // gtk_widget_show_all(pwMain);
        // gtk_widget_modify_bg(pwMain, GTK_STATE_NORMAL, &gtk_widget_get_style(pwMain)->bg[GTK_STATE_SELECTED]);

    // gdk_color_parse ("black", &color);
    // // gdk_color_parse (BGCOLOR, &color);
    // gtk_widget_modify_bg(pwMain, GTK_STATE_NORMAL, &color);
    // gtk_widget_modify_bg(pwMain, GTK_STATE_NORMAL, &gtk_widget_get_style(pwMain)->bg[GTK_STATE_PRELIGHT]);

}



/* *************** */

extern void
GTKAnalyzeFile(void) {
    // QuizConsole();
    CommandQuiz("");

/* ************ END OF QUIZ ************************** */

#if (0)    
    // g_message("GTKAnalyzeFile(): %d\n", AnalyzeFileSettingDef);
    if (AnalyzeFileSettingDef == AnalyzeFileBatch) {
        GTKBatchAnalyse(NULL, 0, NULL);
    } else if (AnalyzeFileSettingDef == AnalyzeFileRegular) {
        AnalyzeSingleFile();
    } else { //   AnalyzeFileSmart, 
        SmartAnalyze();
    }
    return;
#endif
}


extern void
GTKBatchAnalyse(gpointer UNUSED(p), guint UNUSED(n), GtkWidget * UNUSED(pw))
{
    gchar *folder = NULL;
    GSList *filenames = NULL;
    GtkWidget *fc;
    static gchar *last_folder = NULL;
    GtkWidget *add_to_db;
    fInterrupt = FALSE;

    folder = last_folder ? last_folder : default_import_folder;

    fc = GnuBGFileDialog(_("Select files to analyse"), folder, NULL, GTK_FILE_CHOOSER_ACTION_OPEN);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(fc), TRUE);
    add_import_filters(GTK_FILE_CHOOSER(fc));

    add_to_db = gtk_check_button_new_with_label(_("Add to database"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(add_to_db), TRUE);

    gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(fc), add_to_db);

    if (gtk_dialog_run(GTK_DIALOG(fc)) == GTK_RESPONSE_ACCEPT) {
        filenames = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(fc));
    }
    if (filenames) {
        gboolean add_to_db_set;
        int fConfirmNew_s;

        g_free(last_folder);
        last_folder = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(fc));
        add_to_db_set = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(add_to_db));
        gtk_widget_destroy(fc);

        if (!get_input_discard())
            return;

        fConfirmNew_s = fConfirmNew;
        fConfirmNew = 0;
        batch_create_dialog_and_run(filenames, add_to_db_set);
        fConfirmNew = fConfirmNew_s;
    } else
        gtk_widget_destroy(fc);
}
