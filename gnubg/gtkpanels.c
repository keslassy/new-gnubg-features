/*
 * Copyright (C) 2004-2019 Jon Kinsey <jonkinsey@gmail.com>
 * Copyright (C) 2005-2018 the AUTHORS
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
 * $Id: gtkpanels.c,v 1.95 2022/10/22 18:34:13 plm Exp $
 */

#include "config.h"
#include "gtklocdefs.h"

#include <stdlib.h>
#include <ctype.h>
#include "backgammon.h"
#include <string.h>
#include "gtkboard.h"
#include "gtkgame.h"
#include "gtktoolbar.h"
#include "positionid.h"
#if defined(USE_BOARD3D)
#include "inc3d.h"
#endif

static int fDisplayPanels = TRUE;
static int fDockPanels = TRUE;

#define NUM_CMD_HISTORY 10
#define KEY_TAB -247

typedef struct {
    GtkWidget *pwEntry, *pwHelpText, *cmdEntryCombo;
    int showHelp;
    int numHistory;
    int completing;
    char *cmdString;
    char *cmdHistory[NUM_CMD_HISTORY];
} CommandEntryData_T;

static CommandEntryData_T cedPanel = {
    NULL, NULL, NULL, 0, 0, 0, NULL,
    {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}
};


static GtkWidget *game_select_combo = NULL;

typedef gboolean(*panelFun) (void);

static gboolean DeleteMessage(void);
static gboolean DeleteAnalysis(void);
static gboolean DeleteAnnotation(void);
static gboolean DeleteGame(void);
static gboolean DeleteQuizWindow(void);
static gboolean DeleteTheoryWindow(void);
static gboolean DeleteCommandWindow(void);

static gboolean ShowAnnotation(void);
static gboolean ShowMessage(void);
static gboolean ShowAnalysis(void);
static gboolean ShowTheoryWindow(void);
static gboolean ShowQuizWindow(void);
static gboolean ShowCommandWindow(void);

typedef struct {
    const char *winName;
    int showing;
    int docked;
    int dockable;
    int undockable;
    panelFun showFun;
    panelFun hideFun;
    GtkWidget *pwWin;
    windowgeometry wg;
} windowobject;

/* Set up window and panel details 
Careful! It needs to be in the same order as the gnubgwindow enum definition! Else some
windows open together...
*/
static windowobject woPanel[NUM_WINDOWS] = {
    /* main window */
    {
     "main",
     TRUE, FALSE, FALSE, FALSE,
     NULL, NULL,
     0,
     {0, 0, 20, 20, FALSE}
     },
    /* game list */
    {
     "game",
     TRUE, TRUE, TRUE, TRUE,
     ShowGameWindow, DeleteGame,
     0,
     {400, 200, 20, 100, FALSE}
     },
    /* analysis */
    {
     "analysis",
     TRUE, TRUE, TRUE, TRUE,
     ShowAnalysis, DeleteAnalysis,
     0,
     {600, 300, 40, 120, FALSE}
     },
    /* annotation */
    {
     "annotation",
     FALSE, TRUE, TRUE, FALSE,
     ShowAnnotation, DeleteAnnotation,
     0,
     {400, 200, 50, 50, FALSE}
     },
    /* hint */
    {
     "hint",
     FALSE, FALSE, FALSE, FALSE,
     NULL, NULL,
     0,
     {0, 450, 20, 20, FALSE}
     },
    /* quiz */
    {
     "quiz",
     FALSE, TRUE, TRUE, TRUE,
     ShowQuizWindow, DeleteQuizWindow,
     0,
     {160, 20, 50, 50, FALSE} /*    int nWidth, nHeight; int nPosX, nPosY, max;*/
     },
    /* message */
    {
     "message",
     FALSE, TRUE, TRUE, TRUE,
     ShowMessage, DeleteMessage,
     0,
     {300, 200, 20, 20, FALSE}
     },
    /* command */
    {
     "command",
     FALSE, TRUE, TRUE, TRUE,
     ShowCommandWindow, DeleteCommandWindow,
     0,
     {0, 0, 20, 20, FALSE}
     },
    /* theory */
    {
     "theory",
     FALSE, TRUE, TRUE, TRUE,
     ShowTheoryWindow, DeleteTheoryWindow,
     0,
     {160, 0, 20, 20, FALSE}
     }
};


static gboolean
ShowPanel(gnubgwindow window)
{
    setWindowGeometry(window);
    if (!woPanel[window].docked && gtk_widget_get_window(woPanel[window].pwWin))
        gdk_window_raise(gtk_widget_get_window(woPanel[window].pwWin));

    woPanel[window].showing = TRUE;
    /* Avoid showing before main window */
    if (gtk_widget_get_realized(pwMain))
        gtk_widget_show_all(woPanel[window].pwWin);

    return TRUE;
}

extern gboolean
ShowGameWindow(void)
{
    ShowPanel(WINDOW_GAME);
#if defined(USE_GTKUIMANAGER)
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_ui_manager_get_widget(puim,
                                                                                 "/MainMenu/ViewMenu/PanelsMenu/GameRecord")),
                                   TRUE);
#else
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget(pif,
                                                                                   "/View/Panels/Game record")), TRUE);
#endif
    return TRUE;
}

static gboolean
ShowAnnotation(void)
{
#if defined(USE_GTKUIMANAGER)
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_ui_manager_get_widget(puim,
                                                                                 "/MainMenu/ViewMenu/PanelsMenu/Commentary")),
                                   TRUE);
#else
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget(pif,
                                                                                   "/View/Panels/Commentary")), TRUE);
#endif

    woPanel[WINDOW_ANNOTATION].showing = TRUE;
    /* Avoid showing before main window */
    if (gtk_widget_get_realized(pwMain))
        gtk_widget_show_all(woPanel[WINDOW_ANNOTATION].pwWin);
    return TRUE;
}

static gboolean
ShowMessage(void)
{
    ShowPanel(WINDOW_MESSAGE);
#if defined(USE_GTKUIMANAGER)
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_ui_manager_get_widget(puim,
                                                                                 "/MainMenu/ViewMenu/PanelsMenu/Message")),
                                   TRUE);
#else
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget(pif, "/View/Panels/Message")), TRUE);
#endif
    return TRUE;
}

static gboolean
ShowAnalysis(void)
{
    ShowPanel(WINDOW_ANALYSIS);
#if defined(USE_GTKUIMANAGER)
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_ui_manager_get_widget(puim,
                                                                                 "/MainMenu/ViewMenu/PanelsMenu/Analysis")),
                                   TRUE);

#else
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget(pif,
                                                                                   "/View/Panels/Analysis")), TRUE);
#endif
    return TRUE;
}

static gboolean
ShowTheoryWindow(void)
{
    ShowPanel(WINDOW_THEORY);
#if defined(USE_GTKUIMANAGER)
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_ui_manager_get_widget(puim,
                                                                                 "/MainMenu/ViewMenu/PanelsMenu/Theory")),
                                   TRUE);
#else
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget(pif, "/View/Panels/Theory")), TRUE);
#endif
    return TRUE;
}

static gboolean
ShowQuizWindow(void)
{
    // GTKMessage(_("ShowQuizWindow"), DT_INFO);
    ShowPanel(WINDOW_QUIZ);
#if defined(USE_GTKUIMANAGER)
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_ui_manager_get_widget(puim,
                                                                                 "/MainMenu/ViewMenu/PanelsMenu/Quiz")),
                                   TRUE);
#else
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget(pif, "/View/Panels/Quiz")), TRUE);
#endif
    return TRUE;
}

static gboolean
ShowCommandWindow(void)
{
    ShowPanel(WINDOW_COMMAND);
#if defined(USE_GTKUIMANAGER)
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_ui_manager_get_widget(puim,
                                                                                 "/MainMenu/ViewMenu/PanelsMenu/Command")),
                                   TRUE);
#else
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget(pif, "/View/Panels/Command")), TRUE);
#endif
    return TRUE;
}

static void
CreatePanel(gnubgwindow window, GtkWidget * pWidget, char *winTitle, const char *windowRole)
{
    if (!woPanel[window].docked) {
        woPanel[window].pwWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(woPanel[window].pwWin), winTitle);
        gtk_window_set_role(GTK_WINDOW(woPanel[window].pwWin), windowRole);
        gtk_window_set_type_hint(GTK_WINDOW(woPanel[window].pwWin), GDK_WINDOW_TYPE_HINT_UTILITY);

        setWindowGeometry(window);
        gtk_container_add(GTK_CONTAINER(woPanel[window].pwWin), pWidget);
        gtk_window_add_accel_group(GTK_WINDOW(woPanel[window].pwWin), pagMain);

        g_signal_connect(G_OBJECT(woPanel[window].pwWin), "delete_event", G_CALLBACK(woPanel[window].hideFun), NULL);
    } else
        woPanel[window].pwWin = pWidget;
}

static void
CreateMessageWindow(void)
{
    GtkWidget *psw;

    pwMessageText = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(pwMessageText), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(pwMessageText), FALSE);
    psw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(psw), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    gtk_widget_set_size_request(psw, -1, 150);
    gtk_container_add(GTK_CONTAINER(psw), pwMessageText);
    CreatePanel(WINDOW_MESSAGE, psw, _("GNU Backgammon - Messages"), "messages");
}

static GtkWidget *pwTheoryView = NULL;
static GtkWidget *pwQuizView = NULL;


void
UpdateTheoryData(BoardData * bd, int UpdateType, const TanBoard points)
{
    char *pc;
    GtkTreeIter iter;
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(pwTheoryView)));

    if (!pwTheoryView)
        return;

    if (UpdateType & TT_PIPCOUNT) {
        if (ms.gs != GAME_NONE) {
            int diff;
            unsigned int anPip[2];
            PipCount(points, anPip);

            diff = anPip[0] - anPip[1];
            if (diff == 0)
                pc = g_strdup_printf(_("equal"));
            else
                pc = g_strdup_printf("%d %s", abs(diff), (diff > 0) ? _("ahead") : _("behind"));

            gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(store), &iter, NULL, 0);
            gtk_list_store_set(store, &iter, 1, pc, -1);
            g_free(pc);
        }
    }
    if (UpdateType & TT_EPC) {
        if (ms.gs != GAME_NONE) {
            float arEPC[2];

            if (EPC(points, arEPC, NULL, NULL, NULL, TRUE)) {   /* no EPCs available */
                gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(store), &iter, NULL, 1);
                gtk_list_store_set(store, &iter, 1, "", -1);
            } else {
                pc = g_strdup_printf("%.2f (%+.1f)", arEPC[1], arEPC[1] - arEPC[0]);
                gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(store), &iter, NULL, 1);
                gtk_list_store_set(store, &iter, 1, pc, -1);
                g_free(pc);
            }
        }
    }

    if (UpdateType & TT_RETURNHITS) {
        pc = NULL;
        if (bd->valid_move) {
            TanBoard anBoard;

            PositionFromKey(anBoard, &bd->valid_move->key);
            pc = ReturnHits(anBoard);
        }
        if (pc) {
            gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(store), &iter, NULL, 2);
            gtk_list_store_set(store, &iter, 1, pc, -1);
            g_free(pc);
        } else {
            gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(store), &iter, NULL, 2);
            gtk_list_store_set(store, &iter, 1, "", -1);
        }
    }

    if (UpdateType & TT_KLEINCOUNT) {
        if (ms.gs != GAME_NONE) {
            float fKC;
            unsigned int anPip[2];
            PipCount(points, anPip);

            fKC = KleinmanCount(anPip[1], anPip[0]);
            if (fKC >= 0) {
                pc = g_strdup_printf("%.4f", fKC);
                gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(store), &iter, NULL, 3);
                gtk_list_store_set(store, &iter, 1, pc, -1);
                g_free(pc);
            } else {
                gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(store), &iter, NULL, 3);
                gtk_list_store_set(store, &iter, 1, "", -1);
            }
        }
    }
}

static GtkWidget *
CreateTheoryWindow(void)
{
    GtkListStore *store;
    GtkTreeIter iter;
    GtkCellRenderer *renderer;

    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING, -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, _("Pip count"), 1, "", -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, _("EPC"), 1, "", -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, _("Return hits"), 1, "", -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, _("Kleinman count"), 1, "", -1);

    pwTheoryView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(pwTheoryView), -1, NULL, renderer, "text", 0, NULL);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(pwTheoryView), -1, NULL, renderer, "text", 1, NULL);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(pwTheoryView), FALSE);

    CreatePanel(WINDOW_THEORY, pwTheoryView, _("GNU Backgammon - Theory"), "theory");
    return woPanel[WINDOW_THEORY].pwWin;
}


/* ********** */

/* inspired by gtk.org tutorial */
enum
{
  COL_NAME = 0,
  COL_AGE,
  NUM_COLS
} ;


static GtkTreeModel *
create_and_fill_model (void)
{
  GtkListStore *store = gtk_list_store_new (NUM_COLS,
                                            G_TYPE_STRING,
                                            G_TYPE_UINT);

  /* Append a row and fill in some data */
  GtkTreeIter iter;
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      COL_NAME, "Heinz El-Mann",
                      COL_AGE, 51,
                      -1);

  /* append another row and fill in some data */
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      COL_NAME, "Jane Doe",
                      COL_AGE, 23,
                      -1);

  /* ... and a third row */
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      COL_NAME, "Joe Bungop",
                      COL_AGE, 91,
                      -1);

  return GTK_TREE_MODEL (store);
}

static GtkWidget *
create_view_and_model (void)
{
  GtkWidget *view = gtk_tree_view_new ();

  GtkCellRenderer *renderer;

  /* --- Column #1 --- */
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                               "Name",  
                                               renderer,
                                               "text", COL_NAME,
                                               NULL);

  /* --- Column #2 --- */
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                               "Age",  
                                               renderer,
                                               "text", COL_AGE,
                                               NULL);

  GtkTreeModel *model = create_and_fill_model ();

  gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

  /* The tree view has acquired its own reference to the
   *  model, so we can drop ours. That way the model will
   *  be freed automatically when the tree view is destroyed
   */
  g_object_unref (model);

  return view;
}

#if(0) /**************/
int
main (int argc, char **argv)
{
  gtk_init (&argc, &argv);

  GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "destroy", gtk_main_quit, NULL);

  GtkWidget *view = create_view_and_model ();

  gtk_container_add (GTK_CONTAINER (window), view);

  gtk_widget_show_all (window);

  gtk_main ();

  return 0;
}

#endif /***********/

static GtkWidget *
CreateQuizWindow(void)
{




    //   GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    //   g_signal_connect (window, "destroy", gtk_main_quit, NULL);
    //   GtkWidget *view = create_view_and_model ();
    pwQuizView=create_view_and_model ();
    // gtk_container_add (GTK_CONTAINER (window), view);
    //   gtk_widget_show_all (window);

    CreatePanel(WINDOW_QUIZ, pwQuizView, _("GNU Backgammon - Quiz"), "quiz");
    return woPanel[WINDOW_QUIZ].pwWin;



    // GtkListStore *store;
    // GtkTreeIter iter;
    // GtkCellRenderer *renderer;

    // store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING, -1);
    // gtk_list_store_append(store, &iter);
    // gtk_list_store_set(store, &iter, 0, _("Pip count"), 1, "", -1);
    // gtk_list_store_append(store, &iter);
    // gtk_list_store_set(store, &iter, 0, _("EPC"), 1, "", -1);
    // gtk_list_store_append(store, &iter);
    // gtk_list_store_set(store, &iter, 0, _("Return hits"), 1, "", -1);
    // gtk_list_store_append(store, &iter);
    // gtk_list_store_set(store, &iter, 0, _("Kleinman count"), 1, "", -1);

    // pwQuizView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    // g_object_unref(store);
    // renderer = gtk_cell_renderer_text_new();
    // gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(pwQuizView), -1, NULL, renderer, "text", 0, NULL);
    // renderer = gtk_cell_renderer_text_new();
    // gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(pwQuizView), -1, NULL, renderer, "text", 1, NULL);
    // gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(pwQuizView), FALSE);

    // g_message("here");
    // CreatePanel(WINDOW_QUIZ, pwQuizView, _("GNU Backgammon - Quiz"), "quiz");
    // return woPanel[WINDOW_QUIZ].pwWin;

    //      GtkWidget *psw;

    // pwMessageText = gtk_text_view_new();
    // gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(pwMessageText), GTK_WRAP_WORD_CHAR);
    // gtk_text_view_set_editable(GTK_TEXT_VIEW(pwMessageText), FALSE);
    // psw = gtk_scrolled_window_new(NULL, NULL);
    // gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(psw), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    // gtk_widget_set_size_request(psw, -1, 150);
    // gtk_container_add(GTK_CONTAINER(psw), pwMessageText);
    // CreatePanel(WINDOW_QUIZ, psw,  _("GNU Backgammon - Quiz"), "quiz");
    // return woPanel[WINDOW_QUIZ].pwWin;
}


/* Display help for command (pStr) in widget (pwText) */
static void
ShowHelp(GtkWidget * pwText, char *pStr)
{
    command *pc, *pcFull;
    char szCommand[128], szUsage[128], szBuf[255], *cc, *pTemp;
    command cTop = { NULL, NULL, NULL, NULL, acTop };
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    /* Copy string as token striping corrupts string */
    pTemp = strdup(pStr);
    cc = CheckCommand(pTemp, acTop);

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pwText));

    if (cc) {
        sprintf(szBuf, _("Unknown keyword: %s\n"), cc);
        gtk_text_buffer_set_text(buffer, szBuf, -1);
    } else if ((pc = FindHelpCommand(&cTop, pStr, szCommand, szUsage)) != NULL) {
        gtk_text_buffer_set_text(buffer, "", -1);
        gtk_text_buffer_get_end_iter(buffer, &iter);
        if (!*pStr) {
            gtk_text_buffer_insert(buffer, &iter, _("Available commands:\n"), -1);
        } else {
            if (!pc->szHelp) {  /* The command is an abbreviation, search for the full version */
                for (pcFull = acTop; pcFull->sz; pcFull++) {
                    if (pcFull->pf == pc->pf && pcFull->szHelp) {
                        pc = pcFull;
                        strcpy(szCommand, pc->sz);
                        break;
                    }
                }
            }

            sprintf(szBuf, "%s %s\n", _("Command:"), szCommand);
            gtk_text_buffer_insert(buffer, &iter, szBuf, -1);
            gtk_text_buffer_insert(buffer, &iter, gettext(pc->szHelp), -1);
            sprintf(szBuf, "\n\n%s %s", _("Usage:"), szUsage);
            gtk_text_buffer_insert(buffer, &iter, szBuf, -1);

            if (!(pc->pc && pc->pc->sz))
                gtk_text_buffer_insert(buffer, &iter, "\n", -1);
            else {
                gtk_text_buffer_insert(buffer, &iter, _("<subcommand>\n"), -1);
                gtk_text_buffer_insert(buffer, &iter, _("Available subcommands:\n"), -1);
            }
        }

        pc = pc->pc;

        while (pc && pc->sz) {
            if (pc->szHelp) {
                sprintf(szBuf, "%-15s\t%s\n", pc->sz, gettext(pc->szHelp));
                gtk_text_buffer_insert(buffer, &iter, szBuf, -1);
            }
            pc++;
        }
    }
    free(pTemp);
}

static void
PopulateCommandHistory(CommandEntryData_T * pData)
{
    int i;

    for (i = 0; i < pData->numHistory; i++) {
        gtk_combo_box_text_remove(GTK_COMBO_BOX_TEXT(pData->cmdEntryCombo), i);
        gtk_combo_box_text_insert_text(GTK_COMBO_BOX_TEXT(pData->cmdEntryCombo), i, pData->cmdHistory[i]);
    }
}

static void
CommandOK(GtkWidget * UNUSED(pw), CommandEntryData_T * pData)
{
    int i, found = -1;
    pData->cmdString = gtk_editable_get_chars(GTK_EDITABLE(pData->pwEntry), 0, -1);

    /* Update command history */

    /* See if already in history */
    for (i = 0; i < pData->numHistory; i++) {
        if (!StrCaseCmp(pData->cmdString, pData->cmdHistory[i])) {
            found = i;
            break;
        }
    }
    if (found != -1) {          /* Remove old entry */
        free(pData->cmdHistory[found]);
        pData->numHistory--;
        for (i = found; i < pData->numHistory; i++)
            pData->cmdHistory[i] = pData->cmdHistory[i + 1];
    }

    if (pData->numHistory == NUM_CMD_HISTORY) {
        free(pData->cmdHistory[NUM_CMD_HISTORY - 1]);
        pData->numHistory--;
    }
    for (i = pData->numHistory; i > 0; i--)
        pData->cmdHistory[i] = pData->cmdHistory[i - 1];

    pData->cmdHistory[0] = strdup(pData->cmdString);
    pData->numHistory++;
    PopulateCommandHistory(pData);
    gtk_entry_set_text(GTK_ENTRY(pData->pwEntry), "");

    if (pData->cmdString) {
        UserCommand(pData->cmdString);
        g_free(pData->cmdString);
    }
}

static void
CommandTextChange(GtkEntry * entry, CommandEntryData_T * pData)
{
    if (pData->showHelp) {
        /* Make sure the message window is showing */
        if (!PanelShowing(WINDOW_MESSAGE))
            PanelShow(WINDOW_MESSAGE);

        ShowHelp(pData->pwHelpText, gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1));
    }
}

static void
CreateHelpText(CommandEntryData_T * pData)
{
    GtkWidget *psw;
    pData->pwHelpText = gtk_text_view_new();
    gtk_widget_set_size_request(pData->pwHelpText, 400, 300);
    psw = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(psw), pData->pwHelpText);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(psw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    CommandTextChange(GTK_ENTRY(pData->pwEntry), pData);
}

static void
ShowHelpToggled(GtkWidget * widget, CommandEntryData_T * pData)
{
    if (!pData->pwHelpText)
        CreateHelpText(pData);

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        pData->showHelp = 1;
        CommandTextChange(GTK_ENTRY(pData->pwEntry), pData);
    } else
        pData->showHelp = 0;
    gtk_widget_grab_focus(pData->pwEntry);
}

/* Capitalize first letter of each word */
static void
Capitalize(char *str)
{
    int cap = 1;
    while (*str) {
        if (cap) {
            *str = g_ascii_toupper(*str);
            cap = 0;
        } else {
            if (*str == ' ')
                cap = 1;
            *str = g_ascii_tolower(*str);
        }
        str++;
    }
}

static gboolean
CommandKeyPress(GtkWidget * UNUSED(widget), GdkEventKey * event, CommandEntryData_T * pData)
{
    short k = (short) event->keyval;

    if (k == KEY_TAB) {         /* Tab press - auto complete */
        char szCommand[128], szUsage[128];
        command cTop = { NULL, NULL, NULL, NULL, acTop };
        if (FindHelpCommand(&cTop, gtk_editable_get_chars(GTK_EDITABLE(pData->pwEntry), 0, -1), szCommand, szUsage) != NULL) {
            Capitalize(szCommand);
            gtk_entry_set_text(GTK_ENTRY(pData->pwEntry), szCommand);
            gtk_editable_set_position(GTK_EDITABLE(pData->pwEntry), -1);
            return TRUE;
        }
        /* Gtk 1 not good at stopping focus moving - so just move back later */
        pData->completing = 1;
    }
    return FALSE;
}

static gboolean
CommandFocusIn(GtkWidget * UNUSED(widget), GdkEventFocus * UNUSED(eventDetails), CommandEntryData_T * pData)
{
    if (pData->completing) {
        /* Gtk 1 not good at stopping focus moving - so just move back now */
        pData->completing = 0;
        gtk_widget_grab_focus(pData->pwEntry);
        gtk_editable_set_position(GTK_EDITABLE(pData->pwEntry),
                                  (int) strlen(gtk_editable_get_chars(GTK_EDITABLE(pData->pwEntry), 0, -1)));
        return TRUE;
    } else
        return FALSE;
}

static GtkWidget *
CreateCommandWindow(void)
{
    GtkWidget *pwhbox, *pwvbox;
    GtkWidget *pwShowHelp;

#if GTK_CHECK_VERSION(3,0,0)
    pwvbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
    pwvbox = gtk_vbox_new(FALSE, 0);
#endif
    CreatePanel(WINDOW_COMMAND, pwvbox, _("GNU Backgammon - Command"), "command");

    cedPanel.cmdString = NULL;
    cedPanel.pwHelpText = pwMessageText;

    cedPanel.cmdEntryCombo = gtk_combo_box_text_new_with_entry();
    cedPanel.pwEntry = gtk_bin_get_child(GTK_BIN(cedPanel.cmdEntryCombo));
    gtk_entry_set_activates_default(GTK_ENTRY(cedPanel.pwEntry), FALSE);

    PopulateCommandHistory(&cedPanel);

    g_signal_connect(G_OBJECT(cedPanel.pwEntry), "changed", G_CALLBACK(CommandTextChange), &cedPanel);
    g_signal_connect(G_OBJECT(cedPanel.pwEntry), "key-press-event", G_CALLBACK(CommandKeyPress), &cedPanel);
    g_signal_connect(G_OBJECT(cedPanel.pwEntry), "activate", G_CALLBACK(CommandOK), &cedPanel);

#if GTK_CHECK_VERSION(3,0,0)
    pwhbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    pwhbox = gtk_hbox_new(FALSE, 0);
#endif
    gtk_box_pack_start(GTK_BOX(pwvbox), pwhbox, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(pwhbox), cedPanel.cmdEntryCombo, TRUE, TRUE, 10);

    pwShowHelp = gtk_toggle_button_new_with_label(_("Help"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pwShowHelp), cedPanel.showHelp);
    g_signal_connect(G_OBJECT(pwShowHelp), "toggled", G_CALLBACK(ShowHelpToggled), &cedPanel);

    gtk_box_pack_start(GTK_BOX(pwhbox), pwShowHelp, FALSE, FALSE, 5);
    g_signal_connect(G_OBJECT(pwShowHelp), "focus-in-event", G_CALLBACK(CommandFocusIn), &cedPanel);

    return woPanel[WINDOW_COMMAND].pwWin;
}

static GtkWidget *
CreateAnalysisWindow(void)
{
    GtkWidget *pHbox, *sw;
    GtkTextBuffer *buffer;
    if (!woPanel[WINDOW_ANALYSIS].docked) {
        GtkWidget *pwPaned;

#if GTK_CHECK_VERSION(3,0,0)
        pwPaned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
#else
        pwPaned = gtk_vpaned_new();
#endif

        woPanel[WINDOW_ANALYSIS].pwWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);

        gtk_window_set_title(GTK_WINDOW(woPanel[WINDOW_ANALYSIS].pwWin), _("GNU Backgammon - Annotation"));
        gtk_window_set_role(GTK_WINDOW(woPanel[WINDOW_ANALYSIS].pwWin), "annotation");
        gtk_window_set_type_hint(GTK_WINDOW(woPanel[WINDOW_ANALYSIS].pwWin), GDK_WINDOW_TYPE_HINT_UTILITY);

        setWindowGeometry(WINDOW_ANALYSIS);

        gtk_container_add(GTK_CONTAINER(woPanel[WINDOW_ANALYSIS].pwWin), pwPaned);
        gtk_window_add_accel_group(GTK_WINDOW(woPanel[WINDOW_ANALYSIS].pwWin), pagMain);

        gtk_paned_pack1(GTK_PANED(pwPaned), pwAnalysis = gtk_label_new(NULL), TRUE, FALSE);
#if GTK_CHECK_VERSION(3,0,0)
        pHbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
        pHbox = gtk_hbox_new(FALSE, 0);
#endif
        gtk_paned_pack2(GTK_PANED(pwPaned), pHbox, FALSE, TRUE);
    } else {
#if GTK_CHECK_VERSION(3,0,0)
        pHbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
        pHbox = gtk_hbox_new(FALSE, 0);
#endif
        gtk_box_pack_start(GTK_BOX(pHbox), pwAnalysis = gtk_label_new(NULL), TRUE, TRUE, 0);
#if GTK_CHECK_VERSION(3,0,0)
        pHbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
        pHbox = gtk_hbox_new(FALSE, 0);
#endif
    }

    pwCommentary = gtk_text_view_new();

    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(pwCommentary), GTK_WRAP_WORD);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(pwCommentary), TRUE);
    gtk_widget_set_sensitive(pwCommentary, FALSE);

    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(sw), pwCommentary);
    gtk_box_pack_start(GTK_BOX(pHbox), sw, TRUE, TRUE, 0);
    gtk_widget_set_size_request(sw, 100, 150);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pwCommentary));
    g_signal_connect(G_OBJECT(buffer), "changed", G_CALLBACK(CommentaryChanged), buffer);

    if (!woPanel[WINDOW_ANALYSIS].docked) {
        g_signal_connect(G_OBJECT(woPanel[WINDOW_ANALYSIS].pwWin), "delete_event",
                         G_CALLBACK(woPanel[WINDOW_ANALYSIS].hideFun), NULL);
        return woPanel[WINDOW_ANALYSIS].pwWin;
    } else {
        woPanel[WINDOW_ANALYSIS].pwWin = gtk_widget_get_parent(pwAnalysis);
        return pHbox;
    }
}

extern void
GTKGameSelectDestroy(void)
{
    game_select_combo = NULL;
}

extern void
GTKPopGame(int i)
{
    GtkTreeIter iter;
    GtkTreeModel *model;

    model = gtk_combo_box_get_model(GTK_COMBO_BOX(game_select_combo));
    while (gtk_tree_model_iter_nth_child(model, &iter, NULL, i))
        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
}

extern void
GTKAddGame(moverecord * pmr)
{
    char sz[128];
    GtkTreeModel *model;
    gint last_game;

    if (pmr->g.fCrawford && pmr->g.fCrawfordGame)
        sprintf(sz, _("Game %d: %d, %d Crawford"), pmr->g.i + 1, pmr->g.anScore[0], pmr->g.anScore[1]);
    else
        sprintf(sz, _("Game %d: %d, %d"), pmr->g.i + 1, pmr->g.anScore[0], pmr->g.anScore[1]);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(game_select_combo), sz);
    model = gtk_combo_box_get_model(GTK_COMBO_BOX(game_select_combo));
    last_game = gtk_tree_model_iter_n_children(model, NULL);
    GTKSetGame(last_game - 1);

    /* Update Crawford flag on the board */
    ms.fCrawford = pmr->g.fCrawford && pmr->g.fCrawfordGame;
    GTKSet(&ms.fCrawford);
}

extern void
GTKRegenerateGames(void)
{
    listOLD *pl;
    int i = gtk_combo_box_get_active(GTK_COMBO_BOX(game_select_combo));

    GL_SetNames();
    GTKPopGame(0);
    for (pl = lMatch.plNext; pl->p; pl = pl->plNext) {
        listOLD *plg = pl->p;
        GTKAddGame(plg->plNext->p);
    }

    GTKSetGame(i);
}

extern void
GTKSetGame(int i)
{
    gtk_combo_box_set_active(GTK_COMBO_BOX(game_select_combo), i);
}

static void
SelectGame(GtkWidget * pw, void *UNUSED(data))
{
    listOLD *pl;
    int i = 0;

    if (!plGame)
        return;

    i = gtk_combo_box_get_active(GTK_COMBO_BOX(pw));
    if (i == -1)
        return;
    for (pl = lMatch.plNext; i && pl->plNext->p; i--, pl = pl->plNext);

    if (pl->p == plGame)
        return;

    ChangeGame(pl->p);
}

static void
CreateGameWindow(void)
{
    GtkWidget *psw = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *pvbox, *phbox;

#if GTK_CHECK_VERSION(3,0,0)
    g_object_set(G_OBJECT(psw), "expand", TRUE, NULL);
    pvbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    phbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    pvbox = gtk_vbox_new(FALSE, 0);
    phbox = gtk_hbox_new(FALSE, 0);
#endif

    if (!woPanel[WINDOW_GAME].docked) {
        woPanel[WINDOW_GAME].pwWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);

        gtk_window_set_title(GTK_WINDOW(woPanel[WINDOW_GAME].pwWin), _("GNU Backgammon - Game record"));
        gtk_window_set_role(GTK_WINDOW(woPanel[WINDOW_GAME].pwWin), "game record");
        gtk_window_set_type_hint(GTK_WINDOW(woPanel[WINDOW_GAME].pwWin), GDK_WINDOW_TYPE_HINT_UTILITY);

        setWindowGeometry(WINDOW_GAME);

        gtk_container_add(GTK_CONTAINER(woPanel[WINDOW_GAME].pwWin), pvbox);
        gtk_window_add_accel_group(GTK_WINDOW(woPanel[WINDOW_GAME].pwWin), pagMain);
    }
    gtk_box_pack_start(GTK_BOX(pvbox), phbox, FALSE, FALSE, 4);

    game_select_combo = gtk_combo_box_text_new();
    g_signal_connect(G_OBJECT(game_select_combo), "changed", G_CALLBACK(SelectGame), NULL);
    gtk_box_pack_start(GTK_BOX(phbox), game_select_combo, TRUE, TRUE, 4);

    gtk_box_pack_end(GTK_BOX(pvbox), psw, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(psw), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

    gtk_widget_set_size_request(psw, -1, 150);
    gtk_container_add(GTK_CONTAINER(psw), GL_Create());

    if (!woPanel[WINDOW_GAME].docked) {
        g_signal_connect(G_OBJECT(woPanel[WINDOW_GAME].pwWin), "delete_event",
                         G_CALLBACK(woPanel[WINDOW_GAME].hideFun), NULL);
    } else
        woPanel[WINDOW_GAME].pwWin = pvbox;
}

static void
CreateHeadWindow(gnubgwindow panel, const char *sz, GtkWidget * pwWidge)
{
    GtkWidget *pwLab = gtk_label_new(sz);
    GtkWidget *pwVbox, *pwHbox;
    GtkWidget *pwX = gtk_button_new();

#if GTK_CHECK_VERSION(3,0,0)
    pwVbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    pwHbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    pwVbox = gtk_vbox_new(FALSE, 0);
    pwHbox = gtk_hbox_new(FALSE, 0);
#endif

    gtk_button_set_image(GTK_BUTTON(pwX), gtk_image_new_from_icon_name("window-close", GTK_ICON_SIZE_MENU));
    g_signal_connect(G_OBJECT(pwX), "clicked", G_CALLBACK(woPanel[panel].hideFun), NULL);

    gtk_box_pack_start(GTK_BOX(pwVbox), pwHbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pwHbox), pwLab, FALSE, FALSE, 10);
    gtk_box_pack_end(GTK_BOX(pwHbox), pwX, FALSE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(pwVbox), pwWidge, TRUE, TRUE, 0);

    woPanel[panel].pwWin = pwVbox;
}

static void
CreatePanels(void)
{
    CreateGameWindow();
    gtk_box_pack_start(GTK_BOX(pwPanelVbox), woPanel[WINDOW_GAME].pwWin, TRUE, TRUE, 0);

    CreateHeadWindow(WINDOW_ANNOTATION, _("Commentary"), CreateAnalysisWindow());
    gtk_box_pack_start(GTK_BOX(pwPanelVbox), woPanel[WINDOW_ANALYSIS].pwWin, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pwPanelVbox), woPanel[WINDOW_ANNOTATION].pwWin, FALSE, FALSE, 0);

    CreateMessageWindow();
    CreateHeadWindow(WINDOW_MESSAGE, _("Messages"), woPanel[WINDOW_MESSAGE].pwWin);
    gtk_box_pack_start(GTK_BOX(pwPanelVbox), woPanel[WINDOW_MESSAGE].pwWin, FALSE, FALSE, 0);

    CreateHeadWindow(WINDOW_COMMAND, _("Command"), CreateCommandWindow());
    gtk_box_pack_start(GTK_BOX(pwPanelVbox), woPanel[WINDOW_COMMAND].pwWin, FALSE, FALSE, 0);

    CreateHeadWindow(WINDOW_THEORY, _("Theory"), CreateTheoryWindow());
    gtk_box_pack_start(GTK_BOX(pwPanelVbox), woPanel[WINDOW_THEORY].pwWin, FALSE, FALSE, 0);

    CreateHeadWindow(WINDOW_QUIZ, _("Quiz"), CreateQuizWindow());
    // CreateQuizWindow();
    // CreateHeadWindow(WINDOW_QUIZ, _("Quiz"), woPanel[WINDOW_QUIZ].pwWin);
    gtk_box_pack_start(GTK_BOX(pwPanelVbox), woPanel[WINDOW_QUIZ].pwWin, FALSE, FALSE, 0);
}

static gboolean
DeleteMessage(void)
{
    HidePanel(WINDOW_MESSAGE);
#if defined(USE_GTKUIMANAGER)
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_ui_manager_get_widget(puim,
                                                                                 "/MainMenu/ViewMenu/PanelsMenu/Message")),
                                   FALSE);
#else
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget(pif,
                                                                                   "/View/Panels/Message")), FALSE);
#endif
    return TRUE;
}

static gboolean
DeleteAnalysis(void)
{
    HidePanel(WINDOW_ANALYSIS);
#if defined(USE_GTKUIMANAGER)
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_ui_manager_get_widget(puim,
                                                                                 "/MainMenu/ViewMenu/PanelsMenu/Analysis")),
                                   FALSE);
#else
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget(pif,
                                                                                   "/View/Panels/Analysis")), FALSE);
#endif
    return TRUE;
}

static gboolean
DeleteAnnotation(void)
{
    HidePanel(WINDOW_ANNOTATION);
#if defined(USE_GTKUIMANAGER)
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_ui_manager_get_widget(puim,
                                                                                 "/MainMenu/ViewMenu/PanelsMenu/Commentary")),
                                   FALSE);
#else
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget(pif,
                                                                                   "/View/Panels/Commentary")), FALSE);
#endif
    return TRUE;
}

static gboolean
DeleteGame(void)
{
    HidePanel(WINDOW_GAME);
#if defined(USE_GTKUIMANAGER)
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_ui_manager_get_widget(puim,
                                                                                 "/MainMenu/ViewMenu/PanelsMenu/GameRecord")),
                                   FALSE);
#else
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget(pif,
                                                                                   "/View/Panels/Game record")), FALSE);
#endif
    return TRUE;
}

static gboolean
DeleteTheoryWindow(void)
{
    HidePanel(WINDOW_THEORY);
#if defined(USE_GTKUIMANAGER)
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_ui_manager_get_widget(puim,
                                                                                 "/MainMenu/ViewMenu/PanelsMenu/Theory")),
                                   FALSE);
#else
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget(pif, "/View/Panels/Theory")), FALSE);
#endif
    return TRUE;
}

static gboolean
DeleteQuizWindow(void)
{
    HidePanel(WINDOW_QUIZ);
#if defined(USE_GTKUIMANAGER)
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_ui_manager_get_widget(puim,
                                                                                 "/MainMenu/ViewMenu/PanelsMenu/Quiz")),
                                   FALSE);
#else
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget(pif, "/View/Panels/Quiz")), FALSE);
#endif
    return TRUE;
}


static gboolean
DeleteCommandWindow(void)
{
    HidePanel(WINDOW_COMMAND);
#if defined(USE_GTKUIMANAGER)
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_ui_manager_get_widget(puim,
                                                                                 "/MainMenu/ViewMenu/PanelsMenu/Command")),
                                   FALSE);
#else
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget(pif,
                                                                                   "/View/Panels/Command")), FALSE);
#endif
    return TRUE;
}

static void
GetGeometryString(char *buf, windowobject * pwo)
{
    sprintf(buf, "set geometry %s width %d\n"
            "set geometry %s height %d\n"
            "set geometry %s xpos %d\n"
            "set geometry %s ypos %d\n"
            "set geometry %s max %s\n",
            pwo->winName, pwo->wg.nWidth,
            pwo->winName, pwo->wg.nHeight,
            pwo->winName, pwo->wg.nPosX, pwo->winName, pwo->wg.nPosY, pwo->winName, pwo->wg.max ? "yes" : "no");
}

extern void
SaveWindowSettings(FILE * pf)
{
    char szTemp[1024];
    int i;

    int saveShowingPanels, dummy;
    if (fFullScreen)
        GetFullscreenWindowSettings(&saveShowingPanels, &dummy, &woPanel[WINDOW_MAIN].wg.max);
    else
        saveShowingPanels = fDisplayPanels;

    fprintf(pf, "set annotation %s\n", woPanel[WINDOW_ANNOTATION].showing ? "yes" : "no");
    fprintf(pf, "set message %s\n", woPanel[WINDOW_MESSAGE].showing ? "yes" : "no");
    fprintf(pf, "set gamelist %s\n", woPanel[WINDOW_GAME].showing ? "yes" : "no");
    fprintf(pf, "set analysis window %s\n", woPanel[WINDOW_ANALYSIS].showing ? "yes" : "no");
    fprintf(pf, "set theorywindow %s\n", woPanel[WINDOW_THEORY].showing ? "yes" : "no");
    fprintf(pf, "set quizwindow %s\n", woPanel[WINDOW_QUIZ].showing ? "yes" : "no");
    fprintf(pf, "set commandwindow %s\n", woPanel[WINDOW_COMMAND].showing ? "yes" : "no");

    fprintf(pf, "set panels %s\n", saveShowingPanels ? "yes" : "no");

    for (i = 0; i < NUM_WINDOWS; i++) {
        if (i != WINDOW_ANNOTATION) {
            GetGeometryString(szTemp, &woPanel[i]);
            fputs(szTemp, pf);
        }
    }
    /* Save docked slider position */
    fprintf(pf, "set panelwidth %d\n", GetPanelSize());

    /* Save panel dock state (if not docked - default is docked) */
    if (!fDockPanels)
        fputs("set dockpanels off\n", pf);

    if (fFullScreen)
        woPanel[WINDOW_MAIN].wg.max = TRUE;
}

extern void
HidePanel(gnubgwindow window)
{
    if (gtk_widget_get_visible(woPanel[window].pwWin)) {
        getWindowGeometry(window);
        woPanel[window].showing = FALSE;
        gtk_widget_hide(woPanel[window].pwWin);
    }
}

extern void
getWindowGeometry(gnubgwindow window)
{
    windowobject *pwo = &woPanel[window];
    if (pwo->docked || !pwo->pwWin)
        return;


    if (gtk_widget_get_realized(pwo->pwWin)) {
        GdkWindowState state = gdk_window_get_state(gtk_widget_get_window(pwo->pwWin));
        pwo->wg.max = ((state & GDK_WINDOW_STATE_MAXIMIZED) == GDK_WINDOW_STATE_MAXIMIZED);
        if (pwo->wg.max)
            return;             /* Could restore window to get correct restore size - just use previous for now */

        gtk_window_get_position(GTK_WINDOW(pwo->pwWin), &pwo->wg.nPosX, &pwo->wg.nPosY);

        gtk_window_get_size(GTK_WINDOW(pwo->pwWin), &pwo->wg.nWidth, &pwo->wg.nHeight);
    }

}

void
DockPanels(void)
{
    int i;
    int currentSelectedGame = -1;
    if (!fX)
        return;

    if (game_select_combo)
        currentSelectedGame = gtk_combo_box_get_active(GTK_COMBO_BOX(game_select_combo));

    if (fDockPanels) {
        RefreshGeometries();    /* Get the current window positions */

#if defined(USE_GTKUIMANAGER)
        gtk_widget_show((gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/PanelsMenu/Commentary")));

        if (fDisplayPanels) {
            gtk_widget_show((gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/HidePanels")));
            gtk_widget_hide((gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/RestorePanels")));
        } else {
            gtk_widget_hide((gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/HidePanels")));
            gtk_widget_show((gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/RestorePanels")));
        }
#else
        gtk_widget_show(gtk_item_factory_get_widget(pif, "/View/Panels/Commentary"));
        if (fDisplayPanels) {
            gtk_widget_show(gtk_item_factory_get_widget(pif, "/View/Hide panels"));
            gtk_widget_hide(gtk_item_factory_get_widget(pif, "/View/Restore panels"));
        } else {
            gtk_widget_hide(gtk_item_factory_get_widget(pif, "/View/Hide panels"));
            gtk_widget_show(gtk_item_factory_get_widget(pif, "/View/Restore panels"));
        }
#endif

        for (i = 0; i < NUM_WINDOWS; i++) {
            if (woPanel[i].undockable && woPanel[i].pwWin)
                gtk_widget_destroy(woPanel[i].pwWin);

            if (woPanel[i].dockable)
                woPanel[i].docked = TRUE;
        }
        CreatePanels();
        
        if (fDisplayPanels)
            SwapBoardToPanel(TRUE, TRUE);
    } else {
        if (fDisplayPanels)
            SwapBoardToPanel(FALSE, TRUE);

#if defined(USE_GTKUIMANAGER)
        gtk_widget_hide((gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/PanelsMenu/Commentary")));
        gtk_widget_hide((gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/HidePanels")));
        gtk_widget_hide((gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/RestorePanels")));
#else
        gtk_widget_hide(gtk_item_factory_get_widget(pif, "/View/Panels/Commentary"));
        gtk_widget_hide(gtk_item_factory_get_widget(pif, "/View/Hide panels"));
        gtk_widget_hide(gtk_item_factory_get_widget(pif, "/View/Restore panels"));
#endif

        for (i = 0; i < NUM_WINDOWS; i++) {
            if (woPanel[i].dockable && woPanel[i].pwWin) {
                gtk_widget_destroy(woPanel[i].pwWin);
                woPanel[i].pwWin = NULL;
                woPanel[i].docked = FALSE;
            }
        }
        CreateGameWindow();
        CreateAnalysisWindow();
        CreateQuizWindow();
        CreateMessageWindow();
        CreateTheoryWindow();
        CreateCommandWindow();
    }
#if defined(USE_GTKUIMANAGER)
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(puim,
                                                       "/MainMenu/ViewMenu/PanelsMenu/Message"), !fDockPanels
                             || fDisplayPanels);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/PanelsMenu/GameRecord"), !fDockPanels
                             || fDisplayPanels);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/PanelsMenu/Commentary"), !fDockPanels
                             || fDisplayPanels);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/PanelsMenu/Analysis"), !fDockPanels
                             || fDisplayPanels);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/PanelsMenu/Command"), !fDockPanels
                             || fDisplayPanels);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/PanelsMenu/Quiz"), !fDockPanels
                             || fDisplayPanels);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/PanelsMenu/Theory"), !fDockPanels
                             || fDisplayPanels);
#else
    gtk_widget_set_sensitive(gtk_item_factory_get_widget(pif, "/View/Panels/Message"), !fDockPanels || fDisplayPanels);
    gtk_widget_set_sensitive(gtk_item_factory_get_widget(pif, "/View/Panels/Game record"), !fDockPanels
                             || fDisplayPanels);
    gtk_widget_set_sensitive(gtk_item_factory_get_widget(pif, "/View/Panels/Commentary"), !fDockPanels
                             || fDisplayPanels);
    gtk_widget_set_sensitive(gtk_item_factory_get_widget(pif, "/View/Panels/Analysis"), !fDockPanels || fDisplayPanels);
    gtk_widget_set_sensitive(gtk_item_factory_get_widget(pif, "/View/Panels/Command"), !fDockPanels || fDisplayPanels);
    gtk_widget_set_sensitive(gtk_item_factory_get_widget(pif, "/View/Panels/Quiz"), !fDockPanels || fDisplayPanels);
    gtk_widget_set_sensitive(gtk_item_factory_get_widget(pif, "/View/Panels/Theory"), !fDockPanels || fDisplayPanels);
#endif
    if (!fDockPanels || fDisplayPanels) {
        for (i = 0; i < NUM_WINDOWS; i++) {
            if (woPanel[i].dockable && woPanel[i].showing)
                woPanel[i].showFun();
        }
    }
    /* Refresh panel contents */
    GTKRegenerateGames();
    ChangeGame(NULL);
    if (currentSelectedGame != -1)
        GTKSetGame(currentSelectedGame);

    /* Make sure check item is correct */
#if defined(USE_GTKUIMANAGER)
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_ui_manager_get_widget(puim,
                                                                                 "/MainMenu/ViewMenu/DockPanels")),
                                   fDockPanels);
#else
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget(pif, "/View/Dock panels")),
                                   fDockPanels);
#endif
    /* Resize screen */
    SetMainWindowSize();
}

extern void
ShowAllPanels(gpointer UNUSED(p), guint UNUSED(n), GtkWidget * UNUSED(pw))
{
    BoardData *bd = BOARD(pwBoard)->board_data;
    int i;
    /* Only valid if panels docked */
    if (!fDockPanels)
        return;

    /* Hide for smoother appearance */
#if defined(USE_BOARD3D)
    if (display_is_3d(bd->rd))
        gtk_widget_hide(GetDrawingArea3d(bd->bd3d));
    else
#endif
        gtk_widget_hide(bd->drawing_area);

    fDisplayPanels = 1;

    for (i = 0; i < NUM_WINDOWS; i++) {
        if (woPanel[i].dockable && woPanel[i].showing)
            woPanel[i].showFun();
    }

#if defined(USE_GTKUIMANAGER)
    gtk_widget_show((gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/HidePanels")));
    gtk_widget_hide((gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/RestorePanels")));
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/PanelsMenu/Message"), TRUE);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/PanelsMenu/GameRecord"), TRUE);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/PanelsMenu/Commentary"), TRUE);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/PanelsMenu/Analysis"), TRUE);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/PanelsMenu/Command"), TRUE);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/PanelsMenu/Quiz"), TRUE);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/PanelsMenu/Theory"), TRUE);
#else
    gtk_widget_show(gtk_item_factory_get_widget(pif, "/View/Hide panels"));
    gtk_widget_hide(gtk_item_factory_get_widget(pif, "/View/Restore panels"));

    gtk_widget_set_sensitive(gtk_item_factory_get_widget(pif, "/View/Panels/Message"), TRUE);
    gtk_widget_set_sensitive(gtk_item_factory_get_widget(pif, "/View/Panels/Game record"), TRUE);
    gtk_widget_set_sensitive(gtk_item_factory_get_widget(pif, "/View/Panels/Commentary"), TRUE);
    gtk_widget_set_sensitive(gtk_item_factory_get_widget(pif, "/View/Panels/Analysis"), TRUE);
    gtk_widget_set_sensitive(gtk_item_factory_get_widget(pif, "/View/Panels/Command"), TRUE);
    gtk_widget_set_sensitive(gtk_item_factory_get_widget(pif, "/View/Panels/Quiz"), TRUE);
    gtk_widget_set_sensitive(gtk_item_factory_get_widget(pif, "/View/Panels/Theory"), TRUE);
#endif
    SwapBoardToPanel(TRUE, TRUE);

#if defined(USE_BOARD3D)
    if (display_is_3d(bd->rd))
        gtk_widget_show(GetDrawingArea3d(bd->bd3d));
    else
#endif
        gtk_widget_show(bd->drawing_area);
}

void
DoHideAllPanels(int updateEvents)
{
    BoardData *bd = BOARD(pwBoard)->board_data;
    int i;
    /* Only valid if panels docked */
    if (!fDockPanels)
        return;

    /* Hide for smoother appearance */
#if defined(USE_BOARD3D)
    if (display_is_3d(bd->rd))
        gtk_widget_hide(GetDrawingArea3d(bd->bd3d));
    else
#endif
        gtk_widget_hide(bd->drawing_area);

    fDisplayPanels = 0;

    for (i = 0; i < NUM_WINDOWS; i++) {
        if (woPanel[i].dockable && woPanel[i].showing) {
            woPanel[i].hideFun();
            woPanel[i].showing = TRUE;
        }
    }

#if defined(USE_GTKUIMANAGER)
    gtk_widget_show((gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/RestorePanels")));
    gtk_widget_set_sensitive((gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/RestorePanels")), TRUE);
    gtk_widget_hide((gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/HidePanels")));

    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/PanelsMenu/Message"), FALSE);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/PanelsMenu/GameRecord"), FALSE);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/PanelsMenu/Commentary"), FALSE);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/PanelsMenu/Analysis"), FALSE);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/PanelsMenu/Quiz"), FALSE);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/PanelsMenu/Theory"), FALSE);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(puim, "/MainMenu/ViewMenu/PanelsMenu/Command"), FALSE);
#else
    gtk_widget_show(gtk_item_factory_get_widget(pif, "/View/Restore panels"));
    gtk_widget_set_sensitive(gtk_item_factory_get_widget(pif, "/View/Restore panels"), TRUE);
    gtk_widget_hide(gtk_item_factory_get_widget(pif, "/View/Hide panels"));

    gtk_widget_set_sensitive(gtk_item_factory_get_widget(pif, "/View/Panels/Message"), FALSE);
    gtk_widget_set_sensitive(gtk_item_factory_get_widget(pif, "/View/Panels/Game record"), FALSE);
    gtk_widget_set_sensitive(gtk_item_factory_get_widget(pif, "/View/Panels/Commentary"), FALSE);
    gtk_widget_set_sensitive(gtk_item_factory_get_widget(pif, "/View/Panels/Analysis"), FALSE);
    gtk_widget_set_sensitive(gtk_item_factory_get_widget(pif, "/View/Panels/Quiz"), FALSE);
    gtk_widget_set_sensitive(gtk_item_factory_get_widget(pif, "/View/Panels/Theory"), FALSE);
    gtk_widget_set_sensitive(gtk_item_factory_get_widget(pif, "/View/Panels/Command"), FALSE);
#endif
    SwapBoardToPanel(FALSE, updateEvents);

    /* Resize screen */
    SetMainWindowSize();

#if defined(USE_BOARD3D)
    if (display_is_3d(bd->rd))
        gtk_widget_show(GetDrawingArea3d(bd->bd3d));
    else
#endif
        gtk_widget_show(bd->drawing_area);
}

extern void
HideAllPanels(gpointer UNUSED(p), guint UNUSED(n), GtkWidget * UNUSED(pw))
{
    DoHideAllPanels(TRUE);
}

#if defined(USE_GTKUIMANAGER)
void
ToggleDockPanels(GtkToggleAction * action, gpointer UNUSED(user_data))
{
    int newValue = gtk_toggle_action_get_active(action);
    if (fDockPanels != newValue) {
        fDockPanels = newValue;
        DockPanels();
    }
}
#else
extern void
ToggleDockPanels(gpointer UNUSED(p), guint UNUSED(n), GtkWidget * pw)
{
    int newValue = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(pw));
    if (fDockPanels != newValue) {
        fDockPanels = newValue;
        DockPanels();
    }
}
#endif

extern void
DisplayWindows(void)
{
    int i;
    /* Display any other windows now */
    for (i = 0; i < NUM_WINDOWS; i++) {
        if (woPanel[i].pwWin && woPanel[i].dockable) {
            if (woPanel[i].showing)
                gtk_widget_show_all(woPanel[i].pwWin);
            else
                gtk_widget_hide(woPanel[i].pwWin);
        }
    }
    if (!fDisplayPanels) {
        gtk_widget_hide(hpaned);        /* Need to stop wrong panel position getting set - gtk 2.6 */
        HideAllPanels(0, 0, 0);
    }
}

void
DestroyPanel(gnubgwindow window)
{
    if (woPanel[window].pwWin) {
        gtk_widget_destroy(woPanel[window].pwWin);
        woPanel[window].pwWin = NULL;
        woPanel[window].showing = FALSE;
    }
}

GtkWidget *
GetPanelWidget(gnubgwindow window)
{
    return woPanel[window].pwWin;
}

void
SetPanelWidget(gnubgwindow window, GtkWidget * pWin)
{
    woPanel[window].pwWin = pWin;
    woPanel[WINDOW_HINT].showing = pWin ? TRUE : FALSE;
}

extern void
setWindowGeometry(gnubgwindow window)
{
    windowobject *pwo = &woPanel[window];

    if (pwo->docked || !pwo->pwWin || !fGUISetWindowPos)
        return;


    gtk_window_set_default_size(GTK_WINDOW(pwo->pwWin),
                                (pwo->wg.nWidth > 0) ? pwo->wg.nWidth : -1,
                                (pwo->wg.nHeight > 0) ? pwo->wg.nHeight : -1);

    gtk_window_move(GTK_WINDOW(pwo->pwWin),
                    (pwo->wg.nPosX >= 0) ? pwo->wg.nPosX : 0, (pwo->wg.nPosY >= 0) ? pwo->wg.nPosY : 0);

    if (pwo->wg.max)
        gtk_window_maximize(GTK_WINDOW(pwo->pwWin));
    else
        gtk_window_unmaximize(GTK_WINDOW(pwo->pwWin));
}

void
ShowHidePanel(gnubgwindow panel)
{
    if (woPanel[panel].showing)
        woPanel[panel].showFun();
    else
        woPanel[panel].hideFun();
}

int
SetMainWindowSize(void)
{
    if (woPanel[WINDOW_MAIN].wg.nWidth && woPanel[WINDOW_MAIN].wg.nHeight) {
        gtk_window_set_default_size(GTK_WINDOW(pwMain),
                                    woPanel[WINDOW_MAIN].wg.nWidth, woPanel[WINDOW_MAIN].wg.nHeight);
        return 1;
    } else
        return 0;
}

void
PanelShow(gnubgwindow panel)
{
    woPanel[panel].showFun();
}

void
PanelHide(gnubgwindow panel)
{
    woPanel[panel].hideFun();
}

extern void
RefreshGeometries(void)
{
    int i;
    for (i = 0; i < NUM_WINDOWS; i++)
        getWindowGeometry((gnubgwindow) i);
}

extern void
CommandSetAnnotation(char *sz)
{

    SetToggle("annotation", &woPanel[WINDOW_ANNOTATION].showing, sz,
              _("Move analysis and commentary will be displayed."),
              _("Move analysis and commentary will not be displayed."));
}

extern void
CommandSetMessage(char *sz)
{

    SetToggle("message", &woPanel[WINDOW_MESSAGE].showing, sz,
              _("Show window with messages."), _("Do not show window with messages."));
}

extern void
CommandSetTheoryWindow(char *sz)
{

    SetToggle("theorywindow", &woPanel[WINDOW_THEORY].showing, sz,
              _("Show window with theory."), _("Do not show window with theory."));
}

extern void
CommandSetQuizWindow(char *sz)
{

    SetToggle("quizwindow", &woPanel[WINDOW_QUIZ].showing, sz,
              _("Show window with quiz categories."), _("Do not show window with quiz categories."));
}

extern void
CommandSetCommandWindow(char *sz)
{

    SetToggle("commandwindow", &woPanel[WINDOW_COMMAND].showing, sz,
              _("Show window to enter commands."), _("Do not show window to enter commands."));
}

extern void
CommandSetGameList(char *sz)
{

    SetToggle("gamelist", &woPanel[WINDOW_GAME].showing, sz,
              _("Show game window with moves."), _("Do not show game window with moves."));
}

extern void
CommandSetAnalysisWindows(char *sz)
{

    SetToggle("analysis window", &woPanel[WINDOW_ANALYSIS].showing, sz,
              _("Show window with analysis."), _("Do not show window with analysis."));
}

static gnubgwindow pwoSetPanel;

extern void
CommandSetGeometryAnalysis(char *sz)
{
    pwoSetPanel = WINDOW_ANALYSIS;
    HandleCommand(sz, acSetGeometryValues);
}

extern void
CommandSetGeometryHint(char *sz)
{
    pwoSetPanel = WINDOW_HINT;
    HandleCommand(sz, acSetGeometryValues);
}

extern void
CommandSetGeometryGame(char *sz)
{
    pwoSetPanel = WINDOW_GAME;
    HandleCommand(sz, acSetGeometryValues);
}

extern void
CommandSetGeometryMain(char *sz)
{
    pwoSetPanel = WINDOW_MAIN;
    HandleCommand(sz, acSetGeometryValues);
}

extern void
CommandSetGeometryMessage(char *sz)
{
    pwoSetPanel = WINDOW_MESSAGE;
    HandleCommand(sz, acSetGeometryValues);
}

extern void
CommandSetGeometryCommand(char *sz)
{
    pwoSetPanel = WINDOW_COMMAND;
    HandleCommand(sz, acSetGeometryValues);
}

extern void
CommandSetGeometryTheory(char *sz)
{
    pwoSetPanel = WINDOW_THEORY;
    HandleCommand(sz, acSetGeometryValues);
}

extern void
CommandSetGeometryQuiz(char *sz)
{
    pwoSetPanel = WINDOW_QUIZ;
    HandleCommand(sz, acSetGeometryValues);
}

extern void
CommandSetGeometryWidth(char *sz)
{

    int n;

    if ((n = ParseNumber(&sz)) == INT_MIN)
        outputf(_("Illegal value. " "See 'help set geometry %s width'.\n"), woPanel[pwoSetPanel].winName);
    else {

        woPanel[pwoSetPanel].wg.nWidth = n;
        outputf(_("Width of %s window set to %d.\n"), woPanel[pwoSetPanel].winName, n);

        if (fX)
            setWindowGeometry(pwoSetPanel);

    }

}

extern void
CommandSetGeometryHeight(char *sz)
{

    int n;

    if ((n = ParseNumber(&sz)) == INT_MIN)
        outputf(_("Illegal value. " "See 'help set geometry %s height'.\n"), woPanel[pwoSetPanel].winName);
    else {

        woPanel[pwoSetPanel].wg.nHeight = n;
        outputf(_("Height of %s window set to %d.\n"), woPanel[pwoSetPanel].winName, n);

        if (fX)
            setWindowGeometry(pwoSetPanel);
    }

}

extern void
CommandSetGeometryPosX(char *sz)
{

    int n;

    if ((n = ParseNumber(&sz)) == INT_MIN)
        outputf(_("Illegal value. " "See 'help set geometry %s xpos'.\n"), woPanel[pwoSetPanel].winName);
    else {

        woPanel[pwoSetPanel].wg.nPosX = n;
        outputf(_("X-position of %s window set to %d.\n"), woPanel[pwoSetPanel].winName, n);

        if (fX)
            setWindowGeometry(pwoSetPanel);

    }

}

extern void
CommandSetGeometryPosY(char *sz)
{

    int n;

    if ((n = ParseNumber(&sz)) == INT_MIN)
        outputf(_("Illegal value. " "See 'help set geometry %s ypos'.\n"), woPanel[pwoSetPanel].winName);
    else {

        woPanel[pwoSetPanel].wg.nPosY = n;
        outputf(_("Y-position of %s window set to %d.\n"), woPanel[pwoSetPanel].winName, n);

        if (fX)
            setWindowGeometry(pwoSetPanel);
    }

}

extern void
CommandSetGeometryMax(char *sz)
{
    int maxed = (StrCaseCmp(sz, "yes") == 0);
    woPanel[pwoSetPanel].wg.max = maxed;
    outputf(maxed ? _("%s window maximised.\n") : _("%s window unmaximised.\n"), woPanel[pwoSetPanel].winName);

    if (fX)
        setWindowGeometry(pwoSetPanel);
}

extern void
CommandSetPanels(char *sz)
{

    SetToggle("panels", &fDisplayPanels, sz,
              _("Game list, Annotation and Message panels/windows will be displayed."),
              _("Game list, Annotation and Message panels/windows will not be displayed.")
        );

    if (fX) {
        if (fDisplayPanels)
            ShowAllPanels(0, 0, 0);
        else
            HideAllPanels(0, 0, 0);
    }

}

extern void
CommandShowPanels(char *UNUSED(sz))
{
    if (fDisplayPanels)
        outputl(_("Game list, Annotation and Message panels/windows " "will be displayed."));
    else
        outputl(_("Game list, Annotation and Message panels/windows " "will not be displayed."));
}

extern void
CommandSetDockPanels(char *sz)
{

    SetToggle("dockdisplay", &fDockPanels, sz, _("Windows will be docked."), _("Windows will be detached."));
    DockPanels();
}

static void
GetGeometryDisplayString(char *buf, windowobject * pwo)
{
    char dispName[50];
    sprintf(dispName, "%c%s %s", toupper(pwo->winName[0]), &pwo->winName[1], _("window"));

    sprintf(buf, "%-17s : size %dx%d, position (%d,%d)\n",
            dispName, pwo->wg.nWidth, pwo->wg.nHeight, pwo->wg.nPosX, pwo->wg.nPosY);
}

extern void
CommandShowGeometry(char *UNUSED(sz))
{
    int i;
    char szBuf[1024];
    output(_("Default geometries:\n\n"));

    for (i = 0; i < NUM_WINDOWS; i++) {
        GetGeometryDisplayString(szBuf, &woPanel[i]);
        output(szBuf);
    }
}

int
DockedPanelsShowing(void)
{
    return fDockPanels && fDisplayPanels;
}

int
ArePanelsShowing(void)
{
    return fDisplayPanels;
}

int
ArePanelsDocked(void)
{
    return fDockPanels;
}

int
IsPanelDocked(gnubgwindow window)
{
    return woPanel[window].docked;
}

#if ! GTK_CHECK_VERSION(3,0,0)
int
GetPanelWidth(gnubgwindow panel)
{
    return woPanel[panel].wg.nWidth;
}
#endif

int
IsPanelShowVar(gnubgwindow panel, void * const p)
{
    return (p == &woPanel[panel].showing);
}

int
PanelShowing(gnubgwindow window)
{
    return woPanel[window].showing && (fDisplayPanels || !woPanel[window].docked);
}

extern void
ClosePanels(void)
{
    /* loop through and close each panel (not main window) */
    int window;
    for (window = 0; window < NUM_WINDOWS; window++) {
        if (window != WINDOW_MAIN) {
            if (woPanel[window].pwWin) {
                gtk_widget_destroy(woPanel[window].pwWin);
                woPanel[window].pwWin = NULL;
            }
        }
    }
    pwTheoryView = NULL;        /* Reset this to stop errors - may be other ones that need reseting? */
}
