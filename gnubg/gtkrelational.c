/*
 * Copyright (C) 2006-2008 Christian Anthon <anthon@kiku.dk>
 * Copyright (C) 2006-2022 the AUTHORS
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
 * $Id: gtkrelational.c,v 1.60 2022/09/04 22:09:31 plm Exp $
 */

#include "config.h"
#include "backgammon.h"
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "gtkrelational.h"
#include "gtkwindows.h"
#include "gtklocdefs.h"
#include "gtkgame.h"

enum {
    COLUMN_NICK,
    COLUMN_GNUE,
    COLUMN_GCHE,
    COLUMN_GCUE,
    COLUMN_SNWE,
    COLUMN_SCHE,
    COLUMN_SCUE,
    COLUMN_WRPA,
    COLUMN_WRTA,
    COLUMN_WDTG,
    COLUMN_WDBD,
    COLUMN_MDAC,
    COLUMN_MDBC,
    COLUMN_LUCK,
    NUM_COLUMNS
};

static const gchar *titles[] = {
    N_("Nick"),
    N_("GNU\nErr"),
    N_("GNU\nMove"),
    N_("GNU\nCube"),
    N_("Snw\nErr"),
    N_("Snw\nMove"),
    N_("Snw\nCube"),
    N_("Pass"),
    N_("Take"),
    N_("WDb\nPass"),
    N_("WDb\nTake"),
    N_("MDb\nPass"),
    N_("MDb\nTake"),
    N_("Luck")
};

static GtkWidget *pwPlayerName;
static GtkWidget *pwPlayerNotes;
static GtkWidget *pwQueryText;
static GtkWidget *pwQueryResult = NULL;
static GtkWidget *pwQueryBox;

static GtkListStore *playerStore;
static GtkListStore *dbStore;
static GtkTreeIter selected_iter;
static int optionsValid;
static GtkWidget *playerTreeview, *adddb, *deldb, *gameStats, *dbList, *dbtype, *user, *password, *hostname, *login,
    *helptext;

static void CheckDatabase(const char *database);
static void DBListSelected(GtkTreeView * treeview, gpointer userdata);

#define PACK_OFFSET 4
#define OUTSIDE_FRAME_GAP PACK_OFFSET
#define INSIDE_FRAME_GAP PACK_OFFSET
#define NAME_NOTES_VGAP PACK_OFFSET
#define BUTTON_GAP PACK_OFFSET
#define QUERY_BORDER 1


/* ***************************************************************************** 
    code for drawing the plot of GNU matchErrorRate using the records
   ***************************************************************************** 

This code is based on the MWC plot.

*/
#define WIDTH   640
#define HEIGHT  480
/* Number of matches to fetch and plot */
#define NUM_PLOT 100
#define PLOT_WINDOW 5
// #define NUM_RECORDS 250 /* make sure it's no less than NUM_PLOT*/

static GdkRectangle da;            /* GtkDrawingArea size */
static double margin1x=0.08;
static double margin2x=0.08;
static double margin1y=0.08;
static double margin2y=0.05;
// static int alreadyComputed=0; /* when drawing it computes all arrays twice :( )*/


/*  static because needed for both the computing + drawing functions...
*/
static int matchMoves [NUM_PLOT]={-1}; /* vector of numbers of moves in matches*/
static double matchErrors [NUM_PLOT]={-1.0}; /* vector of total errors in matches*/
static double matchErrorRate [NUM_PLOT]={-1.0}; /* vector of match error rate per match*/
static double matchAvgErrorRate [NUM_PLOT]={-1.0}; /* vector of match error rate over last 
                                    PLOT_WINDOW matches*/
static int matchCumMoves [NUM_PLOT+1]={0}; /* vector of cumulative numbers of moves in matches*/
static double matchCumErrors [NUM_PLOT+1]={0.0}; /* vector of cumulative total errors in matches*/
static char opponentNames[NUM_PLOT][100]; /* names of opponentNames*/

static double maxError=0.001; //to avoid dividing by 0 in case of mistake
static double minError=1000.0; //to avoid dividing by 0 in case of mistake

static double minYScale, maxYScale;

static int numRecords=NUM_PLOT; 
// #define EPSILON 0.001

/*shows translation x->X when x=0=>X=a and x=1=>X=b
defined with plot MWC, could also make it extern there;
but the functions below depends on the margin definitions, so extern is more tricky,
and we'd need to start having the margins in the function parameters*/
double scaleValue(double x,double a,double b) {
    return a+x*(b-a);
}
/* convert x in [0,1] to its X plotting value */
double xToX (double x) { 
    /*
    x=0->X=margin1*d
    x=1->X=(1-margin2)*d
    */   
    return scaleValue(x,margin1x*da.width,(1-margin2x)*da.width);
    //  OLD   // (i/(n-1))*da.width*19/20+da.width/20
    // /*
    // x=0->X=margin*d
    // x=1->X=d
    // */       
    // return (da.width*(x*(1-margin)+margin));
}
/* convert index i to its X plotting value by using the number of moves at match i */
double iToX (int i) { 
    /* for i: how many moves have been played between i and "the end of the vector == 
        the beginning of the match records", scaled by total played moves */
    double x=((double)matchCumMoves[i]) / ((double)matchCumMoves[0]);
    return xToX(x);
}
/* convert y in [0,1] to its Y plotting value */
double trueHistY (double y) { //}, gfloat h, gfloat margin) {
    /*
    y=0->-h(1-margin1) on screen->Y=+h(1-margin1)
    y=1->-h*margin2 on screen->Y=+h*margin2
    */
    return scaleValue(y,(1-margin1y)*da.height,margin2y*da.height);

//    /*
//    y=0->-h(1-margin) on screen->Y=+h(1-margin)
//    y=1->Y=0
//    */
//     return (da.height*(1-y)*(1-margin));
}
/* convert error to its Y plotting value using the rigth scaling */
double errorToY(double error){
    double y=(error-minYScale)/(maxYScale-minYScale);
    return trueHistY(y);
}

// #define MAX(a,b) ((a) > (b) ? (a) : (b))
// #define MIN(a,b) ((a) < (b) ? (a) : (b))


static gboolean
DrawHistoryPlot (GtkWidget *widget, GdkEventExpose *event, gpointer UNUSED(user_data))
{
    // GdkRectangle da;            /* GtkDrawingArea size */
    double dx = 2.0, dy = 2.0; /* Pixels between each point */
    double fontSize =11.0;
    double clip_x1 = 0.0, clip_y1 = 0.0, clip_x2 = 0.0, clip_y2 = 0.0;
    // gdouble i, clip_x1 = 0.0, clip_y1 = 0.0, clip_x2 = 0.0, clip_y2 = 0.0;
    char strTemp[100];

#if GTK_CHECK_VERSION(3,0,0)
    /*this is an "on_draw" in GTK3*/
        // "convert" the G*t*kWidget to G*d*kWindow (no, it's not a GtkWindow!)
    GdkWindow* window = gtk_widget_get_window(widget);  

    cairo_region_t * cairoRegion = cairo_region_create();

    GdkDrawingContext * drawingContext;
    drawingContext = gdk_window_begin_draw_frame (window,cairoRegion);

    // say: "I want to start drawing"
    cairo_t * cr = gdk_drawing_context_get_cairo_context (drawingContext);
#else
    cairo_t *cr = gdk_cairo_create (widget->window);
#endif    
       /* Define a clipping zone to improve performance */
    cairo_rectangle (cr,
            event->area.x,
            event->area.y,
            event->area.width,
            event->area.height);
    cairo_clip (cr);
#if GTK_CHECK_VERSION(3,0,0)
    /* Determine GtkDrawingArea dimensions */
    gdk_window_get_geometry (window,
            &da.x,
            &da.y,
            &da.width,
            &da.height);
#else
    /* Determine GtkDrawingArea dimensions */
    int unused = 0;
    gdk_window_get_geometry (widget->window,
            &da.x,
            &da.y,
            &da.width,
            &da.height,
            &unused);
#endif  

    /* we check already before calling the function, this is just to make sure and 
    could be deleted*/

    if(numRecords>1){

        /* Determine the data points to calculate (i.e. those in the clipping zone */
        cairo_device_to_user_distance (cr, &dx, &dy);
        cairo_clip_extents (cr, &clip_x1, &clip_y1, &clip_x2, &clip_y2);
        cairo_set_font_size(cr, fontSize);
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

        /* set scale on y-axis plot + sanity check + making sure min<max + leaving some margin*/
        minYScale=MIN(minError,maxError)/1.05+0.001;
        maxYScale=MAX(minError,maxError)*1.05-0.001;

        const double dashed2[] = {14.0, 6.0};
        int len2  = sizeof(dashed2) / sizeof(dashed2[0]);

        /* Draws x and y axes */
        cairo_set_line_width (cr, dy);
        // cairo_set_source_rgb (cr, 0.1, 0.9, 0.0);
        cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
        drawArrow(cr, xToX(0.0), trueHistY(0.0),xToX(0.0), trueHistY(1.0));
        drawArrow(cr, xToX(0.0), trueHistY(0.0),xToX(1.0), trueHistY(0.0));
        // cairo_move_to (cr, xToX(0.0), trueHistY(0.0));
        // cairo_line_to (cr, xToX(0.0), trueHistY(1.0));
        // cairo_move_to (cr, xToX(0.0), trueHistY(0.0));
        // cairo_line_to (cr, xToX(1.0), trueHistY(0.0));
        // cairo_stroke (cr);

        /* PLOT 1: Avg error */
        cairo_set_source_rgb (cr, 1.0, 0.65, 0.0);
        /* 1. the newest record is the first, so we conceptually start by plotting 
        the oldest; 2. it's an average, so it's not defined on all i's*/
        for (int i = numRecords-PLOT_WINDOW; i >=0; --i) {
            cairo_line_to (cr, iToX(i), errorToY(matchAvgErrorRate[i]));
        }
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_MITER); 
        cairo_stroke (cr);
            /*discs*/
        for (int i = numRecords-PLOT_WINDOW; i >=0; --i) {
            cairo_arc(cr, iToX(i), errorToY(matchAvgErrorRate[i]), dx/2, 0, 2 * M_PI);
            cairo_stroke_preserve(cr);
            cairo_fill(cr);
        }
        
            /* +legend */
        cairo_set_source_rgb (cr, 1.0, 0.65, 0.0);
        cairo_set_dash(cr, dashed2, 0, 1); /*disable*/
        cairo_move_to (cr, xToX(0.5), trueHistY(1.0+margin2y/2));
        cairo_line_to (cr, xToX(0.55), trueHistY(1.0+margin2y/2));
        cairo_stroke (cr);
        cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
        cairo_move_to(cr,  xToX(0.57), trueHistY(1.0+margin2y/2)+0.3*fontSize);
        cairo_show_text(cr, "5-match error");
        cairo_stroke (cr);

        /* PLOT 2: match error*/
        cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
        for (int i = numRecords-1; i >=0; --i) {
        // for (int i = 0; i < numRecords; i ++) {
            cairo_line_to (cr, iToX(i), errorToY(matchErrorRate[i]));
            // g_message("i=%d,val=%f",i,matchErrorRate[i]);
        }
        // cairo_set_source_rgba (cr, 1, 0.6, 0.0, 0.6); //red, green, blue, translucency;
                            //cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0) = black
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_MITER); 
        cairo_stroke (cr);
            /*discs*/
        for (int i = numRecords-1; i >=0; --i) {
        // for (int i = 0; i < numRecords; i ++) {
            cairo_arc(cr, iToX(i), errorToY(matchErrorRate[i]), dx/2, 0, 2 * M_PI);
            cairo_stroke_preserve(cr);
            cairo_fill(cr);
        }        
            /* +legend */
        cairo_set_line_width (cr, dy/3);
        cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
        cairo_move_to (cr, xToX(0.05), trueHistY(1.0+margin2y/2));
        cairo_line_to (cr, xToX(0.1), trueHistY(1.0+margin2y/2));
        cairo_stroke (cr);
        cairo_move_to(cr,  xToX(0.12), trueHistY(1.0+margin2y/2)+0.3*fontSize);
        cairo_show_text(cr, "Match error rate");
        cairo_stroke (cr);

         /* x axis*/
        // for (int i = 10; i < numRecords; i=i+10) {
        // for (int j = 1; j <=10; j++) {
        double xLabel=ceil(0.11*((double)matchCumMoves[0]));
        for (double i = xLabel; i <(double)matchCumMoves[0]; i+=xLabel) {
            /* grid lines*/
            cairo_set_line_width (cr, dy/3);
            cairo_set_dash(cr, dashed2, len2, 1);          
            cairo_set_source_rgb (cr, 0.6, 0.6, 0.6);
            // for (int i = numRecords-1; i >=0; i=MIN(i-1,i-numRecords/5)) {
            // g_message("matchCumMoves[0]=%d,i=%f, xLabel=%f",matchCumMoves[0],i,xLabel);
            // double x=((double)matchCumMoves[i]) / ((double)matchCumMoves[0]);
            // /* [commented: axis markers] */
            // cairo_move_to (cr, xToX(((double)i)/(n-1)), trueHistY(-0.03));
            // cairo_line_to (cr, xToX(((double)i)/(n-1)), trueHistY(0.03));
            cairo_move_to (cr, xToX(i/ ((double)matchCumMoves[0])), trueHistY(0.0));
            cairo_line_to (cr, xToX(i/ ((double)matchCumMoves[0])), trueHistY(1.0));
            cairo_stroke (cr);

            /* text: x-axis labels */
            cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
            // cairo_move_to (cr, xToX(((double)i)/(numRecords-1)), trueHistY(0.0));
            // cairo_line_to (cr, xToX(((double)i)/(numRecords-1)), trueHistY(1.0));
            cairo_move_to(cr, xToX(i/ ((double)matchCumMoves[0]))-4*dx, trueHistY(0.0)+1.25*fontSize);
            sprintf(strTemp, "%d", (int)i);
            cairo_show_text(cr, strTemp);  
        }
            cairo_move_to(cr, xToX(0.5)-10*dx, trueHistY(0.0)+2.5*fontSize);
            cairo_show_text(cr, "decisions (cube or move)");  

         /* y axis*/
        for (double j = 0.0; j <1.0; j+=0.1) {
            /*grid lines*/
            if (j>0.0) {
                cairo_set_source_rgb (cr, 0.6, 0.6, 0.6);
                cairo_set_dash(cr, dashed2, len2, 1);
                cairo_move_to (cr, xToX(0.0), trueHistY(j));
                cairo_line_to (cr, xToX(1.0), trueHistY(j));
                cairo_stroke (cr);
            }

            /* text: y-axis labels */
            cairo_move_to(cr, xToX(-0.08), trueHistY(j)+0.3*fontSize);
            sprintf(strTemp, "%.1f", scaleValue(j,minYScale,maxYScale));
            cairo_show_text(cr, strTemp); 
            cairo_stroke (cr);
        }         

        /* drawing: new matches (vertical lines) */
        cairo_set_line_width (cr, dy/3);
        cairo_set_source_rgb (cr, 0.3, 0.3, 0.3);
        cairo_set_dash(cr, dashed2, 0, 1); /*disable*/
        int jTemp=0;
        /* this time we start from i=0 to make sure to include the latest match */
        for (int i = 0; i <numRecords; i+=MAX(1,numRecords/7)) {
        // for (int i = numRecords-1; i >=0; i=i-MAX(1,numRecords/5)) {
            // g_message("i=%d",i);
            // cairo_move_to (cr, iToX(i), errorToY(matchErrorRate[i]));
            // cairo_line_to (cr, iToX(i), trueHistY(0.93));
            cairo_set_source_rgb (cr, 0.0, 0.0, 0.55);
            drawArrow(cr, iToX(i), trueHistY(0.86), iToX(i), 
                errorToY(matchErrorRate[i])-3*dy);
            cairo_stroke (cr);
    
            /* text: match 1, match 2... */
            // int jTemp=0;

            jTemp++;
            double Y1= (jTemp % 2 == 0)? fontSize:-fontSize;
            double Y2= (jTemp % 2 == 0)? 2*fontSize:0;
            cairo_move_to(cr, iToX(i)-10*dx,(trueHistY(0.95)+Y1));
            sprintf(strTemp, "match %d", numRecords-i);
            cairo_show_text(cr, strTemp);    
            cairo_move_to(cr, iToX(i)-10*dx,(trueHistY(0.95)+Y2));
            sprintf(strTemp, "vs. %s", opponentNames[i]);
            cairo_show_text(cr, strTemp);    
            cairo_stroke (cr);
        }    

// #if 0 /* *********************************************** */
// #endif //if 0


        // g_message("clip_x1,clip_x2,clip_y1,clip_y2:(%f,%f,%f,%f), dx,dy=%f,%f, width, height=%d,%d",
        //         clip_x1,clip_x2,clip_y1,clip_y2,dx,dy,da.width,da.height);
    } else 
        GTKMessage(_("Error, not enough datapoints for a plot."), DT_INFO);
#if GTK_CHECK_VERSION(3,0,0)

    // say: "I'm finished drawing
    gdk_window_end_draw_frame(window,drawingContext);
 
    // cleanup
    cairo_region_destroy(cairoRegion);
#else
    cairo_destroy (cr);
#endif 
    return FALSE;
}

void HistoryPlotInfo(GtkWidget* UNUSED(pw), GtkWidget* pwParent) 
{
    GtkWidget* pwInfoDialog, * pwBox;
    // const char* pch;

    pwInfoDialog = GTKCreateDialog(_("History Plot Explanations"), DT_INFO, pwParent, DIALOG_FLAG_MODAL, NULL, NULL);
#if GTK_CHECK_VERSION(3,0,0)
    pwBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
    pwBox = gtk_vbox_new(FALSE, 0);
#endif
    gtk_container_set_border_width(GTK_CONTAINER(pwBox), 8);

    gtk_container_add(GTK_CONTAINER(DialogArea(pwInfoDialog, DA_MAIN)), pwBox);

        // Add explanation text for mwc plot
    AddText(pwBox, _(" This plot shows how match winning chances (History) have evolved over the match:\
\n\n- To draw the plot, you need to first analyze a given game or match (not money play).\
\n\n- The x-axis represents all decisions in the match, including both cube and \
move decisions, and including both the user and the opponent.\
\n\n- The y-axis represents the chances of winning for the user \
(i.e., the player sitting at the bottom of the board). For instance, at the start, \
the user has 50% chances of winning. \
\n\n- The black plot shows the chances of winning the match. At the end, it either gets \
to 100\% (when the user wins) or 0\% (when the opponent wins). \
\n\n- The vertical red (respectively green) arrows represent mistakes by the user \
(resp. the opponent). Their size is equal to the History difference with the optimal decision.\
\n\n- The c above the red arrows (resp. below the green ones) indicates that it is a cube \
decision. Other mistakes correspond to move decisions. \
\n\n- The orange plot illustrates the cumulative skill difference. It is the sum of the \
red (negative) and green (positive) arrows. It is centered at 50\% for convenience. \
\n\n- The orange plot is not equal to the black plot because of the impact of dice (luck)."));

    GTKRunDialog(pwInfoDialog);
}

void CreateHistoryWindow (void)  //GtkWidget* pwParent) {
{

#if GTK_CHECK_VERSION(3,0,0)
    GtkWindow * window; 
    { // window setup
        window = (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_default_size (window, WIDTH, HEIGHT);
        gtk_window_set_position     (window, GTK_WIN_POS_CENTER);
        gtk_window_set_title        (window, "History plot");

        g_signal_connect(window, "destroy", G_CALLBACK(gtk_widget_destroy), NULL);
    }  
    GtkDrawingArea* da;
    {
        da = (GtkDrawingArea*) gtk_drawing_area_new();
        gtk_container_add(GTK_CONTAINER(window), 
            (GtkWidget*)da);

        g_signal_connect((GtkWidget*)da, "draw", G_CALLBACK(on_expose_event), NULL);    
        // g_signal_connect((GtkWidget*)da, "draw", G_CALLBACK(on_draw), NULL);    
    } 
    gtk_widget_show_all ((GtkWidget*)window);
#else

    GtkWidget *window;
    // GtkWidget *da;
    GtkWidget *helpButton;
    // window = GTKCreateDialog(_("History plot"), DT_INFO, pwParent, DIALOG_FLAG_MODAL | DIALOG_FLAG_MINMAXBUTTONS, NULL, NULL);
    //pwDialog = GTKCreateDialog(_("GNU Backgammon - Credits"), DT_INFO, pwParent, DIALOG_FLAG_MODAL, NULL, NULL);
    window = GTKCreateDialog("", DT_INFO, NULL, DIALOG_FLAG_MINMAXBUTTONS, NULL, NULL);
    //window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size (GTK_WINDOW (window), WIDTH, HEIGHT);
    gtk_window_set_title (GTK_WINDOW (window), "History plot");
    // g_signal_connect (G_OBJECT (window), "destroy", gtk_main_quit, NULL);
    // g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    // g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_widget_hide), NULL);
    // g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(CloseWindow), window);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_widget_destroy), NULL);

    gtk_container_add(GTK_CONTAINER(DialogArea(window, DA_BUTTONS)),
        helpButton = gtk_button_new_with_label(_("Explanations")));
    gtk_widget_set_tooltip_text(helpButton, 
        _("Click to obtain more explanations on this History plot")); 
    g_signal_connect(helpButton, "clicked", G_CALLBACK(HistoryPlotInfo), window);


    GtkWidget * da = gtk_drawing_area_new ();
    gtk_container_add(GTK_CONTAINER(DialogArea(window, DA_MAIN)), da);
    g_signal_connect (G_OBJECT (da), "expose-event", G_CALLBACK (DrawHistoryPlot), NULL);

    gtk_widget_show_all (window);
#endif    
}


void initHistoryArrays(void) {
    for (int i = 0; i < numRecords; i++) {
        matchErrorRate [i]=-1.0;
        matchAvgErrorRate [i]=-1.0; 
        matchMoves[i]=-1;
        matchErrors[i]=-1.0;
        matchCumMoves[i]=0;
        matchCumErrors[i]=0.0; 
    }
    maxError=0.001;
    minError=1000.0;
    numRecords=NUM_PLOT;
}

extern void ComputeHistory(void)//GtkWidget* pwParent)
{
    /* let's re-initialize all the static values and recompute the History */
    // if (numRecords<NUM_PLOT+1) {
        initHistoryArrays();
    // }
    /*   compute the needed values and fill the arrays  */

    RowSet *rs;
    RowSet *rs2;

    int moves[2];
    unsigned int i, j;
    gfloat stats[2];

    /* get player_id of player at bottom*/
    char szRequest[600];
    sprintf(szRequest, "player_id FROM player WHERE name='%s'",ap[1].szName);
    // g_message("request1=%s",szRequest);
    rs = RunQuery(szRequest);
    if (!rs){
        GTKMessage(_("Problem accessing database"), DT_INFO);
        return;
    }
    int userID=(int) strtol(rs->data[1][0], NULL, 0);
    // g_message("userID=%d",userID);

    //  player_id, name FROM player WHERE player.player_id =2
    // char szRequest[600]; 
    sprintf(szRequest, 
                    "matchstat_id,"
                    "unforced_moves," /*moves[1]*/
                    "close_cube_decisions," /*moves[2]*/
                    "cube_error_total_normalised," /* stats[6]*/
                    "chequer_error_total_normalised," /* stats[7]*/
                    "player_id0, player_id1 " 
                    "FROM matchstat NATURAL JOIN player NATURAL JOIN session "
                    "WHERE name='%s' "
                    "ORDER BY matchstat_id DESC "
                    "LIMIT %d",
                    ap[1].szName,
                    NUM_PLOT);
    // sprintf(szRequest, 
    //                 "matchstat_id,"
    //                 "total_moves,"
    //                 "unforced_moves," /*moves[1]*/
    //                 "close_cube_decisions," /*moves[2]*/
    //                 "snowie_moves,"
    //                 "error_missed_doubles_below_cp_normalised,"
    //                 "error_missed_doubles_above_cp_normalised,"
    //                 "error_wrong_doubles_below_dp_normalised,"
    //                 "error_wrong_doubles_above_tg_normalised,"
    //                 "error_wrong_takes_normalised,"
    //                 "error_wrong_passes_normalised,"
    //                 "cube_error_total_normalised," /* stats[6]*/
    //                 "chequer_error_total_normalised," /* stats[7]*/
    //                 "luck_total_normalised,"
    //                 "player_id0, player_id1,matchstat_id " 
    //                 "FROM matchstat NATURAL JOIN player NATURAL JOIN session "
    //                 "WHERE name='isaac' "
    //                 "ORDER BY matchstat_id DESC "
    //                 "LIMIT %d",
    //                 NUM_PLOT);
    // g_message("request=%s",szRequest);
    rs = RunQuery(szRequest);

    if (!rs){
        GTKMessage(_("Problem accessing database"), DT_INFO);
        return;
    }

    if (rs->rows < 2) {
        GTKMessage(_("No data in database"), DT_INFO);
        FreeRowset(rs);
        return ;
    }

    // <= ?
    for (j = 1; j < rs->rows; ++j) {
        for (i = 1; i < 3; ++i)
            moves[i - 1] = (int) strtol(rs->data[j][i], NULL, 0);

        for (i = 3; i < 5; ++i)
            stats[i - 3] = (float) g_strtod(rs->data[j][i], NULL);

        matchErrors[j-1]=(stats[0] + stats[1]) * 1000.0f;
        matchMoves[j-1]=moves[0] + moves[1];
        matchErrorRate[j-1]=Ratio(stats[0] + stats[1], moves[0] + moves[1]) * 1000.0f;
        // g_message("error:%f=%f/%d",matchErrorRate[j-1],matchErrors[j-1],matchMoves[j-1]);
        int mID=(int) strtol(rs->data[j][0],NULL, 0);
        // g_message("ID: %d, names: %s, %s",mID,rs->data[j][5],rs->data[j][6]);


        /* get name of player at top of screen*/
        int opponentID= (userID ==(int) strtol(rs->data[j][5],NULL, 0))?
            strtol(rs->data[j][6],NULL, 0) : strtol(rs->data[j][5],NULL, 0);
        // g_message("opponentID: %d",opponentID);
        sprintf(szRequest, "name FROM player WHERE player_id='%d'",opponentID);
        // g_message("request=%s",szRequest);
        rs2 = RunQuery(szRequest);
        if (!rs2){
            GTKMessage(_("Problem accessing database"), DT_INFO);
            return;
        }
        sprintf(opponentNames[j-1], "%s",rs2->data[1][0]);
        // int userID=(int) strtol(rs->data[1][0], NULL, 0);
        // g_message("opponent name=%s",opponentNames[j-1]);

    }   
    numRecords=MIN(j-1,NUM_PLOT);
    // g_message("numRecords=%d",numRecords);

    /* counting backwards because the oldest record is at the end*/
    // matchCumMoves[numRecords-1]=matchMoves[numRecords];
    // matchCumErrors[numRecords-1]=matchErrors[numRecords];
    for (int i = numRecords-1; i >=0; --i) {
        matchCumErrors[i]=matchCumErrors[i+1]+matchErrors[i];
        matchCumMoves[i]=matchCumMoves[i+1]+matchMoves[i];
        maxError=MAX(maxError,matchErrorRate[i]);
        minError=MIN(minError,matchErrorRate[i]);
        // g_message("maxerror:%f",maxError);
    }

    if(numRecords>=PLOT_WINDOW+1) { /* if we have enough data to get at least 2 points*/
        for (int i = numRecords-PLOT_WINDOW; i >=0; --i) {
            matchAvgErrorRate[i]=Ratio((matchCumErrors[i]-matchCumErrors[i+PLOT_WINDOW]),
                    (matchCumMoves[i]-matchCumMoves[i+PLOT_WINDOW]));
            // g_message("matchAvgErrorRate[%d]=%f",i,matchAvgErrorRate[i]);
        }    
    }    
    if(numRecords>1) 
        CreateHistoryWindow();
    else
        GTKMessage(_("Error, not enough datapoints for a plot."), DT_INFO);

    FreeRowset(rs);
    return;
}

/* creating this placeholder function with all the inputs needed when pressing a button;
the real function above doesn't have inputs*/
void PlotHistoryTrigger(gpointer UNUSED(p), guint UNUSED(n), GtkWidget * UNUSED(pw)){
    // this is a problem when we close the History window:
    //gtk_widget_destroy(pwStatDialog);
    ComputeHistory(); //pwStatDialog);
}



/* ***************************************************************************** */

static GtkTreeModel *
create_model(void)
{
    GtkTreeIter iter;
    RowSet *rs;

    int moves[4];
    unsigned int i, j;
    gfloat stats[9];



    /* create list store */
    playerStore = gtk_list_store_new(NUM_COLUMNS,
                                     G_TYPE_STRING,
                                     G_TYPE_FLOAT,
                                     G_TYPE_FLOAT,
                                     G_TYPE_FLOAT,
                                     G_TYPE_FLOAT,
                                     G_TYPE_FLOAT,
                                     G_TYPE_FLOAT,
                                     G_TYPE_FLOAT,
                                     G_TYPE_FLOAT,
                                     G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT);

    /* prepare the SQL query */
    rs = RunQuery("name,"
                  "SUM(total_moves),"
                  "SUM(unforced_moves),"
                  "SUM(close_cube_decisions),"
                  "SUM(snowie_moves),"
                  "SUM(error_missed_doubles_below_cp_normalised),"
                  "SUM(error_missed_doubles_above_cp_normalised),"
                  "SUM(error_wrong_doubles_below_dp_normalised),"
                  "SUM(error_wrong_doubles_above_tg_normalised),"
                  "SUM(error_wrong_takes_normalised),"
                  "SUM(error_wrong_passes_normalised),"
                  "SUM(cube_error_total_normalised),"
                  "SUM(chequer_error_total_normalised),"
                  "SUM(luck_total_normalised) " 
                  "FROM matchstat NATURAL JOIN player group by name");
    if (!rs)
        return 0;

    if (rs->rows < 2) {
        GTKMessage(_("No data in database"), DT_INFO);
        FreeRowset(rs);
        return 0;
    }

    for (j = 1; j < rs->rows; ++j) {
        for (i = 1; i < 5; ++i)
            moves[i - 1] = (int) strtol(rs->data[j][i], NULL, 0);

        for (i = 5; i < 14; ++i)
            stats[i - 5] = (float) g_strtod(rs->data[j][i], NULL);

        gtk_list_store_append(playerStore, &iter);
        gtk_list_store_set(playerStore, &iter,
                           COLUMN_NICK,
                           rs->data[j][0],
                           COLUMN_GNUE,
                           Ratio(stats[6] + stats[7], moves[1] + moves[2]) * 1000.0f,
                           COLUMN_GCHE,
                           Ratio(stats[7], moves[1]) * 1000.0f,
                           COLUMN_GCUE,
                           Ratio(stats[6], moves[2]) * 1000.0f,
                           COLUMN_SNWE,
                           Ratio(stats[6] + stats[7], moves[3]) * 1000.0f,
                           COLUMN_SCHE,
                           Ratio(stats[7], moves[3]) * 1000.0f,
                           COLUMN_SCUE,
                           Ratio(stats[6], moves[3]) * 1000.0f,
                           COLUMN_WRPA,
                           Ratio(stats[5], moves[3]) * 1000.0f,
                           COLUMN_WRTA,
                           Ratio(stats[4], moves[3]) * 1000.0f,
                           COLUMN_WDTG,
                           Ratio(stats[3], moves[3]) * 1000.0f,
                           COLUMN_WDBD,
                           Ratio(stats[2], moves[3]) * 1000.0f,
                           COLUMN_MDAC,
                           Ratio(stats[1], moves[3]) * 1000.0f,
                           COLUMN_MDBC,
                           Ratio(stats[0], moves[3]) * 1000.0f, 
                           COLUMN_LUCK, Ratio(stats[8], moves[0]) * 1000.0f, 
                           -1);
    }
    FreeRowset(rs);
    return GTK_TREE_MODEL(playerStore);
}

static void
cell_data_func(GtkTreeViewColumn * UNUSED(col),
               GtkCellRenderer * renderer, GtkTreeModel * model, GtkTreeIter * iter, gpointer column)
{
    gfloat data;
    gchar buf[20];

    /* get data pointed to by column */
    gtk_tree_model_get(model, iter, GPOINTER_TO_INT(column), &data, -1);

    /* format the data to two digits */
    g_snprintf(buf, sizeof(buf), "%.2f", data);

    /* render the string right aligned */
    g_object_set(renderer, "text", buf, NULL);
    g_object_set(renderer, "xalign", 1.0, NULL);
}

static void
add_columns(GtkTreeView * treeview)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    gint i;

    renderer = gtk_cell_renderer_text_new();

    column = gtk_tree_view_column_new_with_attributes(_("Nick"), renderer, "text", COLUMN_NICK, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 0);
    gtk_tree_view_append_column(treeview, column);

    for (i = 1; i < NUM_COLUMNS; i++) {
        column = gtk_tree_view_column_new();
        renderer = gtk_cell_renderer_text_new();
        gtk_tree_view_column_pack_end(column, renderer, TRUE);
        gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, GINT_TO_POINTER(i), NULL);
        gtk_tree_view_column_set_sort_column_id(column, i);
        gtk_tree_view_column_set_title(column, gettext(titles[i]));
        gtk_tree_view_append_column(treeview, column);
    }
}

static GtkWidget *
do_list_store(void)
{
    GtkTreeModel *model;
    GtkWidget *treeview;

    model = create_model();
    if (!model)
        return NULL;

    treeview = gtk_tree_view_new_with_model(model);
    g_object_unref(model);
#if GTK_CHECK_VERSION(3,14,0)
    /* 
     * This should not be hard coded but set from the theme.
     * Explicit deprecation starts at 3.14 but it may
     * not work in earlier gtk3 as well.
     */
#else
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(treeview), TRUE);
#endif
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(treeview), COLUMN_NICK);


    add_columns(GTK_TREE_VIEW(treeview));

    return treeview;
}

static char *
GetSelectedPlayer(void)
{
    char *name;
    GtkTreeModel *model;
    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(playerTreeview));
    if (gtk_tree_selection_count_selected_rows(sel) != 1)
        return NULL;

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(playerTreeview));
    gtk_tree_selection_get_selected(sel, &model, &selected_iter);

    gtk_tree_model_get(model, &selected_iter, COLUMN_NICK, &name, -1);
    return name;
}

static void
ShowRelationalSelect(GtkWidget * UNUSED(pw), int UNUSED(y), int UNUSED(x),
                     GdkEventButton * UNUSED(peb), GtkWidget * UNUSED(pwCopy))
{
    char *pName = GetSelectedPlayer();
    RowSet *rs;
    char *query;

    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(pwPlayerNotes)), "", -1);

    if (!pName)
        return;

    query = g_strdup_printf("player_id, name, notes FROM player WHERE player.name = '%s'", pName);
    g_free(pName);

    rs = RunQuery(query);
    g_free(query);
    if (!rs) {
        gtk_entry_set_text(GTK_ENTRY(pwPlayerName), "");
        return;
    }

    g_assert(rs->rows == 2);    /* Should be exactly one entry */

    gtk_entry_set_text(GTK_ENTRY(pwPlayerName), rs->data[1][1]);
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(pwPlayerNotes)), rs->data[1][2], -1);

    FreeRowset(rs);
}

static void
ShowRelationalClicked(GtkTreeView * UNUSED(treeview), GtkTreePath * UNUSED(path),
                      GtkTreeViewColumn * UNUSED(col), gpointer UNUSED(userdata))
{
    gchar *name = GetSelectedPlayer();
    if (!name)
        return;

    CommandRelationalShowDetails(name);
    g_free(name);
}

static GtkWidget *
GtkRelationalShowStats(void)
{
    GtkWidget *scrolledWindow;

    scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledWindow), GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledWindow), GTK_SHADOW_IN);

    playerTreeview = do_list_store();
    g_signal_connect(playerTreeview, "row-activated", (GCallback) ShowRelationalClicked, NULL);
    gtk_container_add(GTK_CONTAINER(scrolledWindow), playerTreeview);
    g_signal_connect(playerTreeview, "cursor-changed", G_CALLBACK(ShowRelationalSelect), NULL);

    return scrolledWindow;
}

extern void
GtkRelationalAddMatch(gpointer UNUSED(p), guint UNUSED(n), GtkWidget * UNUSED(pw))
{
    CommandRelationalAddMatch(NULL);
    outputx();
}

static GtkWidget *
GetRelList(RowSet * pRow)
{
    unsigned int i, j;
    GtkListStore *store;
    GType *types;
    GtkTreeIter iter;
    GtkCellRenderer *renderer;
    GtkWidget *treeview;

    unsigned int cols = pRow ? (unsigned int) pRow->cols : 0;
    unsigned int rows = pRow ? (unsigned int) pRow->rows : 0;

    if (!pRow || !rows || !cols)
        return gtk_label_new(_("Search failed or empty."));

    types = g_new(GType, cols);
    for (j = 0; j < cols; j++)
        types[j] = G_TYPE_STRING;
    store = gtk_list_store_newv(cols, types);
    g_free(types);

    for (i = 1; i < rows; i++) {
        gtk_list_store_append(store, &iter);
        for (j = 0; j < cols; j++)
            gtk_list_store_set(store, &iter, j, pRow->data[i][j], -1);
    }
    treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);
    renderer = gtk_cell_renderer_text_new();
    for (j = 0; j < cols; j++)
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), -1, pRow->data[0][j], renderer, "text", j,
                                                    NULL);
    return treeview;
}

static void
ShowRelationalErase(GtkWidget * UNUSED(pw), GtkWidget * UNUSED(notused))
{
    char *buf;
    gchar *player = GetSelectedPlayer();
    if (!player)
        return;

    buf = g_strdup_printf(_("Remove all data for %s?"), player);
    if (!GetInputYN(buf))
        return;

    sprintf(buf, "\"%s\"", player);
    CommandRelationalErase(buf);
    g_free(buf);

    gtk_list_store_remove(GTK_LIST_STORE(playerStore), &selected_iter);
}

static char *
GetText(GtkTextView * pwText)
{
    GtkTextIter start, end;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(pwText);
    char *pch;

    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    pch = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    return pch;
}

static void
UpdatePlayerDetails(GtkWidget * UNUSED(pw), GtkWidget * UNUSED(notused))
{
    char *notes;
    const char *newname;
    gchar *oldname = GetSelectedPlayer();
    if (!oldname)
        return;

    notes = GetText(GTK_TEXT_VIEW(pwPlayerNotes));
    newname = gtk_entry_get_text(GTK_ENTRY(pwPlayerName));
    if (RelationalUpdatePlayerDetails(oldname, newname, notes) != 0)
        gtk_list_store_set(GTK_LIST_STORE(playerStore), &selected_iter, 0, newname, -1);

    g_free(notes);
    g_free(oldname);
}

static void
RelationalQuery(GtkWidget * UNUSED(pw), GtkWidget * UNUSED(pwVbox))
{
    RowSet *rs;
    char *pch, *query;

    pch = GetText(GTK_TEXT_VIEW(pwQueryText));

    if (!StrNCaseCmp("select ", pch, strlen("select ")))
        query = pch + strlen("select ");
    else
        query = pch;

    rs = RunQuery(query);
    if (pwQueryResult)
        gtk_widget_destroy(pwQueryResult);
    pwQueryResult = GetRelList(rs);
    gtk_box_pack_start(GTK_BOX(pwQueryBox), pwQueryResult, TRUE, TRUE, 0);
    gtk_widget_show(pwQueryResult);
    if (rs)
        FreeRowset(rs);

    g_free(pch);
}

static DBProvider *
GetSelectedDBType(void)
{
    DBProviderType dbType = (DBProviderType) gtk_combo_box_get_active(GTK_COMBO_BOX(dbtype));
    return GetDBProvider(dbType);
}

static void
TryConnection(DBProvider * pdb, GtkWidget * dbList)
{
    const char *msg;
    DBProviderType dbType = (DBProviderType) gtk_combo_box_get_active(GTK_COMBO_BOX(dbtype));

    gtk_list_store_clear(GTK_LIST_STORE(dbStore));
    msg = TestDB(dbType);
    gtk_widget_set_sensitive(login, FALSE);
    if (msg) {
        gtk_label_set_text(GTK_LABEL(helptext), msg);
        optionsValid = FALSE;
        gtk_widget_set_sensitive(adddb, FALSE);
        gtk_widget_set_sensitive(deldb, FALSE);
    } else {                    /* Test ok */
        GList *pl = pdb->GetDatabaseList(pdb->username, pdb->password, pdb->hostname);
        if (g_list_find_custom(pl, pdb->database, (GCompareFunc) g_ascii_strcasecmp) == NULL) { /* Somehow selected database not in list, so add it */
            pl = g_list_append(pl, g_strdup(pdb->database));
        }
        while (pl) {
            int ok, seldb;
            GtkTreeIter iter;
            char *database = (char *) pl->data;
            seldb = !StrCaseCmp(database, pdb->database);
            if (seldb)
                ok = TRUE;
            else {
                const char *tmpDatabase = pdb->database;
                pdb->database = database;
                ok = (TestDB(dbType) == NULL);
                pdb->database = tmpDatabase;
            }
            if (ok) {
                gtk_list_store_append(GTK_LIST_STORE(dbStore), &iter);
                gtk_list_store_set(GTK_LIST_STORE(dbStore), &iter, 0, database, -1);
                if (seldb) {
                    gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(dbList)), &iter);
                    gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(dbList),
                                                 gtk_tree_model_get_path(gtk_tree_view_get_model(GTK_TREE_VIEW(dbList)),
                                                                         &iter), NULL, TRUE, 1, 0);
                }
            }
            g_free(database);
            pl = pl->next;
        }
        g_list_free(pl);
        CheckDatabase(pdb->database);
    }
}

static void
CredentialsChanged(void)
{
    gtk_widget_set_sensitive(login, TRUE);
}

static void
LoginClicked(GtkButton * UNUSED(button), gpointer dbList)
{
    const char *tmpUser, *tmpPass, *tmpHost;
    DBProvider *pdb = GetSelectedDBType();

    if (pdb == NULL)
        return;

    tmpUser = pdb->username;
    tmpPass = pdb->password;
    tmpHost = pdb->hostname;

    pdb->username = gtk_entry_get_text(GTK_ENTRY(user));
    pdb->password = gtk_entry_get_text(GTK_ENTRY(password));
    pdb->hostname = gtk_entry_get_text(GTK_ENTRY(hostname));

    TryConnection(pdb, dbList);

    pdb->username = tmpUser;
    pdb->password = tmpPass;
    pdb->hostname = tmpHost;
}

static void
TypeChanged(GtkComboBox * UNUSED(widget), gpointer dbList)
{
    DBProvider *pdb = GetSelectedDBType();

    if (pdb == NULL)
        return;

    if (pdb->HasUserDetails) {
        gtk_widget_set_sensitive(user, TRUE);
        gtk_widget_set_sensitive(password, TRUE);
        gtk_widget_set_sensitive(hostname, TRUE);
        gtk_entry_set_text(GTK_ENTRY(user), pdb->username);
        gtk_entry_set_text(GTK_ENTRY(password), pdb->password);
        if (pdb->hostname)
            gtk_entry_set_text(GTK_ENTRY(hostname), pdb->hostname);
        else
            gtk_entry_set_text(GTK_ENTRY(hostname), "");
    } else {
        gtk_widget_set_sensitive(user, FALSE);
        gtk_widget_set_sensitive(password, FALSE);
        gtk_widget_set_sensitive(hostname, FALSE);
    }

    TryConnection(pdb, dbList);
}

void
CheckDatabase(const char *database)
{
    int valid = FALSE;
    int dbok = 0;
    DBProvider *pdb = GetSelectedDBType();

    if (pdb)
        dbok =
            (pdb->Connect(database, gtk_entry_get_text(GTK_ENTRY(user)), gtk_entry_get_text(GTK_ENTRY(password)),
                          gtk_entry_get_text(GTK_ENTRY(hostname))) >= 0);

    if (!dbok)
        gtk_label_set_text(GTK_LABEL(helptext), _("Failed to connect to database!"));
    else {
        int version = RunQueryValue(pdb, "next_id FROM control WHERE tablename = 'version'");
        int matchcount = RunQueryValue(pdb, "count(*) FROM session");

        char *dbString, *buf, *buf2 = NULL;
        if (version < DB_VERSION)
            dbString = _("This database is from an old version of GNU Backgammon and cannot be used");
        else if (version > DB_VERSION)
            dbString = _("This database is from a new version of GNU Backgammon and cannot be used");
        else {
            if (matchcount < 0)
                dbString = _("This database structure is invalid");
            else {
                valid = TRUE;
                if (matchcount == 0)
                    dbString = _("This database contains no matches");
                else if (matchcount == 1)
                    dbString = _("This database contains 1 match");
                else {
                    buf2 = g_strdup_printf(_("This database contains %d matches\n"), matchcount);
                    dbString = buf2;
                }
            }
        }
        buf = g_strdup_printf(_("Database connection successful\n%s\n"), dbString);
        gtk_label_set_text(GTK_LABEL(helptext), buf);
        g_free(buf);
        g_free(buf2);

        pdb->Disconnect();
    }
    gtk_widget_set_sensitive(adddb, dbok);
    gtk_widget_set_sensitive(deldb, dbok);
    optionsValid = valid;
}

static char *
GetSelectedDB(GtkTreeView * treeview)
{
    GtkTreeModel *model;
    char *db = NULL;
    GtkTreeSelection *sel = gtk_tree_view_get_selection(treeview);
    if (gtk_tree_selection_count_selected_rows(sel) != 1)
        return NULL;
    gtk_tree_selection_get_selected(sel, &model, &selected_iter);
    gtk_tree_model_get(model, &selected_iter, 0, &db, -1);
    return db;
}

static void
DBListSelected(GtkTreeView * treeview, gpointer UNUSED(userdata))
{
    char *db = GetSelectedDB(treeview);
    if (db) {
        CheckDatabase(db);
        g_free(db);
    }
}

static void
AddDBClicked(GtkButton * UNUSED(button), gpointer dbList)
{
    char *dbName = GTKGetInput(_("Add Database"), _("Database Name:"), NULL);
    if (dbName) {
        DBProvider *pdb = GetSelectedDBType();
        int con = 0;
        gchar *sz;
        GtkTreeIter iter;
        GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(dbList));

        /* If the database name is already in the list, don't try to add a new one */
        if (gtk_tree_model_get_iter_first(model, &iter))
            do {
                gtk_tree_model_get(model, &iter, 0, &sz, -1);
                if (g_ascii_strcasecmp(dbName, sz) == 0) {
                    gtk_label_set_text(GTK_LABEL(helptext), _("Failed to create, database exists!"));
                    g_free(dbName);
                    return;
                }
            } while (gtk_tree_model_iter_next(model, &iter));

        if (pdb)
            con =
                pdb->Connect(dbName, gtk_entry_get_text(GTK_ENTRY(user)), gtk_entry_get_text(GTK_ENTRY(password)),
                             gtk_entry_get_text(GTK_ENTRY(hostname)));

        if (con > 0 || ((pdb) && CreateDatabase(pdb))) {
            gtk_list_store_append(GTK_LIST_STORE(dbStore), &iter);
            gtk_list_store_set(GTK_LIST_STORE(dbStore), &iter, 0, dbName, -1);
            gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(dbList)), &iter);
            pdb->Disconnect();
            CheckDatabase(dbName);
        } else
            gtk_label_set_text(GTK_LABEL(helptext), _("Failed to create database!"));

        g_free(dbName);
    }
}

static void
DelDBClicked(GtkButton * UNUSED(button), gpointer dbList)
{
    char *db = GetSelectedDB(GTK_TREE_VIEW(dbList));
    if (db && GetInputYN(_("Are you sure you want to delete all the matches in this database?"))) {
        DBProvider *pdb = GetSelectedDBType();

        if (pdb
            && pdb->DeleteDatabase(db, gtk_entry_get_text(GTK_ENTRY(user)), gtk_entry_get_text(GTK_ENTRY(password)),
                                   gtk_entry_get_text(GTK_ENTRY(hostname)))) {
            gtk_list_store_remove(GTK_LIST_STORE(dbStore), &selected_iter);
            optionsValid = FALSE;
            gtk_widget_set_sensitive(deldb, FALSE);
            gtk_label_set_text(GTK_LABEL(helptext), _("Database successfully removed"));
            pdb->database = "gnubg";
        } else
            gtk_label_set_text(GTK_LABEL(helptext), _("Failed to delete database!"));
    }
}

extern void
RelationalOptionsShown(void)
{                               /* Setup the options when tab selected */
    gtk_combo_box_set_active(GTK_COMBO_BOX(dbtype), dbProviderType);
}

extern void
RelationalSaveOptions(void)
{
    if (optionsValid) {
        DBProviderType dbType = (DBProviderType) gtk_combo_box_get_active(GTK_COMBO_BOX(dbtype));
        SetDBSettings(dbType, GetSelectedDB(GTK_TREE_VIEW(dbList)), gtk_entry_get_text(GTK_ENTRY(user)),
                      gtk_entry_get_text(GTK_ENTRY(password)), gtk_entry_get_text(GTK_ENTRY(hostname)));

        storeGameStats = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gameStats));
    }
}

extern GtkWidget *
RelationalOptions(void)
{
    int i;
    GtkWidget *hb1, *hb2, *vb1, *vb2, *lbl, *help, *pwScrolled;
#if GTK_CHECK_VERSION(3,0,0)
    GtkWidget *grid;
#else
    GtkWidget *table;
    GtkWidget *align;
#endif

    dbStore = gtk_list_store_new(1, G_TYPE_STRING);
    dbList = gtk_tree_view_new_with_model(GTK_TREE_MODEL(dbStore));
    g_object_unref(dbStore);
    gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(dbList)), GTK_SELECTION_BROWSE);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(dbList), -1, _("Databases"), gtk_cell_renderer_text_new(),
                                                "text", 0, NULL);
    gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(dbList), FALSE);

    g_signal_connect(dbList, "cursor-changed", G_CALLBACK(DBListSelected), NULL);

    dbtype = gtk_combo_box_text_new();
    for (i = 0; i < NUM_PROVIDERS; i++)
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(dbtype), GetProviderName(i));
    g_signal_connect(dbtype, "changed", G_CALLBACK(TypeChanged), dbList);

#if GTK_CHECK_VERSION(3,0,0)
    vb2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    hb2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    vb2 = gtk_vbox_new(FALSE, 0);
    hb2 = gtk_hbox_new(FALSE, 0);
#endif
    gtk_box_pack_start(GTK_BOX(vb2), hb2, FALSE, FALSE, 10);

#if GTK_CHECK_VERSION(3,0,0)
    vb1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    hb1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    vb1 = gtk_vbox_new(FALSE, 0);
    hb1 = gtk_hbox_new(FALSE, 0);
#endif
    gtk_widget_set_tooltip_text(hb1, _("Database type can be SQLite (bundled with GNU Backgammon), MySQL/MariaDB or PostgreSQL. " "The first one needs only a database name. For the other two you need an external database server and to configure how to access it"));
    gtk_box_pack_start(GTK_BOX(hb1), gtk_label_new(_("DB Type")), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hb1), dbtype, TRUE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vb1), hb1, FALSE, FALSE, 0);

#if GTK_CHECK_VERSION(3,0,0)
    grid = gtk_grid_new();
#else
    table = gtk_table_new(4, 2, FALSE);
#endif

    lbl = gtk_label_new(_("Username"));

#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_halign(lbl, GTK_ALIGN_START);
    gtk_widget_set_valign(lbl, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(grid), lbl, 0, 0, 1, 1);
#else
    gtk_misc_set_alignment(GTK_MISC(lbl), 0, 0.5);
    gtk_table_attach(GTK_TABLE(table), lbl, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
#endif

    user = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(user), 20);
    g_signal_connect(user, "changed", G_CALLBACK(CredentialsChanged), dbList);
#if GTK_CHECK_VERSION(3,0,0)
    gtk_grid_attach(GTK_GRID(grid), user, 1, 0, 1, 1);
#else
    gtk_table_attach(GTK_TABLE(table), user, 1, 2, 0, 1, 0, 0, 0, 0);
#endif

    lbl = gtk_label_new(_("Password"));

#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_halign(lbl, GTK_ALIGN_START);
    gtk_widget_set_valign(lbl, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(grid), lbl, 0, 1, 1, 1);
#else
    gtk_misc_set_alignment(GTK_MISC(lbl), 0, 0.5);
    gtk_table_attach(GTK_TABLE(table), lbl, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
#endif

    password = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(password), 20);
    gtk_entry_set_visibility(GTK_ENTRY(password), FALSE);
    g_signal_connect(password, "changed", G_CALLBACK(CredentialsChanged), dbList);
#if GTK_CHECK_VERSION(3,0,0)
    gtk_grid_attach(GTK_GRID(grid), password, 1, 1, 1, 1);
#else
    gtk_table_attach(GTK_TABLE(table), password, 1, 2, 1, 2, 0, 0, 0, 0);
#endif

    lbl = gtk_label_new(_("DB server"));
    gtk_widget_set_tooltip_text(lbl, _("Entered as: hostname:port"));

#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_halign(lbl, GTK_ALIGN_START);
    gtk_widget_set_valign(lbl, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(grid), lbl, 0, 2, 1, 1);
#else
    gtk_misc_set_alignment(GTK_MISC(lbl), 0, 0.5);
    gtk_table_attach(GTK_TABLE(table), lbl, 0, 1, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
#endif

    hostname = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(hostname), 64);
    gtk_entry_set_width_chars(GTK_ENTRY(hostname), 20);
    gtk_entry_set_visibility(GTK_ENTRY(hostname), TRUE);
    g_signal_connect(hostname, "changed", G_CALLBACK(CredentialsChanged), dbList);
#if GTK_CHECK_VERSION(3,0,0)
    gtk_grid_attach(GTK_GRID(grid), hostname, 1, 2, 1, 1);
#else
    gtk_table_attach(GTK_TABLE(table), hostname, 1, 2, 2, 3, 0, 0, 0, 0);
#endif

    login = gtk_button_new_with_label(_("Login"));
    g_signal_connect(login, "clicked", G_CALLBACK(LoginClicked), dbList);
    gtk_widget_set_tooltip_text(login, _("Check connection to database server"));

#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_halign(login, GTK_ALIGN_END);
    gtk_widget_set_valign(login, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), login, 1, 3, 1, 1);
#else
    align = gtk_alignment_new(1, 0, 0, 0); 	 
    gtk_container_add(GTK_CONTAINER(align), login);
    gtk_table_attach(GTK_TABLE(table), align, 1, 2, 3, 4, GTK_EXPAND | GTK_FILL, 0, 0, 0);
#endif

#if GTK_CHECK_VERSION(3,0,0)
    gtk_box_pack_start(GTK_BOX(vb1), grid, FALSE, FALSE, 4);
#else
    gtk_box_pack_start(GTK_BOX(vb1), table, FALSE, FALSE, 4);
#endif

    gameStats = gtk_check_button_new_with_label(_("Store game stats"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gameStats), storeGameStats);
    gtk_box_pack_start(GTK_BOX(vb1), gameStats, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(gameStats, _("Store individual games statistics in addition to global match ones"));

    gtk_box_pack_start(GTK_BOX(hb2), vb1, FALSE, FALSE, 10);

    help = gtk_frame_new(_("Info"));
    helptext = gtk_label_new(NULL);
#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_halign(helptext, GTK_ALIGN_START);
    gtk_widget_set_valign(helptext, GTK_ALIGN_START);
    g_object_set(helptext, "margin", 4, NULL);
#else
    gtk_misc_set_alignment(GTK_MISC(helptext), 0, 0);
    gtk_misc_set_padding(GTK_MISC(helptext), 4, 4);
#endif
    gtk_widget_set_size_request(helptext, 400, 70);
    gtk_container_add(GTK_CONTAINER(help), helptext);
    gtk_box_pack_start(GTK_BOX(vb2), help, FALSE, FALSE, 4);

    pwScrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(pwScrolled, 100, 100);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(pwScrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(pwScrolled), dbList);

#if GTK_CHECK_VERSION(3,0,0)
    vb1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
    vb1 = gtk_vbox_new(FALSE, 0);
#endif
    gtk_box_pack_start(GTK_BOX(hb2), vb1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vb1), pwScrolled, FALSE, FALSE, 0);
#if GTK_CHECK_VERSION(3,0,0)
    hb1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    hb1 = gtk_hbox_new(FALSE, 0);
#endif
    gtk_box_pack_start(GTK_BOX(vb1), hb1, FALSE, FALSE, 0);
    adddb = gtk_button_new_with_label(_("Add database"));
    g_signal_connect(adddb, "clicked", G_CALLBACK(AddDBClicked), dbList);
    gtk_box_pack_start(GTK_BOX(hb1), adddb, FALSE, FALSE, 0);
    deldb = gtk_button_new_with_label(_("Delete database"));
    g_signal_connect(deldb, "clicked", G_CALLBACK(DelDBClicked), dbList);
    gtk_box_pack_start(GTK_BOX(hb1), deldb, FALSE, FALSE, 4);

    return vb2;
}
 
extern void
GtkShowRelational(gpointer UNUSED(p), guint UNUSED(n), GtkWidget * UNUSED(pw))
{
    
    GtkWidget *pwRun, *pwDialog, *pwHbox2, *pwVbox2,
        *pwPlayerFrame, *pwUpdate, *pwPaned, *pwVbox, *pwErase, *pwOpen, 
        *pwn, *pwLabel, *pwScrolled, *pwHbox, *histButton;
    DBProvider *pdb;
    static GtkTextBuffer *query = NULL; /*remember query */

    if (((pdb = ConnectToDB(dbProviderType)) == NULL) || RunQueryValue(pdb, "count(*) FROM player") < 2) {
        if (pdb)
            pdb->Disconnect();

        GTKMessage(_("No data in database"), DT_INFO);
        return;
    }
    pdb->Disconnect();

    pwDialog = GTKCreateDialog(_("GNU Backgammon - Database"),
            DT_INFO, NULL, DIALOG_FLAG_MINMAXBUTTONS, NULL, NULL);
            // DT_INFO, NULL, DIALOG_FLAG_MODAL | DIALOG_FLAG_MINMAXBUTTONS, NULL, NULL);

#define REL_DIALOG_HEIGHT 600
    gtk_window_set_default_size(GTK_WINDOW(pwDialog), -1, REL_DIALOG_HEIGHT);

    gtk_container_add(GTK_CONTAINER(DialogArea(pwDialog, DA_BUTTONS)),
        histButton = gtk_button_new_with_label(_("Plot History")));
    g_signal_connect(histButton, "clicked", G_CALLBACK(PlotHistoryTrigger), NULL);

    pwn = gtk_notebook_new();
    gtk_container_set_border_width(GTK_CONTAINER(pwn), 0);

/*******************************************************
** Start of (left hand side) of player screen...
*******************************************************/

#if GTK_CHECK_VERSION(3,0,0)
    pwPaned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    pwVbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
    pwPaned = gtk_vpaned_new();
    pwVbox = gtk_vbox_new(FALSE, 0);
#endif
    gtk_paned_set_position(GTK_PANED(pwPaned), (int) (REL_DIALOG_HEIGHT * 0.6));
    gtk_notebook_append_page(GTK_NOTEBOOK(pwn), pwPaned, gtk_label_new(_("Players")));
    gtk_container_set_border_width(GTK_CONTAINER(pwVbox), INSIDE_FRAME_GAP);
    gtk_paned_add1(GTK_PANED(pwPaned), pwVbox);
    gtk_box_pack_start(GTK_BOX(pwVbox), GtkRelationalShowStats(), TRUE, TRUE, 0);
#if GTK_CHECK_VERSION(3,0,0)
    pwHbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    pwHbox2 = gtk_hbox_new(FALSE, 0);
#endif
    gtk_box_pack_start(GTK_BOX(pwVbox), pwHbox2, FALSE, FALSE, 0);

    pwOpen = gtk_button_new_with_label(_("Open"));
    g_signal_connect(G_OBJECT(pwOpen), "clicked", G_CALLBACK(ShowRelationalClicked), NULL);
    gtk_box_pack_start(GTK_BOX(pwHbox2), pwOpen, FALSE, FALSE, 0);

    pwErase = gtk_button_new_with_label(_("Erase"));
    g_signal_connect(G_OBJECT(pwErase), "clicked", G_CALLBACK(ShowRelationalErase), NULL);
    gtk_box_pack_start(GTK_BOX(pwHbox2), pwErase, FALSE, FALSE, BUTTON_GAP);

/*******************************************************
** Start of right hand side of player screen...
*******************************************************/

    pwPlayerFrame = gtk_frame_new(_("Player"));
    gtk_container_set_border_width(GTK_CONTAINER(pwPlayerFrame), OUTSIDE_FRAME_GAP);
    gtk_paned_add2(GTK_PANED(pwPaned), pwPlayerFrame);

#if GTK_CHECK_VERSION(3,0,0)
    pwVbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, NAME_NOTES_VGAP);
#else
    pwVbox = gtk_vbox_new(FALSE, NAME_NOTES_VGAP);
#endif
    gtk_container_set_border_width(GTK_CONTAINER(pwVbox), INSIDE_FRAME_GAP);
    gtk_container_add(GTK_CONTAINER(pwPlayerFrame), pwVbox);

#if GTK_CHECK_VERSION(3,0,0)
    pwHbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    pwHbox2 = gtk_hbox_new(FALSE, 0);
#endif
    gtk_box_pack_start(GTK_BOX(pwVbox), pwHbox2, FALSE, FALSE, 0);

    pwLabel = gtk_label_new(_("Name"));
    gtk_box_pack_start(GTK_BOX(pwHbox2), pwLabel, FALSE, FALSE, 0);
    pwPlayerName = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(pwHbox2), pwPlayerName, TRUE, TRUE, 0);

#if GTK_CHECK_VERSION(3,0,0)
    pwVbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
    pwVbox2 = gtk_vbox_new(FALSE, 0);
#endif
    gtk_box_pack_start(GTK_BOX(pwVbox), pwVbox2, TRUE, TRUE, 0);

    pwLabel = gtk_label_new(_("Notes"));
#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_halign(pwLabel, GTK_ALIGN_START);
    gtk_widget_set_valign(pwLabel, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment(GTK_MISC(pwLabel), 0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(pwVbox2), pwLabel, FALSE, FALSE, 0);

    pwPlayerNotes = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(pwPlayerNotes), TRUE);

    pwScrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(pwScrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(pwScrolled), pwPlayerNotes);
    gtk_box_pack_start(GTK_BOX(pwVbox2), pwScrolled, TRUE, TRUE, 0);

#if GTK_CHECK_VERSION(3,0,0)
    pwHbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    pwHbox2 = gtk_hbox_new(FALSE, 0);
#endif
    gtk_box_pack_start(GTK_BOX(pwVbox), pwHbox2, FALSE, FALSE, 0);

    pwUpdate = gtk_button_new_with_label(_("Update Details"));
    gtk_box_pack_start(GTK_BOX(pwHbox2), pwUpdate, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(pwUpdate), "clicked", G_CALLBACK(UpdatePlayerDetails), NULL);

/*******************************************************
** End of right hand side of player screen...
*******************************************************/

    /* Query sheet */
#if GTK_CHECK_VERSION(3,0,0)
    pwVbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
    pwVbox = gtk_vbox_new(FALSE, 0);
#endif
    gtk_notebook_append_page(GTK_NOTEBOOK(pwn), pwVbox, gtk_label_new(_("Query")));
    gtk_container_set_border_width(GTK_CONTAINER(pwVbox), INSIDE_FRAME_GAP);

    pwLabel = gtk_label_new(_("Query text"));
#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_halign(pwLabel, GTK_ALIGN_START);
    gtk_widget_set_valign(pwLabel, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment(GTK_MISC(pwLabel), 0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(pwVbox), pwLabel, FALSE, FALSE, 0);

    if (!query) {
        query = gtk_text_buffer_new(NULL);
        pwQueryText = gtk_text_view_new_with_buffer(query);
        gtk_text_buffer_set_text(query,
                                 "s.session_id, s.length, p1.name, p2.name from (player p1, player p2) join session s on s.player_id0 = p1.player_id and s.player_id1 = p2.player_id",
                                 -1);
    } else
        pwQueryText = gtk_text_view_new_with_buffer(query);


    gtk_text_view_set_border_window_size(GTK_TEXT_VIEW(pwQueryText), GTK_TEXT_WINDOW_TOP, QUERY_BORDER);
    gtk_text_view_set_border_window_size(GTK_TEXT_VIEW(pwQueryText), GTK_TEXT_WINDOW_RIGHT, QUERY_BORDER);
    gtk_text_view_set_border_window_size(GTK_TEXT_VIEW(pwQueryText), GTK_TEXT_WINDOW_BOTTOM, QUERY_BORDER);
    gtk_text_view_set_border_window_size(GTK_TEXT_VIEW(pwQueryText), GTK_TEXT_WINDOW_LEFT, QUERY_BORDER);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(pwQueryText), TRUE);
    gtk_box_pack_start(GTK_BOX(pwVbox), pwQueryText, FALSE, FALSE, 0);
    gtk_widget_set_size_request(pwQueryText, 250, 80);

#if GTK_CHECK_VERSION(3,0,0)
    pwHbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    pwHbox = gtk_hbox_new(FALSE, 0);
#endif
    gtk_box_pack_start(GTK_BOX(pwVbox), pwHbox, FALSE, FALSE, 0);
    pwLabel = gtk_label_new(_("Result"));
#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_halign(pwLabel, GTK_ALIGN_START);
    gtk_widget_set_valign(pwLabel, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment(GTK_MISC(pwLabel), 0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(pwHbox), pwLabel, TRUE, TRUE, 0);

    pwRun = gtk_button_new_with_label(_("Run Query"));
    g_signal_connect(G_OBJECT(pwRun), "clicked", G_CALLBACK(RelationalQuery), pwVbox);
    gtk_box_pack_start(GTK_BOX(pwHbox), pwRun, FALSE, FALSE, 0);

#if GTK_CHECK_VERSION(3,0,0)
    pwQueryBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
    pwQueryBox = gtk_vbox_new(FALSE, 0);
#endif
    pwScrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(pwScrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
#if GTK_CHECK_VERSION(3, 8, 0)
    gtk_container_add(GTK_CONTAINER(pwScrolled), pwQueryBox);
#else
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(pwScrolled), pwQueryBox);
#endif
    gtk_box_pack_start(GTK_BOX(pwVbox), pwScrolled, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(DialogArea(pwDialog, DA_MAIN)), pwn);

    GTKRunDialog(pwDialog);
}

extern void
GtkShowQuery(RowSet * pRow)
{
    GtkWidget *pwDialog;

    pwDialog = GTKCreateDialog(_("GNU Backgammon - Database Result"), DT_INFO, NULL, DIALOG_FLAG_MODAL, NULL, NULL);

    gtk_container_add(GTK_CONTAINER(DialogArea(pwDialog, DA_MAIN)), GetRelList(pRow));

    GTKRunDialog(pwDialog);
}
