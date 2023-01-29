/*
 * Copyright (C) 2005-2009 Jon Kinsey <jonkinsey@gmail.com>
 * Copyright (C) 2006-2018 the AUTHORS
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
 * $Id: gtkmovelist.c,v 1.44 2022/12/15 22:23:00 plm Exp $
 */

#include "config.h"
#include "gtklocdefs.h"
#include "gtkgame.h"
#include <string.h>

#include "format.h"
#include "gtkmovelistctrl.h"
#include "drawboard.h"
#include "multithread.h"

#define DETAIL_COLUMN_COUNT 11
#define MIN_COLUMN_COUNT 5

enum {
    ML_COL_RANK = 0,
    ML_COL_TYPE,
    ML_COL_WIN,
    ML_COL_GWIN,
    ML_COL_BGWIN,
    ML_COL_LOSS,
    ML_COL_GLOSS,
    ML_COL_BGLOSS,
    ML_COL_EQUITY,
    ML_COL_DIFF,
    ML_COL_MOVE,
    ML_COL_FGCOL,
    ML_COL_DATA
};

static GtkListStore * globalStore;
static GtkTreeIter * globalpIter;
static int globalSkipoldcode;

extern void
MoveListCreate(hintdata * phd)
{
    static const char *aszTitleDetails[] = {
        N_("Rank"),
        N_("analysisType|Type"),
        N_("Win"),
        N_("W g"),
        N_("W bg"),
        N_("Lose"),
        N_("L g"),
        N_("L bg"),
        NULL,
        N_("Diff."),
        N_("noun|Move")
    };
    unsigned int i;
    int showWLTree = showMoveListDetail && !phd->fDetails;

    /* Create list widget */
    GtkListStore *store;
    GtkTreeIter iter;
    GtkTreeSelection *sel;
    GtkWidget *view = gtk_tree_view_new();
    int offset = (phd->fDetails) ? 0 : MIN_COLUMN_COUNT - DETAIL_COLUMN_COUNT;

    if (showWLTree) {
        GtkStyle *psDefault = gtk_widget_get_style(view);

        GtkCellRenderer *renderer = custom_cell_renderer_movelist_new();
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, _(aszTitleDetails[ML_COL_RANK]), renderer,
                                                    "movelist", 0, "rank", 1, NULL);
        gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);
        g_object_set(renderer, "cell-background-gdk", &psDefault->bg[GTK_STATE_NORMAL],
                     "cell-background-set", TRUE, NULL);

        g_object_set_data(G_OBJECT(view), "hintdata", phd);
    } else {
        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        g_object_set(renderer, "ypad", 0, NULL);

        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, _(aszTitleDetails[ML_COL_RANK]), renderer,
                                                    "text", ML_COL_RANK, "foreground", ML_COL_FGCOL + offset, NULL);
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, Q_(aszTitleDetails[ML_COL_TYPE]), renderer,
                                                    "text", ML_COL_TYPE, "foreground", ML_COL_FGCOL + offset, NULL);

        if (phd->fDetails) {
            gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, _(aszTitleDetails[ML_COL_WIN]),
                                                        renderer, "text", ML_COL_WIN, "foreground",
                                                        ML_COL_FGCOL + offset, NULL);
            gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, _(aszTitleDetails[ML_COL_GWIN]),
                                                        renderer, "text", ML_COL_GWIN, "foreground",
                                                        ML_COL_FGCOL + offset, NULL);
            gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, _(aszTitleDetails[ML_COL_BGWIN]),
                                                        renderer, "text", ML_COL_BGWIN, "foreground",
                                                        ML_COL_FGCOL + offset, NULL);
            gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, _(aszTitleDetails[ML_COL_LOSS]),
                                                        renderer, "text", ML_COL_LOSS, "foreground",
                                                        ML_COL_FGCOL + offset, NULL);
            gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, _(aszTitleDetails[ML_COL_GLOSS]),
                                                        renderer, "text", ML_COL_GLOSS, "foreground",
                                                        ML_COL_FGCOL + offset, NULL);
            gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, _(aszTitleDetails[ML_COL_BGLOSS]),
                                                        renderer, "text", ML_COL_BGLOSS, "foreground",
                                                        ML_COL_FGCOL + offset, NULL);
        }

        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, aszTitleDetails[ML_COL_EQUITY],
                                                    renderer, "text", ML_COL_EQUITY + offset, "foreground",
                                                    ML_COL_FGCOL + offset, NULL);
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, _(aszTitleDetails[ML_COL_DIFF]), renderer,
                                                    "text", ML_COL_DIFF + offset, "foreground", ML_COL_FGCOL + offset,
                                                    NULL);
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, Q_(aszTitleDetails[ML_COL_MOVE]), renderer,
                                                    "text", ML_COL_MOVE + offset, "foreground", ML_COL_FGCOL + offset,
                                                    NULL);
    }

    phd->pwMoves = view;

    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
    gtk_tree_selection_set_mode(sel, GTK_SELECTION_MULTIPLE);

    g_signal_connect(view, "row-activated", G_CALLBACK(HintDoubleClick), phd);
    g_signal_connect(sel, "changed", G_CALLBACK(HintSelect), phd);


    /* Add empty rows */
    if (phd->fDetails)
        store =
            gtk_list_store_new(DETAIL_COLUMN_COUNT + 2, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                               G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                               G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
    else {
        if (showWLTree)
            store = gtk_list_store_new(2, G_TYPE_POINTER, G_TYPE_INT);
        else
            store =
                gtk_list_store_new(MIN_COLUMN_COUNT + 2, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
    }

    for (i = 0; i < phd->pml->cMoves; i++)
        gtk_list_store_append(store, &iter);

    gtk_tree_view_set_model(GTK_TREE_VIEW(view), GTK_TREE_MODEL(store));
    //if(!fAnalysisRunning && !fLayeredAnalysis)

        /////// This didn't work, cache.c doesn't know what a gtkwidget is...
        // AnalyseMoveTask *pt = NULL;
        // pt = (AnalyseMoveTask *) malloc(sizeof(AnalyseMoveTask));
        // pt->task.fun = (AsyncFun) MoveListUpdateMT;
        // pt->task.data = pt;
        // pt->task.pLinkedTask = NULL;
        // pt->pmr = NULL;
        // pt->plGame = NULL;
        // pt->psc = NULL;
        // pt->phd=phd;
        // MT_AddTask((Task *) pt, TRUE);

    MoveListUpdate(phd);
}

float rBest;

GtkStyle *psHighlight = NULL;

extern void
MoveListRefreshSize(void)
{
    custom_cell_renderer_invalidate_size();
    if (pwMoveAnalysis) {
        hintdata *phd = (hintdata *) g_object_get_data(G_OBJECT(pwMoveAnalysis), "user_data");
        MoveListUpdate(phd);
    }
}

void MoveListUpdateTaskMT (Task * task) {

    AnalyseMoveTask *amt;
    amt = (AnalyseMoveTask *) task;
    int taskType = amt->taskType;
    // char * sz;
    // sz=amt->sz;
    int skipoldcode=0;
    globalSkipoldcode=0;
    int j,colNum;
        
    g_message("INSIDE MoveListUpdateTaskMT, taskType=%d",amt->taskType);
    // g_message("INSIDE GetRScore: amt->pml->amMoves[0].rScore=%f",amt->pml->amMoves[0].rScore);
    
    if (taskType==1)
        rBest= amt->pml->amMoves[0].rScore;
        
    else if(taskType==2) {
        if (amt->rankKnown) {
            sprintf(amt->sz, "%s%s%u", amt->pml->amMoves[amt->i].cmark ? "+" : "", amt->highlight_sz, amt->i + 1); 
            g_message("INSIDE MoveListUpdateTaskMT, taskType=%d: sz=%s",amt->taskType,amt->sz);
        }
        else
            sprintf(amt->sz, "%s%s??", amt->pml->amMoves[amt->i].cmark ? "+" : "", amt->highlight_sz);

        if (amt->showWLTree) {
            gtk_list_store_set(globalStore, globalpIter, 1, amt->rankKnown ? (int) amt->i + 1 : -1, -1);
            skipoldcode=1; 
            globalSkipoldcode=1;//goto skipoldcode;
        } else
            gtk_list_store_set(globalStore, globalpIter, ML_COL_RANK, amt->sz, -1);    

        FormatEval(amt->sz, &amt->pml->amMoves[amt->i].esMove);  

        if(!skipoldcode) {
            gtk_list_store_set(globalStore, globalpIter, ML_COL_TYPE, amt->sz, -1);  //<--

            /* gwc */
            if (amt->fDetails) {
                colNum = ML_COL_WIN;
                for (j = 0; j < 5; j++) {
                    if (j == 3) {
                        gtk_list_store_set(globalStore, globalpIter, colNum, OutputPercent(1.0f - amt->ar[OUTPUT_WIN]), -1);
                        colNum++;
                    }
                    gtk_list_store_set(globalStore, globalpIter, colNum, OutputPercent(amt->ar[j]), -1);
                    colNum++;
                }
            }

            /* cubeless equity */
            gtk_list_store_set(globalStore, globalpIter, ML_COL_EQUITY + amt->offset, OutputEquity(amt->pml->amMoves[amt->i].rScore, amt->pci, TRUE), -1);
            if (amt->i != 0) {
                gtk_list_store_set(globalStore, globalpIter, ML_COL_DIFF + amt->offset,
                                OutputEquityDiff(amt->pml->amMoves[amt->i].rScore, rBest, amt->pci), -1);
            }

            gtk_list_store_set(globalStore, globalpIter, ML_COL_MOVE + amt->offset, FormatMove(amt->sz, msBoard(), amt->pml->amMoves[amt->i].anMove), -1);

            // /* highlight row */
            // if (phd->piHighlight && *phd->piHighlight == i) {
            //     char buf[20];
            //     sprintf(buf, "#%02x%02x%02x", psHighlight->fg[GTK_STATE_SELECTED].red / 256,
            //             psHighlight->fg[GTK_STATE_SELECTED].green / 256, psHighlight->fg[GTK_STATE_SELECTED].blue / 256);
            //     gtk_list_store_set(globalStore, globalpIter, ML_COL_FGCOL + amt->offset, buf, -1);
            // } else
            //     gtk_list_store_set(globalStore, globalpIter, ML_COL_FGCOL + amt->offset, NULL, -1);
        }
    //   //skipoldcode:             /* Messy as 3 copies of code at moment... */
    //     gtk_tree_model_iter_next(GTK_TREE_MODEL(store), globalpIter);   
    } else 
        g_message("ERROR: taskType");

}

// extern void
// PrintMove(char * message, moverecord *pmr)
// {
//           char tmp[200];
//        TanBoard anBoard;

//         FormatMove(tmp, (ConstTanBoard)anBoard, pmr->n.anMove);
//         g_message("%s: move=%s\n", message, tmp);
// }

// static void
// MoveListUpdateMT(Task * task)
// {
//     AnalyseMoveTask *amt;
//     amt = (AnalyseMoveTask *) task;
//     MoveListUpdate(amt->phd);
// }


/*
 * Call MoveListUpdate to update the movelist in the GTK hint window.
 * For example, after new evaluations, rollouts or toggle of MWC/Equity.
 *
 */
extern void
MoveListUpdate(const hintdata * phd)
{
    unsigned int i, j, colNum;
    //char sz[32];
    char * sz; /*<--changing for layered analysis*/
    sz = (char *)malloc(sizeof(char) * (33));
    cubeinfo ci;
    movelist *pml = phd->pml;
    int col = phd->fDetails ? 8 : 2;
    int showWLTree = showMoveListDetail && !phd->fDetails;

    int offset = (phd->fDetails) ? 0 : MIN_COLUMN_COUNT - DETAIL_COLUMN_COUNT;
    GtkTreeIter iter;
    GtkListStore *store;
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(phd->pwMoves)));
    gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);

    if (!psHighlight) {         /* Get highlight style first time in */
        GtkStyle *psTemp;
        GtkStyle *psMoves = gtk_widget_get_style(phd->pwMoves);
        GetStyleFromRCFile(&psHighlight, "move-done", psMoves);
        /* Use correct background colour when selected */
        memcpy(&psHighlight->bg[GTK_STATE_SELECTED], &psMoves->bg[GTK_STATE_SELECTED], sizeof(GdkColor));

        /* Also get colour to use for w/l stats in detail view */
        GetStyleFromRCFile(&psTemp, "move-winlossfg", psMoves);
        memcpy(&wlCol, &psTemp->fg[GTK_STATE_NORMAL], sizeof(GdkColor));
        g_object_unref(psTemp);
    }

    /* This function should only be called when the game state matches
     * the move list. */
    g_assert(ms.fMove == 0 || ms.fMove == 1);

    GetMatchStateCubeInfo(&ci, &ms);
        // if(!fAnalysisRunning && !fLayeredAnalysis) {
    // MT_Exclusive();
    // g_message("MoveListUpdate: phd->pml->amMoves[0].rScore=%f",phd->pml->amMoves[0].rScore);
        
    // rBest = pml->amMoves[0].rScore; //<================================================================== MYBUG

        /* creating a multi-thread version for the following, as it keeps causing crashes.
        Based on AnalyzeGame(), which does the same.
        Then doing the same for the lines with "<----", which are the ones causing trouble 
        (everything is heuristic here)
        */    
        AnalyseMoveTask *pt = NULL;
        pt = (AnalyseMoveTask *) malloc(sizeof(AnalyseMoveTask));
        pt->task.fun = (AsyncFun) MoveListUpdateTaskMT;
        pt->task.data = pt;
        pt->task.pLinkedTask = NULL;
        pt->pmr = NULL;
        pt->plGame = NULL;
        pt->psc = NULL;
        pt->pml=pml;
        pt->taskType=1;
        MT_AddTask((Task *) pt, TRUE);

    //   GetRScore(pml);
    // MT_Release();

    if (!showWLTree)
        gtk_tree_view_column_set_title(gtk_tree_view_get_column(GTK_TREE_VIEW(phd->pwMoves), col),
                                       (fOutputMWC && ms.nMatchTo) ? _("MWC") : _("Equity"));

    for (i = 0; i < pml->cMoves; i++) {
        float *ar = pml->amMoves[i].arEvalMove;
        int rankKnown;
        const char *highlight_sz;

        if (showWLTree)
            gtk_list_store_set(store, &iter, 0, pml->amMoves + i, -1);
        else
            gtk_list_store_set(store, &iter, ML_COL_DATA + offset, pml->amMoves + i, -1);

        rankKnown = 1;
        if (i && i == pml->cMoves - 1 && phd->piHighlight && i == *phd->piHighlight)
            /* The move made is the last on the list.  Some moves might
             * have been deleted to fit this one in */
        {
            /* Lets count how many moves are possible to see if this is the last move */
            movelist ml;
            int dice[2];
            memcpy(dice, ms.anDice, sizeof(dice));
            if (!dice[0]) {     /* If the dice have got lost, try to find them */
                moverecord *pmr = (moverecord *) plLastMove->plNext->p;
                if (pmr) {
                    dice[0] = pmr->anDice[0];
                    dice[1] = pmr->anDice[1];
                }
            }
            GenerateMoves(&ml, msBoard(), dice[0], dice[1], FALSE);
            if (i < ml.cMoves - 1)
                rankKnown = 0;
        }

        highlight_sz = (phd->piHighlight && *phd->piHighlight == i) ? "*" : "";

        /*layered*/
        globalStore = store;
        globalpIter=&iter;
        globalSkipoldcode=0;

        AnalyseMoveTask *pt = NULL;
        pt = (AnalyseMoveTask *) malloc(sizeof(AnalyseMoveTask));
        pt->task.fun = (AsyncFun) MoveListUpdateTaskMT;
        pt->task.data = pt;
        pt->task.pLinkedTask = NULL;
        pt->pmr = NULL;
        pt->plGame = NULL;
        pt->psc = NULL;
        pt->pml=pml;
        pt->taskType=2;
        pt->highlight_sz=highlight_sz;
        pt->sz=sz;
        pt->i=i;
        pt->rankKnown=rankKnown;
        pt->showWLTree=showWLTree;
        pt->fDetails=phd->fDetails;
        pt->ar=ar;
        pt->offset=offset;
        pt->pci=&ci;

        MT_AddTask((Task *) pt, TRUE);

        // if (rankKnown)
        //     sprintf(sz, "%s%s%u", pml->amMoves[i].cmark ? "+" : "", highlight_sz, i + 1); //<--
        // else
        //     sprintf(sz, "%s%s??", pml->amMoves[i].cmark ? "+" : "", highlight_sz);

          //  g_message("OUTSIDE MoveListUpdateTaskMT,  sz=%s",sz);

        // if (showWLTree) {
        //     gtk_list_store_set(store, &iter, 1, rankKnown ? (int) i + 1 : -1, -1);
        //     goto skipoldcode;
        // } 
        // else {
        //     gtk_list_store_set(store, &iter, ML_COL_RANK, sz, -1);  //<--  
        //         /* From documentation: gtk_list_store_set: The variable argument list should contain integer 
        //         column numbers, each column number followed by the value to be set. The list is terminated by 
        //         a -1.
        //         For example, to set column 0 with type G_TYPE_STRING to “Foo”, you would write 
        //         gtk_list_store_set (store, iter, 0, "Foo", -1).
        //         */
        // }
        // FormatEval(sz, &pml->amMoves[i].esMove);    //<--
        if(!globalSkipoldcode) {
    //         gtk_list_store_set(store, &iter, ML_COL_TYPE, sz, -1);  //<--

    //         /* gwc */
    //         if (phd->fDetails) {
    //             colNum = ML_COL_WIN;
    //             for (j = 0; j < 5; j++) {
    //                 if (j == 3) {
    //                     gtk_list_store_set(store, &iter, colNum, OutputPercent(1.0f - ar[OUTPUT_WIN]), -1);
    //                     colNum++;
    //                 }
    //                 gtk_list_store_set(store, &iter, colNum, OutputPercent(ar[j]), -1);
    //                 colNum++;
    //             }
    //         }

    //         /* cubeless equity */
    //         gtk_list_store_set(store, &iter, ML_COL_EQUITY + offset, OutputEquity(pml->amMoves[i].rScore, &ci, TRUE), -1);
    //         if (i != 0) {
    //             gtk_list_store_set(store, &iter, ML_COL_DIFF + offset,
    //                             OutputEquityDiff(pml->amMoves[i].rScore, rBest, &ci), -1);
    //         }

    //         gtk_list_store_set(store, &iter, ML_COL_MOVE + offset, FormatMove(sz, msBoard(), pml->amMoves[i].anMove), -1);

            /* highlight row */
            if (phd->piHighlight && *phd->piHighlight == i) {
                char buf[20];
                sprintf(buf, "#%02x%02x%02x", psHighlight->fg[GTK_STATE_SELECTED].red / 256,
                        psHighlight->fg[GTK_STATE_SELECTED].green / 256, psHighlight->fg[GTK_STATE_SELECTED].blue / 256);
                gtk_list_store_set(store, &iter, ML_COL_FGCOL + offset, buf, -1);
            } else
                gtk_list_store_set(store, &iter, ML_COL_FGCOL + offset, NULL, -1);
        }
      //skipoldcode:             /* Messy as 3 copies of code at moment... */
        gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
    }

    /* layered: remove if unused*/
    free(sz);
}

extern GList *
MoveListGetSelectionList(const hintdata * phd)
{
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(phd->pwMoves));
    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(phd->pwMoves));
    return gtk_tree_selection_get_selected_rows(sel, &model);
}

/* gtk_tree_path_free() is not the right function type
 * to be called directly from g_list_foreach() below
 */
static inline void
my_gtk_tree_path_free(gpointer data, gpointer UNUSED(user_data))
{
    gtk_tree_path_free((GtkTreePath *) data);
}

extern void
MoveListFreeSelectionList(GList * pl)
{
    g_list_foreach(pl, my_gtk_tree_path_free, NULL);
    g_list_free(pl);
}

extern move *
MoveListGetMove(const hintdata * phd, GList * pl)
{
    move *m;
    int showWLTree = showMoveListDetail && !phd->fDetails;
    int col, offset = (phd->fDetails) ? 0 : MIN_COLUMN_COUNT - DETAIL_COLUMN_COUNT;
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(phd->pwMoves));

    gboolean check = gtk_tree_model_get_iter(model, &iter, (GtkTreePath *) (pl->data));

    g_assert(check == TRUE);
#if defined(G_DISABLE_ASSERT)
    (void)check;      /* silence warning about unused variable */
#endif

    if (showWLTree)
        col = 0;
    else
        col = ML_COL_DATA + offset;

    gtk_tree_model_get(model, &iter, col, &m, -1);

    return m;
}

extern void
MoveListShowToggledClicked(GtkWidget * UNUSED(pw), hintdata * phd)
{
    int f = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(phd->pwShow));
    if (f)
        gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(phd->pwMoves)), GTK_SELECTION_SINGLE);
    else
        gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(phd->pwMoves)), GTK_SELECTION_MULTIPLE);

    ShowMove(phd, f);
}

extern gint
MoveListClearSelection(GtkWidget * UNUSED(pw), GdkEventSelection * UNUSED(pes), hintdata * phd)
{
    gtk_tree_selection_unselect_all(gtk_tree_view_get_selection(GTK_TREE_VIEW(phd->pwMoves)));
    return TRUE;
}
