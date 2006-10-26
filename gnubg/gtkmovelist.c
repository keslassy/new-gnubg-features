/*
 * gtkmovelist.c
 * by Jon Kinsey, 2005
 *
 * Analysis move list
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 */

#include <config.h>

#include <gtk/gtk.h>
#include <string.h>
#include "eval.h"
#include "gtkchequer.h"
#include "backgammon.h"
#include <glib/gi18n.h>
#include "format.h"
#include "assert.h"
#include "gtkmovelistctrl.h"
#include "gtkgame.h"
#include "drawboard.h"

extern void HintDoubleClick(GtkTreeView        *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       hintdata *phd);
extern void HintSelect(GtkTreeSelection *selection, hintdata *phd);

extern GdkColor wlCol;
void ShowMove ( hintdata *phd, const int f );

#define DETAIL_COLUMN_COUNT 11
#define MIN_COLUMN_COUNT 5

enum
{
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
} ;

void MoveListUpdate ( const hintdata *phd );

void MoveListCreate(hintdata *phd)
{
    static char *aszTitleDetails[] = {
	N_("Rank"), 
        N_("Type"), 
        N_("Win"), 
        N_("W g"), 
        N_("W bg"), 
        N_("Lose"), 
        N_("L g"), 
        N_("L bg"),
       NULL, 
        N_("Diff."), 
        N_("Move")
    };
	int i;
	int showWLTree = showMoveListDetail && !phd->fDetails;

/* Create list widget */
	GtkListStore *store;
	GtkTreeIter iter;
	GtkTreeSelection* sel;
	GtkWidget *view = gtk_tree_view_new();
	int offset = (phd->fDetails) ? 0 : MIN_COLUMN_COUNT - DETAIL_COLUMN_COUNT;

	if (showWLTree)
	{
		GtkStyle *psDefault = gtk_widget_get_style(view);

		GtkCellRenderer *renderer = custom_cell_renderer_movelist_new();
		gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, aszTitleDetails[ML_COL_RANK], renderer, "movelist", 0, "rank", 1, NULL);
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);
		g_object_set(renderer, "cell-background-gdk", &psDefault->bg[GTK_STATE_NORMAL],
               "cell-background-set", TRUE, NULL);

		g_object_set_data(G_OBJECT(view), "hintdata", phd);
	}
	else
	{
		gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, aszTitleDetails[ML_COL_RANK], gtk_cell_renderer_text_new(), "text", ML_COL_RANK, "foreground", ML_COL_FGCOL + offset, NULL);
		gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, aszTitleDetails[ML_COL_TYPE], gtk_cell_renderer_text_new(), "text", ML_COL_TYPE, "foreground", ML_COL_FGCOL + offset, NULL);

		if (phd->fDetails)
		{
			gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, aszTitleDetails[ML_COL_WIN], gtk_cell_renderer_text_new(), "text", ML_COL_WIN, "foreground", ML_COL_FGCOL + offset, NULL);
			gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, aszTitleDetails[ML_COL_GWIN], gtk_cell_renderer_text_new(), "text", ML_COL_GWIN, "foreground", ML_COL_FGCOL + offset, NULL);
			gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, aszTitleDetails[ML_COL_BGWIN], gtk_cell_renderer_text_new(), "text", ML_COL_BGWIN, "foreground", ML_COL_FGCOL + offset, NULL);
			gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, aszTitleDetails[ML_COL_LOSS], gtk_cell_renderer_text_new(), "text", ML_COL_LOSS, "foreground", ML_COL_FGCOL + offset, NULL);
			gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, aszTitleDetails[ML_COL_GLOSS], gtk_cell_renderer_text_new(), "text", ML_COL_GLOSS, "foreground", ML_COL_FGCOL + offset, NULL);
			gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, aszTitleDetails[ML_COL_BGLOSS], gtk_cell_renderer_text_new(), "text", ML_COL_BGLOSS, "foreground", ML_COL_FGCOL + offset, NULL);
		}

		gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, aszTitleDetails[ML_COL_EQUITY], gtk_cell_renderer_text_new(), "text", ML_COL_EQUITY + offset, "foreground", ML_COL_FGCOL + offset, NULL);
		gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, aszTitleDetails[ML_COL_DIFF], gtk_cell_renderer_text_new(), "text", ML_COL_DIFF + offset, "foreground", ML_COL_FGCOL + offset, NULL);
		gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, aszTitleDetails[ML_COL_MOVE], gtk_cell_renderer_text_new(), "text", ML_COL_MOVE + offset, "foreground", ML_COL_FGCOL + offset, NULL);
	}

	phd->pwMoves = view;

	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	gtk_tree_selection_set_mode(sel, GTK_SELECTION_MULTIPLE);

	g_signal_connect(view, "row-activated", G_CALLBACK(HintDoubleClick), phd);
	g_signal_connect(sel, "changed", G_CALLBACK(HintSelect), phd);


/* Add empty rows */
	if (phd->fDetails)
		store = gtk_list_store_new(DETAIL_COLUMN_COUNT + 2, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
	else
	{
		if (showWLTree)
			store = gtk_list_store_new(2, G_TYPE_POINTER, G_TYPE_INT);
		else
			store = gtk_list_store_new(MIN_COLUMN_COUNT + 2, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
	}

	for (i = 0; i < phd->pml->cMoves; i++)
		gtk_list_store_append(store, &iter);

	gtk_tree_view_set_model(GTK_TREE_VIEW(view), GTK_TREE_MODEL(store));
    MoveListUpdate(phd);
}

float rBest;

GtkStyle *psHighlight = NULL;

extern GtkWidget *pwMoveAnalysis;

void MoveListRefreshSize()
{
	custom_cell_renderer_invalidate_size();
	if (pwMoveAnalysis)
	{
		hintdata *phd = (hintdata *)gtk_object_get_user_data(GTK_OBJECT(pwMoveAnalysis));
		MoveListUpdate(phd);
	}
}

/*
 * Call UpdateMostList to update the movelist in the GTK hint window.
 * For example, after new evaluations, rollouts or toggle of MWC/Equity.
 *
 */
void MoveListUpdate ( const hintdata *phd )
{
  int i, j, colNum;
  char sz[ 32 ];
  cubeinfo ci;
  movelist *pml = phd->pml;
  int col = phd->fDetails ? 8 : 2;
  int showWLTree = showMoveListDetail && !phd->fDetails;

	int offset = (phd->fDetails) ? 0 : MIN_COLUMN_COUNT - DETAIL_COLUMN_COUNT;
	GtkTreeIter iter;
	GtkListStore *store;
	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(phd->pwMoves)));
	gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);

if (!psHighlight)
{	/* Get highlight style first time in */
	GtkStyle *psTemp;
    GtkStyle *psMoves = gtk_widget_get_style(phd->pwMoves);
    GetStyleFromRCFile(&psHighlight, "move-done", psMoves);
    /* Use correct background colour when selected */
    memcpy(&psHighlight->bg[GTK_STATE_SELECTED], &psMoves->bg[GTK_STATE_SELECTED], sizeof(GdkColor));

	/* Also get colour to use for w/l stats in detail view */
    GetStyleFromRCFile(&psTemp, "move-winlossfg", psMoves);
    memcpy(&wlCol, &psTemp->fg[GTK_STATE_NORMAL], sizeof(GdkColor));
}

  /* This function should only be called when the game state matches
     the move list. */
  assert( ms.fMove == 0 || ms.fMove == 1 );
    
  GetMatchStateCubeInfo( &ci, &ms );
  rBest = pml->amMoves[ 0 ].rScore;

	if (!showWLTree)
		gtk_tree_view_column_set_title(gtk_tree_view_get_column(GTK_TREE_VIEW(phd->pwMoves), col),
			(fOutputMWC && ms.nMatchTo) ? _("MWC") : _("Equity"));

  for( i = 0; i < pml->cMoves; i++ )
  {
    float *ar = pml->amMoves[ i ].arEvalMove;
	int rankKnown;

	if (showWLTree)
		gtk_list_store_set(store, &iter, 0, pml->amMoves + i, -1);
	else
		gtk_list_store_set(store, &iter, ML_COL_DATA + offset, pml->amMoves + i, -1);

	rankKnown = 1;
    if( i && i == pml->cMoves - 1 && phd->piHighlight && i == *phd->piHighlight )
      /* The move made is the last on the list.  Some moves might
         have been deleted to fit this one in */
	{
		/* Lets count how many moves are possible to see if this is the last move */
		movelist ml;
		if (!ms.anDice[0])
		{	/* If the dice have got lost, try to find them */
			moverecord* pmr = (moverecord*)plLastMove->p;
			if (pmr)
			{
				ms.anDice[0] = pmr->anDice[0];
				ms.anDice[1] = pmr->anDice[1];
			}
		}
		GenerateMoves(&ml, ms.anBoard, ms.anDice[0], ms.anDice[1], FALSE);
		if (i < ml.cMoves - 1)
			rankKnown = 0;
	}

	if (rankKnown)
      sprintf( sz, "%d", i + 1 );
    else
      strcpy( sz, "??" );

	if (showWLTree)
	{
		gtk_list_store_set(store, &iter, 1, rankKnown ? i + 1 : -1, -1);
		goto skipoldcode;
	}
	else
		gtk_list_store_set(store, &iter, ML_COL_RANK, sz, -1);
    FormatEval( sz, &pml->amMoves[ i ].esMove );
	gtk_list_store_set(store, &iter, ML_COL_TYPE, sz, -1);

    /* gwc */
	if ( phd->fDetails )
	{
		colNum = ML_COL_WIN;
		for( j = 0; j < 5; j++ ) 
		{
			if (j == 3)
			{
				gtk_list_store_set(store, &iter, colNum, OutputPercent(1.0f - ar[ OUTPUT_WIN ] ), -1);
				colNum++;
			}
			gtk_list_store_set(store, &iter, colNum, OutputPercent(ar[j]), -1);
			colNum++;
		}
	}

    /* cubeless equity */
	gtk_list_store_set(store, &iter, ML_COL_EQUITY + offset,
                        OutputEquity( pml->amMoves[ i ].rScore, &ci, TRUE ), -1);
	if (i != 0)
	{
		gtk_list_store_set(store, &iter, ML_COL_DIFF + offset,
                          OutputEquityDiff( pml->amMoves[ i ].rScore, 
                                            rBest, &ci ), -1);
    }
	
	gtk_list_store_set(store, &iter, ML_COL_MOVE + offset,
                        FormatMove( sz, ms.anBoard,
                                    pml->amMoves[ i ].anMove ), -1);

	/* highlight row */
	if (phd->piHighlight && *phd->piHighlight == i)
	{
		char buf[20];
		sprintf(buf, "#%02x%02x%02x", psHighlight->fg[GTK_STATE_SELECTED].red / 256, psHighlight->fg[GTK_STATE_SELECTED].green / 256, psHighlight->fg[GTK_STATE_SELECTED].blue / 256);
		gtk_list_store_set(store, &iter, ML_COL_FGCOL + offset, buf, -1);
	}
skipoldcode:	/* Messy as 3 copies of code at moment... */
	gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
  }


  /* update storedmoves global struct */
  UpdateStoredMoves ( pml, &ms );
}

int MoveListGetSelectionCount(const hintdata *phd)
{
	return gtk_tree_selection_count_selected_rows(gtk_tree_view_get_selection(GTK_TREE_VIEW(phd->pwMoves)));
}

GList *MoveListGetSelectionList(const hintdata *phd)
{
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(phd->pwMoves));
	GtkTreeSelection* sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(phd->pwMoves));
	return gtk_tree_selection_get_selected_rows(sel, &model);
}

void MoveListFreeSelectionList(GList *pl)
{
	g_list_foreach (pl, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(pl);
}

move *MoveListGetMove(const hintdata *phd, GList *pl)
{
	move *m;
	int showWLTree = showMoveListDetail && !phd->fDetails;
	int col, offset = (phd->fDetails) ? 0 : MIN_COLUMN_COUNT - DETAIL_COLUMN_COUNT;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(phd->pwMoves));

	gboolean check = gtk_tree_model_get_iter(model, &iter, (GtkTreePath*)(pl->data));
	assert(check);

	if (showWLTree)
		col = 0;
	else
		col = ML_COL_DATA + offset;
	gtk_tree_model_get(model, &iter, col, &m, -1);
	return m;
}

void MoveListShowToggledClicked(GtkWidget *pw, hintdata *phd)
{
	int f = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON ( phd->pwShow ) );
	if (f)
		gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(phd->pwMoves)), GTK_SELECTION_SINGLE);
	else
		gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(phd->pwMoves)), GTK_SELECTION_MULTIPLE);

	ShowMove(phd, f);
}

gint MoveListClearSelection( GtkWidget *pw, GdkEventSelection *pes, hintdata *phd )
{
	gtk_tree_selection_unselect_all(gtk_tree_view_get_selection(GTK_TREE_VIEW(phd->pwMoves)));
    return TRUE;
}
