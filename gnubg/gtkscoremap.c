/*
 * gtkscoremap.c
 *
 * Authors: Aaron Tikuisis and Isaac Keslassy, 2020.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 3 or later of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Scorele Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 */


/***************************************************      Explanations:       ************************************* 
      The main function is GTKShowScoreMap.
            1. It builds the window, including the score map, gauge and radio buttons, by calling BuildTableContainer()
                    [which calls InitQuadrant() for each score (i.e. in each scoremap square=quadrant) to initialize
                    the drawing tools].
                    It also initializes variables.
            2. It calls UpdateCubeInfoArray(), which initializes the match state in each quadrant, by filling 
                    psm->aaQuadrantData[i][j].ci for each i,j
            3. It calls CalcScoreMapEquities(), which for each score calls CalcQuadrantEquities() to find  
                    the equity of each decision and therefore the best decision as well,
                    and uses it to set the text for the corresponding quadrant (the best decision is stored in
                    psm->aaQuadrantData[i][j].decisionString). This text is displayed in step 5 below. 
                    In the move scoremap, FindMostFrequentMoves() finds the top-k most frequent distinct best moves
                    and assigns them distinct colors, as well as English descriptions (in the "alpha version" where 
                    English description is allowed). 
            4. It calls  UpdateScoreMapVisual(), which for each quadrant runs ColourQuadrant(), updating:
                    (a) The color of each quadrant (for cube scoremap: using FindColour1() or FindColour2(), 
                    depending on the coloring scheme; and for move scoremap, using FindMoveColour()). For instance,
                    in the move scoremap, FindMoveColour() checks whether the current best move is equal to one of 
                    the TOP_K most common moves, and updates the quadrant and border colors according to the associated
                    color (e.g., if it's red, it sets the border in strong red, and the quadrant in weak red, based
                    on the equity difference with the 2nd best move).
                        (aa) In addition, in the "alpha version" where English description is allowed,
                        if the English description option is indeed set in the move scoremap, 
                        when the best move is one of the TOP_K moves, FindMoveColour() does not only copy the 
                        associated color, but also the associated English description.
                    (b) The hover text of each quadrant
                    It then sets: 
                    (c) the row/col score labels
                    Next, for the cube scoremap, it runs DrawGauge() which updates 
                    (d) the gauge,
                    and also it adds labels to the gauge.
            5. Careful! Finally, at the end, ColourQuadrant() asks to redraw, which implicitly calls DrawQuadrant(). 
                    This function updates the decision text in each square. For instance, in the move scoremap, if 
                    the English description is set, it will take the description, and cut it in two rows if needed.

     Changing the eval ply launches the ScoreMapPlyToggled function that also calls the functions in steps 2-5 above.
     Changing the colour-by (ND/D or T/P) launches the ColourByToggled function that only calls UpdateScoreMapVisual 
         (no need for new equity values); and so on for the different radio buttons.
*/

/*INSTALL: files to update for the ScoreMap to work:
backgammon.h
commands.inc
gtkchequer.c
gtkchequer.h
gtkcube.c
gtkgame.c
gtkscoremap.c
gtkscoremap.h
gtkuidefs.h
Makefile.am 
show.c 
*/

/* TBD

- Converge on a layout once we get feedback (VERTICAL1, VERTICAL2, HORIZONTAL).


Known issues:

-  the scoremap button of the 1st move of any game in an analyzed match doesn't work at first. The error message is:
"No game in progress (type `new game' to start one)." 
However, if we click on another move (even w/o doing anything else) then come back, the scoremap of the 1st move then works.
Yet, if we change the game and come back: again, it doesn't work.

It looks like this is not a scoremap issue. For instance, the "show" button has the same issue: if we pick the 1st move 
(or one of its alternatives) of any game and click "show", we get an "illegal move" error message; if we click elsewhere 
and come back, it works again. It may be a movelist construction issue.
*/

#include "config.h"
#include "gtklocdefs.h"

#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "backgammon.h"
#include "eval.h"
#include "gtkscoremap.h"
#include "gtkgame.h"
#include "drawboard.h"
#include "format.h"
#include "render.h"
#include "renderprefs.h"
#include "gtkboard.h"
#include "gtkwindows.h"

#define LAYOUT_HORIZONTAL FALSE     // Horizontal layout: options on side, instead of options on bottom
#define LAYOUT_VERTICAL1 FALSE      // First vertical layout with a list of lines, no frames
#define LAYOUT_VERTICAL2 TRUE       // Second vertical layout with frames                                  

#define MAX_SIZE_QUADRANT 100
#define MAX_TABLE_SCREENSIZE 400
#define MAX_FONT_SIZE 12
#define MIN_FONT_SIZE 7

const static int MATCH_SIZE_OPTIONS[] = {3,5,7,9,11,13,15,17,21};   //list of allowed match sizes
#define NUM_MATCH_SIZE_OPTIONS (sizeof(MATCH_SIZE_OPTIONS)/sizeof(int))
#define MAX_TABLE_SIZE 22 // CAREFUL: should be max(MATCH_SIZE_OPTIONS)+1  (max-1 for cube scoremap, max+1 for move)
#define PREFERRED_CUBE_MATCH_SIZE_INDEX 2   //index in MATCH_SIZE_OPTIONS to use as default option for cube scoremap; 
                                            //  define as -1 to get a value close to the current match size 
#define PREFERRED_MOVE_MATCH_SIZE_INDEX 1 
// #define MIN_TABLE_SIZE 6
#define LARGE_TABLE_SIZE 14 //beyond, make row/col score labels smaller 

#define TABLE_SPACING 0 //should we show an "exploded" setting with separated cells, or a merged one (TABLE_SPACING=0)?
#define BORDER_SIZE 3 //size of border that has the full color corresponding to the decision
#define BORDER_PADDING 0 //should we padd the borders and show their inner workings?


#define GAUGE_SIXTH_SIZE 7 
#define GAUGE_SIZE (GAUGE_SIXTH_SIZE * 6+1)
#define GAUGE_BLUNDER (arSkillLevel[SKILL_VERYBAD]) // defines gauge scale (palette of colors in gauge)

/* Used in the "Colour by" radio buttons */
typedef enum _colourbasedonoptions { PT, DND, ALL} colourbasedonoptions;

/* Used in the "Label by" radio buttons */
typedef enum _labelbasedonoptions { LABEL_AWAY, LABEL_SCORE } labelbasedonoptions;

// used in DecisionVal
typedef enum _decisionval {ND, DT, DP, TG} decisionval;

/* Used in the "Describe moves using" radio buttons */
typedef enum _describeoptions { NUMBERS, ENGLISH, BOTH } describeoptions;

/* Array of possible strings for cube actions. */
char * SCOREMAP_DECISION_SHORT_STRING[4][4] = {
{"D/T","D/t","D/P", "D/p"},
{"d/T","d/t","d/P", "d/p"},
{"ND","ND","TGTD","TGTD"},
{"nd","nd","tgtd","tgtd"}};

/* Indices in SCOREMAP_DECISION_SHORT_STRING array */
#define SCOREMAP_D 0
#define SCOREMAP_D_WEAK 1
#define SCOREMAP_ND 2
#define SCOREMAP_ND_WEAK 3
#define SCOREMAP_T 0
#define SCOREMAP_T_WEAK 1
#define SCOREMAP_P 2
#define SCOREMAP_P_WEAK 3

/* Tolerance for using uppercase v lowercase letters in the cube action string: same as doubtful cutoff. */
#define SCOREMAP_STRONG_TOLERANCE (arSkillLevel[SKILL_DOUBTFUL]) 

// **** color definitions ****

// Used in colouring array (e.g., in ApplyColour())
#define RED 0
#define GREEN 1
#define BLUE 2
// we also code the 3 cube colors for ND/TGTD, DT, DP, respectively; and will use them in the 3 cube scoremap coloring options
static const float cubeColors[3][3] = {{1.0f,0.0f,0.0f}, {0.0f,1.0f,0.0f}, {0.0f,0.5f,1.0f}};

#define TOP_K 3 // in move scoremap, number of most frequent best-move decisions we want to keep, corresponding to number
                //      of distinct (non-grey/white) scoremap colors
// careful: need to code below at least the TOP_K corresponding RGB colors, as used in FindMoveColour() 
// converging on the same colors and same intensity as for the cube scoremap for now
static const float topKMoveColors[3][3] = {{1.0f,0.0f,0.0f}, {0.0f,1.0f,0.0f}, {0.0f,0.5f,1.0f}};
// static const float topKMoveColors[3][3] = {{0.0f,0.5f,1.0f}, {1.0f,0.5f,0.5f}, {0.5f,1.0f,0.0f}};
static const float white[3] = {1.0, 1.0, 1.0};  
// if a move is not in TOP_K, we color it in grey:
static const float grey[3] = {0.5, 0.5, 0.5};  //333
// **** end color definitions ****

typedef enum {NOT_TRUE_SCORE, TRUE_SCORE, LIKE_TRUE_SCORE} scorelike;
typedef enum {REGULAR, MONEY, DMP, GG, GS} specialscore;
typedef enum {ALLOWED, NO_CRAWFORD_DOUBLING, MISMATCHED_SCORES, UNREASONABLE_CUBE, UNALLOWED_DOUBLE} allowedscore;


/* Note that quadrantdata and gtkquadrant are decoupled, because the correspondence between gtk quadrants 
   and the quadrant data changes when toggling between label by away-scores and true score. */

typedef struct _quadrantdata {
/* Represents the data that is displayed in a single quadrant. */
    // 1. shared b/w cube and move scoremaps
    cubeinfo ci; // Used when computing the equity for this quadrant.
    char * decisionString; // String for correct decision: 2-letter for cube, more for move     
    scorelike isTrueScore;  // indicates whether equal to current score, same as current score, or neither
    specialscore isSpecialScore; // indicates whether MONEY, DMP, GG, GS, or regular 
    allowedscore isAllowedScore; // indicates whether the score is not allowed and the quadrant should be greyed
    gchar * unallowedExplanation; // if unallowed, explains why  

    // 2. specific to cube scoremap:
    float ndEquity; // Equity for no double
    float dtEquity; // Equity for double-take
    decisionval dec; // The correct decision

    // 3. specific to move scoremap:
    movelist ml; //scores ordered list of best moves
    char * classifiedDecisionString; // Best move "in English", e.g. "split", "run" etc.
} quadrantdata; //xxx

typedef struct _gtkquadrant { 
/* The GTK widgets that make up a single quadrant. */
    GtkWidget * pDrawingAreaWidget; 
    GtkWidget * pContainerWidget; 
    GtkWidget * pDrawingAreaWidgetBorder[4]; 
} gtkquadrant;

typedef struct { 
//in move scoremap, struct to hold decisions and their frequencies 
    char * decision;   
    int nDecisions;   
    int anMove[8];    
} weightedDecision;

typedef struct _scoremap {
// Main structure, holding nearly everything
    // 1. shared b/w cube and move scoremaps:    
    int cubeScoreMap; // whether we want a ScoreMap for cube or move (=1 for cube, =0 for move)
    const matchstate *pms; // The *true* match state 
    evalcontext ec; // The eval context (takes into account the selected ply)
    int tableSize; // This actually represents the max away score minus 1 (since we never show score=1)
    int matchSize; // Max away score
    int nCube; // The selected cube value
    int signednCube;    //same with sign that indicates what player owns the cube
    quadrantdata aaQuadrantData[MAX_TABLE_SIZE][MAX_TABLE_SIZE];
    quadrantdata moneyQuadrantData;  
    gtkquadrant aagQuadrant[MAX_TABLE_SIZE][MAX_TABLE_SIZE];
    gtkquadrant moneygQuadrant;   
    GtkWidget *apwRowLabel[MAX_TABLE_SIZE]; // Row and column score labels                                                         
    GtkWidget *apwColLabel[MAX_TABLE_SIZE];  
    GtkWidget *pwTable;
    GtkWidget *pwTableContainer;
    GtkWidget *pwCubeBox;    
    GtkWidget *pwCubeFrame;

    // 2. specific to cube scoremap:
    GtkWidget *apwFullGauge[GAUGE_SIZE];    // for cube scoremap
    GtkWidget *apwGauge[4];                 // for cube scoremap 
    
    // 3. specific to move scoremap:
    char * topKDecisions[TOP_K];            //top-k most frequent best-move decisions
    int topKDecisionsLength;                //b/w 0 and K
    char * topKClassifiedDecisions[TOP_K];  //top-k most frequent best-move decisions with "English" description
    // describeoptions describeUsing;       //whether to describe moves using move numbers and/or English ->replaced by static variable
} scoremap;

// ******************* DEFAULT STATIC OPTIONS ***********************
// We now define the options to retain betweenn scoremap instances, corresponding to the respective radio buttons
// Note: we do not define eval-ply and match length, as high values may cause increased complexity; rather we focus
//      on presentation settings (and the Jacoby option for the money quadrant)
static labelbasedonoptions labelBasedOn = LABEL_SCORE; 
static describeoptions describeUsing = BOTH;
static int moneyJacoby=0;
static colourbasedonoptions colourBasedOn = ALL; 

// *******************************************************************

#define MOVE_CLASSIFY FALSE   // Define whether we want to allow the "alpha version" move_classify 
                                    //  or want to stick with the safer "beta version" that doesn't   
#define MOVE_CLASSIFICATION_LABEL_SIZE (50) // delete line when including moveclassify.h     

static void
CubeValToggled(GtkWidget * pw, scoremap * psm);

void
MoveClassifyGenerateLabels(char *aLabel[], /*const*/ int ** aanMove, const TanBoard anBoard, const int size){}


int isAllowed (int i, int j, scoremap * psm, int signednCube) {
/*"isAllowed()" helps set the unallowed quadrants in move scoremap. Reasons to forbid
 a quadrant:
REASON 1. we don't allow i=0 i.e. 1-away Crawford with j=1 i.e. 1-away post-Crawford,
         and more generally incompatible scores in i and j
REASON 2. we don't allow doubling in a Crawford game
REASON 3. more tricky: we don't allow an absurd cubing situation, eg someone leading in Crawford
         with a >1 cube, or someone at matchLength-2 who would double to 4
*/
    int iAway=MAX(i,1); //if i==0 i.e. 1-away post-Crawford it should be 1
    int jAway=MAX(j,1); 
    // gchar * str; //=(gchar *) g_malloc(300);
    int p0Cube = 0;
    int p1Cube = 0;
    if (!psm->cubeScoreMap) {
        // REASON 1
        //there are 3 squares we don't allow: (0,1), (1,0) and (1,1)
        if ( (i==0 && j==1) || (i==1 && j==0) || (i==1 && j==1) ) {
            psm->aaQuadrantData[i][j].isAllowedScore = MISMATCHED_SCORES;
            // str = "This score is unallowed because one player is in Crawford while the other is in post-Crawford.";
            // psm->aaQuadrantData[i][j].unallowedExplanation = g_strdup(str);
            psm->aaQuadrantData[i][j].unallowedExplanation ="This score is unallowed because one player is in Crawford while the other is in post-Crawford.";
                    // char *s = "Global View";
                    // psm->aaQuadrantData[i][j].unallowedExplanation = strdup(s);
            return 0;
        } else if (abs(signednCube)>1 && (i==1 || j==1)) { 
            // REASON 2: no doubling in Crawford
            psm->aaQuadrantData[i][j].isAllowedScore = NO_CRAWFORD_DOUBLING;
            psm->aaQuadrantData[i][j].unallowedExplanation ="Doubling is not allowed in a Crawford game.";
            return 0;
        } else {            
            // REASON 3
            //if I have doubled and got signednCube=-4, ie my opponent owns a 4-cube => the previous cube was +2 
            //      => it cannot be that I am 1-away or 2-away from victory (or I wouldn't have had any reason to double); 
            //      and likewise my opponent cannot be 1-away from victory since he doubled to +2 cube before
            //      (& same when mirroring for my opponent => 2*2 sub-cases)
            // Note: signednCube > 1 when player with fMove holds double; but the table is organized by player 0/player 1
            //      => need to correct for that as well
            p0Cube = ((signednCube>1 && psm->pms->fMove == 1) || (signednCube<-1 && psm->pms->fMove != 1));
            p1Cube = (signednCube<-1 && psm->pms->fMove == 1) || (signednCube>1 && psm->pms->fMove != 1);            
            if (p0Cube && (abs(signednCube) >= 2*iAway || abs(signednCube) >= 4*jAway))   {
                // g_print("\n signednCube:%d, iAway:%d, jAway:%d",signednCube,iAway,jAway);  
                psm->aaQuadrantData[i][j].isAllowedScore = UNREASONABLE_CUBE;
                if (abs(signednCube) >= 2*iAway) {
                    psm->aaQuadrantData[i][j].unallowedExplanation =  
                                g_strdup_printf(_("It is unreasonable that %s will double to %d while being %d-away"), 
                                ap[0].szName, abs(signednCube), iAway);
                    //sprintf(str,"It is unreasonable that %s will double while being %d away",iAway);
                } else if (abs(signednCube) >= 4*jAway) { 
                    psm->aaQuadrantData[i][j].unallowedExplanation = 
                                g_strdup_printf(_("It is unreasonable that %s will have previously doubled to %d while being %d-away"), 
                                ap[1].szName, abs(signednCube)/2, jAway);            
                }
                // g_print("\n1:%s",str);
                // psm->aaQuadrantData[i][j].unallowedExplanation = g_strdup(str);
                // g_print("\n2:%s",psm->aaQuadrantData[i][j].unallowedExplanation);
                return 0;         
            } else if (p1Cube && (abs(signednCube) >= 2*jAway || abs(signednCube) >= 4*iAway)) {  
                psm->aaQuadrantData[i][j].isAllowedScore = UNREASONABLE_CUBE;  
                if (abs(signednCube) >= 2*jAway) {
                    psm->aaQuadrantData[i][j].unallowedExplanation =  
                                g_strdup_printf(_("It is unreasonable that %s will double to %d while being %d-away"), 
                                ap[1].szName, abs(signednCube), jAway);
                    //sprintf(str,"It is unreasonable that %s will double while being %d away",iAway);
                } else if (abs(signednCube) >= 4*iAway) { 
                    psm->aaQuadrantData[i][j].unallowedExplanation = 
                                g_strdup_printf(_("It is unreasonable that %s will have previously doubled to %d while being %d-away"), 
                                ap[0].szName, abs(signednCube)/2, iAway);            
                }          
                return 0;  
            } 
            // else {
            //     psm->aaQuadrantData[i][j].isAllowedScore = ALLOWED;         
            //     return 1; 
            // }  
        } 

    }         
    psm->aaQuadrantData[i][j].isAllowedScore = ALLOWED;       
    return 1;
}

static const decisionval
DecisionVal(float ndEq, float dtEq) {
/* Determines what the correct cube action is. */
    int i, j; // Use i for the D/ND decision, j for the P/T decision
    float dEq = MIN(dtEq, 1.0f); // Equity after double

    if ((ndEq < dEq)&&(dtEq < 1.0f))
        return DT; //D/T
    else if (ndEq < dEq)
        return DP; //D/P
    else if (dtEq < 1.0f) // ND
        return ND;
    else  // TGTD
        return TG;
}

static char *
DecisionShortString(float ndEq, float dtEq) {
/* Returns a short string representing the correct cube action (e.g., D/T for double, take). Uses lowercase letters 
   to indicate when the decision is borderline (equity difference less than "doubtful").
*/
    int i, j; // Use i for the D/ND decision, j for the P/T decision
    float dEq = MIN(dtEq, 1.0f); // Equity after double

    if (ndEq < dEq - SCOREMAP_STRONG_TOLERANCE) // Strong double
        i=SCOREMAP_D;
    else if (ndEq < dEq) // Weak double
        i=SCOREMAP_D_WEAK;
    else if (ndEq < dEq+SCOREMAP_STRONG_TOLERANCE) // Weak no double
        i=SCOREMAP_ND_WEAK;
    else // Strong no double
        i=SCOREMAP_ND;

    if (dtEq < 1.0f - SCOREMAP_STRONG_TOLERANCE) // Strong take
        j=SCOREMAP_T;
    else if (dtEq < 1.0f) // Weak take
        j=SCOREMAP_T_WEAK;
    else if (dtEq < 1.0f+SCOREMAP_STRONG_TOLERANCE) // Weak pass
        j=SCOREMAP_P_WEAK;
    else // Strong pass
        j=SCOREMAP_P;

    return SCOREMAP_DECISION_SHORT_STRING[i][j];
}

float
mmwc2eq(const float rMwc, const cubeinfo *pci) {
/* Converts mwc to equity, in case of a match. If a money game, no conversion takes place. */
    if (pci->nMatchTo==0)
        return rMwc;
    else
        return mwc2eq(rMwc,pci);
}

static int
CalcQuadrantEquities(quadrantdata * pq, const scoremap * psm, int recomputeFully) {
/* In Cube ScoreMap: Calculates the DT and ND equities for the given quadrant. Updates data in pq accordingly.
In Move ScoreMap: Calculates the ordered best moves and their equities.
*/
    if (psm->cubeScoreMap) {
        if (!GetDPEq(NULL, NULL, & pq->ci)) { // Cube not available
                pq->decisionString="";
                pq->isAllowedScore = UNALLOWED_DOUBLE;
                // gchar * str = "Unallowed double: cube not available.";
                pq->unallowedExplanation = "Unallowed double: cube not available.";
        } else {
            float ndEq, dtEq;
            float aarOutput[2][NUM_ROLLOUT_OUTPUTS];
            if (recomputeFully) {
                if (GeneralCubeDecisionE(aarOutput, psm->pms->anBoard, & pq->ci, & psm->ec, NULL)) { 
                        //GeneralCubeDecisionE is from eval.c
                        // extern int GeneralCubeDecisionE(float aarOutput[2][NUM_ROLLOUT_OUTPUTS], const TanBoard anBoard,
                        // cubeinfo * const pci, const evalcontext * pec, const evalsetup * UNUSED(pes))
                    ProgressEnd();
                    return -1;
                }    
    
                // Convert MWC to equity, and store in the scoremap
                pq->ndEquity= mmwc2eq(aarOutput[0][OUTPUT_CUBEFUL_EQUITY], & pq->ci);
                pq->dtEquity= mmwc2eq(aarOutput[1][OUTPUT_CUBEFUL_EQUITY], & pq->ci);
            }
            
    //g_print("Score: %d-%d ",psm->aaQuadrantData[i][j].ci.anScore[0], psm->aaQuadrantData[i][j].ci.anScore[1]);
            // Produce 2-letter string corresponding to the correct cube action (eg DT for double-take)
            pq->decisionString=DecisionShortString(pq->ndEquity,pq->dtEquity);
            pq->dec=DecisionVal(pq->ndEquity,pq->dtEquity);
    // g_print("ND %f; DT %f; %s\n",ndEq,dtEq,pq->decisionString);
        }
        return 0;
    } else {  //move scoremap
        if (FindnSaveBestMoves(&(pq->ml),psm->pms->anDice[0],psm->pms->anDice[1], (ConstTanBoard) psm->pms->anBoard, NULL, //or pkey
                                        arSkillLevel[SKILL_DOUBTFUL], &(pq->ci), &psm->ec, aamfAnalysis) <0) {
            pq->decisionString="";
            ProgressEnd();
            return -1;
        } 
        char szMove[FORMATEDMOVESIZE];
        FormatMove(szMove, (ConstTanBoard) psm->pms->anBoard, pq->ml.amMoves[0].anMove);
// g_print("%s\n",szMove);        
        char * buf=(char *)malloc(FORMATEDMOVESIZE+1); //strlen(szMove)+1);
        strcpy(buf,szMove);
        pq->decisionString=buf; //xxx

        // gchar *sz = g_strdup_printf("%s [%s]",
        //                                     GetEquityString(ptmw->atm[m].aarEquity[i][j],
        //                                                     &ci, ptmw->fInvert),
        //                                     FormatMove(szMove, (ConstTanBoard) ptmw->atm[m].pms->anBoard,
        //                                                ptmw->atm[m].aaanMove[i][j]));

// g_print("FindnSaveBestMoves returned %d, %d, %1.3f; decision: %s\n",pq->ml.cMoves,pq->ml.cMaxMoves, pq->ml.rBestScore,pq->decisionString);            
        return 0;  
    }
}

int 
CompareDecisionFrequencies (const void *a, const void *b)
{
/* qsort compare function */
    return (((weightedDecision *)a)->nDecisions < ((weightedDecision *)b)->nDecisions) - 
                (((weightedDecision *)a)->nDecisions > ((weightedDecision *)b)->nDecisions);

    // if (ndiff)
    //     return ndiff;

    // return strcmp (((words_t *)a)->word, ((words_t *)b)->word);
}

void
FindMostFrequentMoves(scoremap *psm) {
/* Find the top-k most frequent moves (in a move scoremap only!), and assign them colors.
If the move classification option is set, also assign them English descriptions.
*/
    int i,j;
    char sz[100];
    // char aux[100];    
    //1. copy scoremap decisions into an array structure called that contains decisions and their frequencies
    //      (using a new struct called weightedDecision)
    int auxLength = psm->tableSize * psm->tableSize+1;
    weightedDecision scoreMapDecisions[auxLength];// = (weightedDecision *) g_malloc(sizeof(...));;
    for (i=0; i<psm->tableSize; i++) {
        for (j=0; j<psm->tableSize; j++) {
            if (psm->aaQuadrantData[i][j].isAllowedScore == ALLOWED) {
                scoreMapDecisions[i*psm->tableSize+j].decision= g_strdup(psm->aaQuadrantData[i][j].decisionString);
                memcpy(scoreMapDecisions[i*psm->tableSize+j].anMove, psm->aaQuadrantData[i][j].ml.amMoves[0].anMove, sizeof(int)*8);
            }
            else {
                scoreMapDecisions[i*psm->tableSize+j].decision= "";
                scoreMapDecisions[i*psm->tableSize+j].nDecisions= 0;
            }
       }
    }
    scoreMapDecisions[auxLength-1].decision=psm->moneyQuadrantData.decisionString;

    // 2. find frequency of each best-move decision: each time we find a decision in the array, we look for all other 
    // occurrences of this decision and delete them (i.e. write decision="") while incrementing the move decision 
    // frequency counter. Therefore, at the end, we have several decisions that appear only once (and contain their 
    // frequencies) and the remainder is a bunch of "" decisions.
    // We assume that the anMove associated with a decision is always the same, and therefore do not look at it
    // for duplicate decisions.
    for (i=0; i<auxLength; i++) {
        if (strcmp(scoreMapDecisions[i].decision,"") != 0) { //non-null decision
            scoreMapDecisions[i].nDecisions=1;
            // g_print("at %d: decision %s\n",i,scoreMapDecisions[i].decision);
            for (j=i+1; j<auxLength; j++) {
                // g_print("\ncompared with dec@%d i.e. %s -> ",j,scoreMapDecisions[j].decision);
                if (strcmp(scoreMapDecisions[j].decision,scoreMapDecisions[i].decision) == 0){
                    // g_print("same!");
                    scoreMapDecisions[j].decision="";
                    scoreMapDecisions[j].nDecisions=0;
                    scoreMapDecisions[i].nDecisions++;
                }
            }
            // g_print("\nat %d: decision %s -> counter %d",i,scoreMapDecisions[i].decision,scoreMapDecisions[i].nDecisions);
        }
    }

    //3. find most frequent moves
    qsort (scoreMapDecisions, auxLength, sizeof (weightedDecision), CompareDecisionFrequencies);  /* sort words by frequency */
    // for (i=0; i<auxLength; i++) {
    //    g_print("\nafter qsort: at %d: decision %s counter %d ",i,scoreMapDecisions[i].decision,scoreMapDecisions[i].nDecisions);
    // }

    //4. record the first k (no need for counter anymore)
    psm->topKDecisionsLength=TOP_K; //we assume there are TOP_K valid distinct decisions unless we find otherwise
    for (i=0; (i<TOP_K && i<auxLength); i++) {
        if (strcmp(scoreMapDecisions[i].decision,"") != 0) { //non-null decision
            psm->topKDecisions[i]=g_strdup(scoreMapDecisions[i].decision);
        } else {
            psm->topKDecisionsLength=i;
            break;
        }
    }

    if (MOVE_CLASSIFY) {
        // We prepare the list of psm->topKDecisionsLength (i.e. up to TOP_K) moves, then apply the
        // MoveClassifyGenerateLabels() function
        char * aLabel[psm->topKDecisionsLength];
        int * aanMove[psm->topKDecisionsLength];

        for (int i=0; i<psm->topKDecisionsLength; i++) {
            aLabel[i]=g_malloc(MOVE_CLASSIFICATION_LABEL_SIZE*sizeof(char)); // MOVE_CLASSIFICATION_LABEL_SIZE defined in moveclassiy.h
            aanMove[i]=g_malloc(8*sizeof(int)); // size of move
            memcpy(aanMove[i], scoreMapDecisions[i].anMove, sizeof(int)*8);
        }
        //MoveClassifyGenerateLabels(char *aLabel[], int ** aanMove, const TanBoard anBoard, const int size)
        MoveClassifyGenerateLabels(aLabel, aanMove, (ConstTanBoard) psm->pms->anBoard, psm->topKDecisionsLength);

        for (int i=0; i<psm->topKDecisionsLength; i++) {
            FormatMove(sz, (ConstTanBoard) psm->pms->anBoard, aanMove[i]);
            psm->topKClassifiedDecisions[i] = g_malloc(100);
            //strcpy(psm->topKClassifiedDecisions[i],psm->topKDecisions[i]);
            //strcat(psm->topKClassifiedDecisions[i], ": ");
            strcpy(psm->topKClassifiedDecisions[i], aLabel[i]);
            // g_print("Move %s\n",psm->topKClassifiedDecisions[i]);
            // g_print("Move %s: %s\n",sz, aLabel[i]);
            g_free(aLabel[i]);       
        }
    } 
}


static int
CalcScoreMapEquities(scoremap * psm, int oldSize, int calcOnlyMoneyEquity)
/* Iterate through scores. Find equities at each score, and use these to set the text for the corresponding box.
   (Does not update the gui.)
   Only does entries in the table >= oldSize. (Avoid computing old values when resizing the table.)
   In the move scoremap, it also computes the most frequent best moves across the scoremap
*/
{
    ProgressStartValue(_("Finding correct actions"), psm->tableSize*psm->tableSize+1-oldSize*oldSize); 
            // - Should actually be 1 less if oldSize>0 (because money)
            // - If we only recompute the money equity, the formula correctly yields 1 for oldSize = psm->tableSize
//g_print("Finding %d-ply cube equities:\n",pec->nPlies);
    if (!calcOnlyMoneyEquity)
        for (int i=0; i<psm->tableSize; i++) {
            // i,j correspond to the locations in the table. E.g., in cube ScoreMap, the away-scores 
            //      are 2+i, 2+j (because the (0,0)-entry of the table corresponds to 2-away 2-away).
            for (int j=0; j<psm->tableSize; j++) {
                if (psm->aaQuadrantData[i][j].isAllowedScore == ALLOWED) {
                    CalcQuadrantEquities(& psm->aaQuadrantData[i][j], psm, (i>=oldSize || j>=oldSize));
                    // g_print("i=%d,j=%d,FindnSaveBestMoves returned %d, %1.3f; decision: %s\n",i,j,psm->aaQuadrantData[i][j].ml.
                    //         cMoves,psm->aaQuadrantData[i][j].ml.rBestScore,psm->aaQuadrantData[i][j].decisionString); 
                    if (i>=oldSize || j>=oldSize) // Only count the ones where equities are recomputed (other ones occur near-instantly)
                        ProgressValueAdd(1);
                } else {
                    psm->aaQuadrantData[i][j].decisionString="";
                }
            }
        }
    CalcQuadrantEquities(&(psm->moneyQuadrantData), psm, oldSize==0);
    ProgressValueAdd(1);

    ProgressEnd();

    if (!psm->cubeScoreMap)
        FindMostFrequentMoves(psm);

    return 0;
}

// rgbcolor 
// FloatToRgbcolor (float rgbvals[3]) {
//     /* this function translates the float array notation into an rgbcolor struct */
//     rgbcolor rgbAux;
//     rgbAux.rval=rgbvals[RED];
//     rgbAux.gval=rgbvals[GREEN];
//     rgbAux.bval=rgbvals[BLUE];
//     return rgbAux;
// }

void
ApplyColour(GtkWidget * pw, const float rgbvals[])
/* Colours the drawing area (pw) according to rgbvals.
*/
{
//g_print("%f %f %f\n", rgbvals[0],rgbvals[1],rgbvals[2]);
    
    /* First set the background colour to the given RGB values. (Convert from floats from 0 to 1 to integers from 0 to 255).
    */
    GtkStyle *ps = gtk_style_copy(gtk_widget_get_style(pw));
    
    ps->bg[GTK_STATE_NORMAL].red = (guint16) (rgbvals[RED] * 0xFFFF);  
    ps->bg[GTK_STATE_NORMAL].green = (guint16) (rgbvals[GREEN] * 0xFFFF);             
    ps->bg[GTK_STATE_NORMAL].blue =  (guint16) (rgbvals[BLUE] * 0xFFFF); 

    float * rval = g_malloc(sizeof(float));
    float * gval = g_malloc(sizeof(float));
    float * bval = g_malloc(sizeof(float));

    *rval = rgbvals[RED];
    *gval = rgbvals[GREEN];
    *bval = rgbvals[BLUE];

    /* Next store the colours in pw's association table. In GTK 3+, these values are read and "painted" on the
       quadrant to apply the colour. */
    g_object_set_data_full(G_OBJECT(pw), "rval", rval, g_free); //table of associations from strings to pointers.
    g_object_set_data_full(G_OBJECT(pw), "gval", gval, g_free); 
    g_object_set_data_full(G_OBJECT(pw), "bval", bval, g_free); 
    
    gtk_widget_set_style(pw, ps);
}

// //we decided to not use this function at the end and keep all fonts in black
// void
// UpdateFontColor(quadrantdata * pq, rgbcolor rgbBackground) 
// {
//     //we want to use a black font color on dark background colors, and white on dark
//     // the following formula computes the background luminnace for that
//     // it is taken from https://stackoverflow.com/questions/1855884/determine-font-color-based-on-background-color
//     float luminance = 0.299 * rgbBackground.rval + 0.587 * rgbBackground.gval + 0.114 * rgbBackground.bval; //luminance
//     // float luminance = 0.299 * rgbvals[0] + 0.587 * rgbvals[1] + 0.114 * rgbvals[2]; //luminance

//     strcpy(pq->rgbFont, (luminance>0.5) ? ("black") : ("white")); //black vs white //[test: ("red") : ("yellow"));]

//     // pgq->rgbFont.rval = pgq->rgbFont.gval = pgq->rgbFont.bval = (luminance>0.5) ? (0.0) : (1.0); //black vs white
//     // rgbFont[0]=pgq->rgbFont[1]=pgq->rgbFont[2]= (luminance>0.5) ? (0.0) : (1.0); //black vs white
//     // if (luminance > 0.5) 
//     //      = {0.0,0.0,0.0}; //black 
//     // else 
//     //     = {1.0,1.0,1.0}; //white  
// }

void
UpdateStyleGrey(gtkquadrant * pgq) 
{
    float rgbvals[3]={0.75,0.75,0.75};
    ApplyColour(pgq->pDrawingAreaWidget,rgbvals);
    for (int index=0; index<4; index++) { // apply strong color on all 4 borders
        ApplyColour(pgq->pDrawingAreaWidgetBorder[index], rgbvals);
    }
    //return FloatToRgbcolor(rgbvals);
}

float CalcIntensity(float r)  {
/* Transforms the value r (an equity difference) to an intensity to be used
   to determine the colour. This can be tweaked.
   Currently, anything considered "Very bad" is given full intensity, and linear in between.
*/
    float maxIntensity = arSkillLevel[SKILL_VERYBAD];
    return MAX(MIN(r/maxIntensity,1.0f),-1.0f); 
} 

void 
ColorInterpolate(float rgbResult[3], float x, const float rgbvals0[3], const float rgbvals1[3])
{
/*  fills rgbResult with a linearly-interpolated color at x \in [0,1] between color rgbvals0 and color rgbval1, such that 
 x=0 yields rgbvals0 and x=1 yields rgbvals1,
*/     
    x=MAX(0.0,MIN(1.0,x));
    for (int i=0; i<3; i++) {
        rgbResult[i] = (1.0f-x)*rgbvals0[i]+x*rgbvals1[i];
    }
}

void
FindColour1(float rgbvals[3], GtkWidget * pw, float r, colourbasedonoptions toggledColor)//ccc
/* Changes the colour according to the value r for the two binary gauges.
In addition, writes in rgbvals the desired border color for the quadrant.
*/
{
    r = CalcIntensity(r); // returns r in [-1,1] 
    int bestDecisionColor=-1;
    int colorIndex;

    // GtkStyle *ps = gtk_style_copy(gtk_widget_get_style(pw)); //used?   <-----------------------------
    // float rgbvals[3];

    if (toggledColor==DND) { //no double / double
        if (r<0) { //i.e. use red
            ColorInterpolate(rgbvals,-r,white,cubeColors[0]);
            bestDecisionColor=0;
            // rgbvals[RED]=1.0f;
            // rgbvals[BLUE]=rgbvals[GREEN]=MAX(1.0f+r,0.0f);
        } else { // green/cyan?
            ColorInterpolate(rgbvals,r,white,cubeColors[1]);   
            bestDecisionColor=1;     
            // rgbvals[GREEN]=1.0;
            // rgbvals[RED]=rgbvals[BLUE]=MAX(1.0f-r,0.0f);
        }
    } else { //Take/Pass
        if (r<0) { //green
            ColorInterpolate(rgbvals,-r,white,cubeColors[1]);        
            bestDecisionColor=1;     
            // rgbvals[GREEN]=1.0;
            // rgbvals[RED]=rgbvals[BLUE]=MAX(1.0f+r,0.0f);
        } else { // blue
            ColorInterpolate(rgbvals,r,white,cubeColors[2]);        
            bestDecisionColor=2;     
            // rgbvals[BLUE]=1.0;
            // rgbvals[GREEN]=MAX(1.0f-r,0.0f);
            // rgbvals[RED]=MAX((1.0f-r)/2.0f,0.0f); //avoiding purple tint
        }
    }
    //colors the quadrant in the desired (light) color
    ApplyColour(pw,rgbvals);
    //writes in rgbvals the desired (strong) quadrant border color
    if (bestDecisionColor>=0) {
        for (colorIndex=0; colorIndex<3; colorIndex++) {
            rgbvals[colorIndex]=cubeColors[bestDecisionColor][colorIndex];
        }
    } else // should cover all cases
    {
        g_assert_not_reached();
    }
    // return FloatToRgbcolor(rgbvals);
}



void 
FindColour2(float rgbvals[3], GtkWidget * pw, float nd, float dt) {
/* In cube colormap, changes the colour according to the nd and dt values, when the colouring is based on both decisions
Note: since the cube colormap also has a gauge that uses this function, we cannot set gtkquadrant * pgq as an input (since
a gauge doesn't have it), and therefore cannot color the border directly from here, since we only have a pointer to the 
main quadrant widget, not the borders around it.
As a result, the function writes in rgbvals the desired border color for the quadrant, and we use it after calling the function.
*/
    // initial idea: let color=color(opt) = red for ND, green for DT, blue for DP (corresponding to RGB...)
    //          and let final_r=sqrt(mistake)=sqrt(equity(opt)-equity(2nd best))=strength of color, using sqrt(x)
    //          rather than x to amplify small differences
    //          For example, full ND = red = (1,0,0) in RGB and DT = green = (0,1,0); so
    //          borderline ND/DT = (1,1,0) [or (0.5,0.5,0)] = yellow
    //          See https://www.freestylersupport.com/wiki/tutorial:sequences_ideas:rainbow_tutorial for an illustration.
    //
    // Version 2.0: a mix of colors looks bad/complicated (see alpha in commented example below: even a weak yellow doesn't work), 
    //          so let's have borderline ND/D = (1,1,1) = white, not yellow
    //          With full white: the color is independent of the direction (symmetric) => 3 cases, not 6
    //          Note that ND=TGTD in such a case, but we can use a different color if it helps
    // 
    // V 3.0: strong green and weak green look the same, trying different colors. Also the blue has a purple tint, correcting it.
    // 
    // V4.0: the sqrt was flattening differences. Replacing the sqrt by something like (error/maxError), this looks better 
    //          (see CalcIntensity()).

    float r=0.0f, dp=1.0f;
    int bestDecisionColor=-1;
    int colorIndex;
    //float rgbvals[3]={0.9, 0.9, 0.9}; //arbitrary initialization to grey
    //float alpha=0.05;

    if (nd >= MIN(dt,dp)) { //opt=nd or tgtd; symmetric => we don't distinguish cases
        r=CalcIntensity(nd-MIN(dt,dp)); //note that CalcIntensity(x>=0) is guaranteed to be in [0,1] 
        ColorInterpolate(rgbvals,r,white,cubeColors[0]);
        bestDecisionColor=0;     
        // rgbvals[RED]=1.0f;  
        // rgbvals[GREEN]=1.0f-r;      
        // rgbvals[BLUE]=1.0f-r;
    } else if (nd <= MIN(dt,dp) && dt <= dp) { //opt=dt
        r=CalcIntensity(MIN(dt-nd,dp-dt)); //r=1->green=(0,1,0), r=0->white=(1,1,1)
        ColorInterpolate(rgbvals,r,white,cubeColors[1]);
        bestDecisionColor=1;     
        // rgbvals[RED]=1.0f-r;  
        // rgbvals[GREEN]=1.0f;      
        // rgbvals[BLUE]=1.0f-r;
        // if it's not symmetric, we need to distinguish cases, like:
        // if(dt-nd<=dp-dt){ // i.e. nd is the closest 2nd option
        //     r=CalcIntensity(dt-nd); //r=1->green=(0,1,0), r=0->white=(1,1,1)
        //     rgbvals[RED]=1.0-r;  
        //     rgbvals[GREEN]=1.0;     
        //     rgbvals[BLUE]=1.0-r;
        // } else{ //(etc.)        
    } else if (nd <= MIN(dt,dp) && dp <= dt) { //opt=dp
        r=CalcIntensity(MIN(dp-nd,dt-dp)); 
        ColorInterpolate(rgbvals,r,white,cubeColors[2]);
        bestDecisionColor=2;     
        // rgbvals[RED]=(1.0f-r)/2.0f;  //<---- here the /2.0 to avoid a purple-ish color
        // rgbvals[GREEN]=1.0f-r;     
        // rgbvals[BLUE]=1.0f;
    } else // should cover all cases; (else black by default).
    {
        g_assert_not_reached();
    }
        
    //colors the quadrant in the desired (light) color
    ApplyColour(pw,rgbvals);
    //writes in rgbvals the desired (strong) quadrant border color
    if (bestDecisionColor>=0) {
        for (colorIndex=0; colorIndex<3; colorIndex++) {
            rgbvals[colorIndex]=cubeColors[bestDecisionColor][colorIndex];
        }
    } else // should cover all cases
    {
        g_assert_not_reached();
    }
    // return FloatToRgbcolor(rgbvals);
}

// float CalcMoveIntensity(float r)  {
// /* Transforms the value r (an equity difference) to an intensity to be used
//    to determine the colour. This can be tweaked.
//    Currently, anything considered "Very bad" is given full intensity, and linear in between.
// */
//     float maxIntensity = 0.050f; //arSkillLevel[SKILL_VERYBAD];
//     return MAX(MIN(r/maxIntensity,1.0f),0.0f); //fmaxf, fabsf
// } 

void  
FindMoveColour(gtkquadrant * pgq, quadrantdata * pq, const scoremap * psm, float r) { //)GtkWidget * pw, char * bestMove, const scoremap * psm, float r) { 
/*  This function colors the quadrants in the move scoremap.
It looks at the best move of each quadrant, and compares it with the top-k most frequent moves of the scoremap 
(or less if there are less than k); it then colors the quadrant according to TOP_K_COLORS.
For example, if the best move is the 3rd most frequent move, it looks at TOP_K_COLORS[3].
In addition, the function applies directly a desired strong border color TOP_K_COLORS[3] for the quadrant.
For instance, if TOP_K_COLORS[3]==(deep) blue, the border will be colored in deep blue, while the quadrant will be colored in light 
blue, where the blue intensity is proportional to the equity difference with the 2nd best move, i.e. the magnitude of the mistake.
Finally, the function also adds the decision classification that corresponds to the best move (and therefore to the 
corresponding color).
*/
    float rgbvals[] = {0.9,0.9,0.9}; //greyish if no match
    // rgbcolor rgbBackground; //rgbvals={0.9f, 0.9f, 0.9f};    
    int i,j; 
    // rgbvals[RED]=rgbvals[BLUE]=rgbvals[GREEN]=0.9f; //greyish if no match

    // for each decision, we look whether it's equal to a top color, and color it accordingly 
    // Note: it's better to pick bright colors (for the black font), and maybe colors 
    //      that are different from the cube scoremap colors
//  g_print("\n %f->%f",r,CalcIntensity(r));
    r = CalcIntensity(r); 
// g_print("\n-> r:%f....best move:%s",r, bestMove);    
    for (i=0; i<psm->topKDecisionsLength; i++) {
        if (strcmp(pq->decisionString,psm->topKDecisions[i]) ==0){ //there is a match //ccc
            ColorInterpolate(rgbvals,r,white,topKMoveColors[i]);
            ApplyColour(pgq->pDrawingAreaWidget, rgbvals); // apply intensity-modulated color on main quadrant
            for (int index=0; index<4; index++) { // apply strong color on all 4 borders
                ApplyColour(pgq->pDrawingAreaWidgetBorder[index], topKMoveColors[i]);
            }
            if (MOVE_CLASSIFY && describeUsing != NUMBERS) { // i.e. is ENGLISH or BOTH
                pq->classifiedDecisionString=g_malloc(200);
                if (describeUsing == ENGLISH) {
                    strcpy(pq->classifiedDecisionString,psm->topKClassifiedDecisions[i]);//xxx
                } else { //BOTH
                    strcpy(pq->classifiedDecisionString,psm->topKDecisions[i]);//xxx                
                    strcat(pq->classifiedDecisionString,": ");//xxx                
                    strcat(pq->classifiedDecisionString,psm->topKClassifiedDecisions[i]);//xxx                
                }
                // g_print("\n...%d: %s", i, psm->topKClassifiedDecisions[i]);
                // strcpy(pq->classifiedDecisionString,pq->decisionString);//->topKClassifiedDecisions[i]);//xxx
            }
            return; //break;
            // if (i==0) {
            //     rgbvals[RED]=1.0f-r;
            //     rgbvals[GREEN]=1.0f-r/2;
            //     rgbvals[BLUE]=1.0f;
            //     break;
            // }
            // else if (i==1) {
            //     rgbvals[RED]=1.0f;                
            //     rgbvals[GREEN]=1.0f-r;
            //     rgbvals[BLUE]=1.0f-r/2;
            //     break;                 
            // }
            // else if (i==2) {
            //     rgbvals[RED]=1.0f-r/2; 
            //     rgbvals[GREEN]=1.0f;
            //     rgbvals[BLUE]=1.0f-r;                
            //     break;                
            // }   
//  g_print("... i:%d",i);                   
        }
    }  
// if no match at all (no return): we pick grey for all remaining moves that are not in top-k
    ColorInterpolate(rgbvals,r,white,grey); //333
    ApplyColour(pgq->pDrawingAreaWidget, rgbvals); // apply intensity-modulated color on main quadrant
    for (int index=0; index<4; index++) { // apply strong color on all 4 borders
        ApplyColour(pgq->pDrawingAreaWidgetBorder[index], grey);
    }
    if (MOVE_CLASSIFY && describeUsing != NUMBERS) { // i.e. is ENGLISH or BOTH
        pq->classifiedDecisionString=g_malloc(200);
        // g_print("\n...no match: %s", pq->decisionString);
        strcpy(pq->classifiedDecisionString,pq->decisionString);//xxx
    }


    // ApplyColour(pgq->pDrawingAreaWidget, rgbvals);

    // rgbBackground.rval=rgbvals[RED];
    // rgbBackground.gval=rgbvals[GREEN];
    // rgbBackground.bval=rgbvals[BLUE];
    // return rgbBackground;
    // return FloatToRgbcolor(rgbvals);
} 

#define CalcRVal(toggle,nd,dt) (toggle==PT ? dt-1.0f : MIN(dt,1.0f)-nd)
                    //if yes, it's T vs P -> confusing, because the 2nd player wants the *lower* score =>r = -(dp-dt)
                    // if no, it's ND vs D -> r = dt-nd is natural, but we need to take TGTD into account: 
                    //     so 1st player doubles if the best option for player 2, namely min(dt,dp), is >nd


float 
Interpolate(float x, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3) 
{
    // float t;

    if (x <= x0) { 
        return y0; 
    } else if (x <= x1) { 
        return y0 + (x-x0)/(x1-x0)*(y1-y0); 
    } else if (x <= x2) { 
        return y1 + (x-x1)/(x2-x1)*(y2-y1); 
    } else if (x <= x3) { 
        return y2 + (x-x2)/(x3-x2)*(y3-y2); 
    } else
        return y3; 
}

void
DrawGauge(GtkWidget * pw, int i, colourbasedonoptions toggledColor) {

    float nd,dt,x;
    float maxDiff=GAUGE_BLUNDER; //e.g. SKILL_VERYBAD;
    float rgbvals[3]={0.75,0.75,0.75}; // init with grey


    //If we consider the "multi-color" choice of ND/DT/DP/TGTD, we call function FindColour2 that depends on a float array.
    // Else, if we want a simpler binary choice, we call UpdateStyle that only depends on a single float.
    if (toggledColor == ALL){
        // VERSION 1.0: getting a difference between the opt and the 2nd best of 1 at the peak
        // if (i<=GAUGE_THIRD_SIZE){
        //     nd=2.0-3.0*(float)i/(float)GAUGE_THIRD_SIZE;
        //     dt=1.0-(float)i/(float)GAUGE_THIRD_SIZE;
        // } else if (i<=2*GAUGE_THIRD_SIZE){
        //     nd=(float)i/(float)GAUGE_THIRD_SIZE-2.0;            
        //     dt=2.0*(float)i/(float)GAUGE_THIRD_SIZE-2.0;
        // }  else if (i<=3*GAUGE_THIRD_SIZE){
        //     nd=2.0*(float)i/(float)GAUGE_THIRD_SIZE-4.0;
        //     dt=4.0-(float)i/(float)GAUGE_THIRD_SIZE;
        // }
        // V 2.0: flatten the gauge differences: getting a difference between the opt and the 2nd best of maxDiff at the peak
        //      This enables linking it to SKILL_VERYBAD for instance.
        nd=Interpolate(0.5f*(float)i/(float)GAUGE_SIXTH_SIZE, 0.0f,1.0f+maxDiff,1.0f,1.0f-2.0f*maxDiff,2.0f,1.0f-maxDiff,3.0f,1.0f+maxDiff);
        dt=Interpolate(0.5f*(float)i/(float)GAUGE_SIXTH_SIZE, 0.0f,1.0f,1.0f,1.0f-maxDiff,2.0f,1.0f+maxDiff,3.0f,1.0f);
        FindColour2(rgbvals,pw, nd, dt);
    } else {
        x=GAUGE_BLUNDER*((float)i /(float)(GAUGE_SIXTH_SIZE*3)-1.0f); // we want to scale the gauge by the GAUGE_BLUNDER value
        FindColour1(rgbvals, pw, x, toggledColor);
    }
}

void
ColourQuadrant(gtkquadrant * pgq, quadrantdata * pq, const scoremap * psm) {
/* Colours the given quadrant and sets the hover text.
*/
    float nd=pq->ndEquity;
    float dt=pq->dtEquity;
    float dp=1.0f;
    float r;
    char ssz[200]; 
    char buf [300];
    int len0, len1, len2, len; 
    char space[50];        
    char szMove0[FORMATEDMOVESIZE], szMove1[FORMATEDMOVESIZE], szMove2[FORMATEDMOVESIZE];
    float rgbvals[3];
    // rgbcolor rgbBackground; 

    // color the squares      
    if (psm->cubeScoreMap && pq->isAllowedScore!=ALLOWED) { // Cube not available
        UpdateStyleGrey(pgq);
        // UpdateFontColor(pq, rgbBackground);
        gtk_widget_set_tooltip_text(pgq->pContainerWidget, pq->unallowedExplanation);  
    } else if (!psm->cubeScoreMap && pq->isAllowedScore!=ALLOWED) { //unallowed square, e.g. i says Crawford and j post-C.
        UpdateStyleGrey(pgq);
        // UpdateFontColor(pq, rgbBackground); 
// g_print("\n%s",pq->unallowedExplanation); 
//         const gchar * d=strdup(pq->unallowedExplanation);
        gtk_widget_set_tooltip_text(pgq->pContainerWidget, pq->unallowedExplanation); //    "Unallowed square: the scores of the two players are not compatible or the cube is not reasonable");  
    } else {
        // First compute and fill the color:
        if (psm->cubeScoreMap) {
            if (colourBasedOn != ALL) {
                r = CalcRVal(colourBasedOn, nd, dt);
                FindColour1(rgbvals,pgq->pDrawingAreaWidget, r,colourBasedOn);
                for (int index=0; index<4; index++) { // apply strong color on all 4 borders
                    ApplyColour(pgq->pDrawingAreaWidgetBorder[index], rgbvals);
                }                
                // UpdateFontColor(pq, rgbBackground); 
            } else {
                FindColour2(rgbvals,pgq->pDrawingAreaWidget,nd,dt);
                // g_print("\n%f",rgbvals[0]);
                for (int index=0; index<4; index++) { // apply strong color on all 4 borders
                    ApplyColour(pgq->pDrawingAreaWidgetBorder[index], rgbvals);
                }
                // UpdateFontColor(pq, rgbBackground); 
            }
        } else {
            //r is the equity difference b/w the top 2 moves, assuming they exist
            if (pq->ml.cMoves==0) //no moves
                r=0.0f;
            else
                r = (pq->ml.cMoves>1) ? (pq->ml.amMoves[0].rScore - pq->ml.amMoves[1].rScore) : (1.0f) ;  
            // FindMoveColour(pgq->pDrawingAreaWidget,pq->decisionString,psm,r);//pgq 
            FindMoveColour(pgq,pq,psm,r);//pgq  
            // FindMoveColour(pgq->pDrawingAreaWidgetBorder[0],pq->decisionString,psm,r);//pgq 
            // use black font on bright background, and white on dark
            // UpdateFontColor(pq, rgbBackground);
// g_print("%f",backgroundColor.bval);        
        }
        // next put the hover text; we first put a header text for special squares (e.g. same score as current match, or money), 
        //      then we put the equities for the 3 alternatives (ND, D/T, D/P).
        //      Note: we add one more space for "ND" b/c it has one less character than D/T, D/P so they are later aligned
        switch (pq->isTrueScore) {
            case TRUE_SCORE:
                // if (pq->ci.nMatchTo==0) 
                //     strcpy(buf,"<b>Money (as in current game): </b>\n");
                // else
                strcpy(buf,"<b>Current match score: </b>\n\n"); 
                break;
            case LIKE_TRUE_SCORE:
                strcpy(buf,"<b>Like current match score: </b>\n\n");
                break;            
            case NOT_TRUE_SCORE:
                strcpy(buf,"");
                break;
        }    
        switch (pq->isSpecialScore) {  //the square is special: MONEY, DMP etc. => write in hover text
            case MONEY:
                if (moneyJacoby)
                    strcat(buf,"<b>Money (Jacoby): </b>\n\n");
                else
                    strcat(buf,"<b>Money (no Jacoby): </b>\n\n");
                break;
            case DMP:    
                strcat(buf,"<b>DMP (double match point): </b>\n\n");       
                break;
            case GG:    
                strcat(buf,"<b>Typical GG (gammon-go): </b>\n\n");       
                break;
            case GS:    
                strcat(buf,"<b>Typical GS (gammon-save): </b>\n\n");        
                break;                
            case REGULAR:    
                strcat(buf,"");       
                break;                
        } 
        if (psm->cubeScoreMap) {
            if (pq->dec == ND) 
                sprintf(ssz,"<tt>1. ND \t%+.3f\n2. D/P\t%+.3f\t%+.3f\n3. D/T\t%+.3f\t%+.3f</tt>",nd,dp,dp-nd,dt,dt-nd);
            else if (pq->dec == DT)
                sprintf(ssz,"<tt>1. D/T\t%+.3f\n2. D/P\t%+.3f\t%+.3f\n3. ND \t%+.3f\t%+.3f</tt>",dt,dp,dp-dt,nd,nd-dt);
            else if (pq->dec == DP) 
                sprintf(ssz,"<tt>1. D/P\t%+.3f\n2. D/T\t%+.3f\t%+.3f\n3. ND \t%+.3f\t%+.3f</tt>",dp,dt,dt-dp,nd,nd-dt);
            else 
                sprintf(ssz,"<tt>1. ND \t%+.3f\n2. D/T\t%+.3f\t%+.3f\n3. D/P\t%+.3f\t%+.3f</tt>",nd,dt,dt-nd,dp,dp-nd); 
            strcat(buf,ssz);
        } else { // move scoremap //hhh
            // in the mouse hover window, we want to display the best move, then the 2nd and 3rd best moves and 
            //      their equity difference with the best move. We also introduce spacing to align the "2nd column"
            //      on the right for the equity differences of the 2nd and 3rd best moves.
            if (pq->ml.cMoves>0) { // best move
                FormatMove(szMove0, (ConstTanBoard) psm->pms->anBoard, pq->ml.amMoves[0].anMove);
            }
            if (pq->ml.cMoves>1) { // if exists, 2nd best move
                FormatMove(szMove1, (ConstTanBoard) psm->pms->anBoard, pq->ml.amMoves[1].anMove);               
            }
            if (pq->ml.cMoves>2) { // if exists, 3rd best move
                FormatMove(szMove2, (ConstTanBoard) psm->pms->anBoard, pq->ml.amMoves[2].anMove);
                // as in the bottom-right window, we want to display 3 columns: "move...moveEquity...equityDiffWithBestMove"
                // unfortunately, it is not easy to align the 3 columns: pango does not have a multi-alignment option,
                //      and cannot run a <table><tr><td> either
                // therefore, as we first want to align the column of the 3 equities (middle column), we find the length
                //      the length of the longest string in the first column and add a variable number of spaces to make the
                //      first column of the same length in all rows.
                // in addition, we use <tt> to make sure that all characters take the same amount of space ("sans" font was
                //      not enough")
                // finally, if some lines have a positive equity (".012") and others a negative ("-.012"), then the "-"
                //      breaks the alignment by 1 character, so we want to take it into account as well
                len0=strlen(szMove0)+(pq->ml.amMoves[0].rScore<-0.0005);//length, + 1 potentially b/c of the "-"
                len1=strlen(szMove1)+(pq->ml.amMoves[1].rScore<-0.0005);
                len2=strlen(szMove2)+(pq->ml.amMoves[2].rScore<-0.0005);
                len=MAX(len0,MAX(len1,len2))+2; //we leave an additional spacing of 2 characters, where 2 is arbitrary,
                                                    //  b/w the longest move string and its equity
                sprintf(space,"%*c", len-len0, ' ');   //define spacing for best move
                sprintf(ssz,"<tt>1. %s%s%1.3f",szMove0,space,pq->ml.amMoves[0].rScore);
                strcat(buf,ssz);                
                sprintf(space,"%*c", len-len1, ' ');   //define spacing for 2nd best move
                sprintf(ssz,"\n2. %s%s%1.3f  %1.3f",szMove1,space,pq->ml.amMoves[1].rScore,pq->ml.amMoves[1].rScore-pq->ml.amMoves[0].rScore);
                strcat(buf,ssz);      
                sprintf(space,"%*c", len-len2, ' ');   //define spacing for 3rd best move              
                sprintf(ssz,"\n3. %s%s%1.3f  %1.3f</tt>",szMove2,space,pq->ml.amMoves[2].rScore,pq->ml.amMoves[2].rScore-pq->ml.amMoves[0].rScore);
                strcat(buf,ssz);   
// g_print("moves:%s...%s... len: %d,%d,%d -> %s",szMove1,szMove2,len1,len2,len,buf)  ;                     
            }  else if (pq->ml.cMoves==2) { // there are only 2 moves
                len0=strlen(szMove0)+(pq->ml.amMoves[0].rScore<-0.0005);
                len1=strlen(szMove1)+(pq->ml.amMoves[1].rScore<-0.0005);            
                len=MAX(len0,len1)+2;
                sprintf(space,"%*c", len-len0, ' ');   //define spacing for best move
                sprintf(ssz,"<tt>1. %s%s%1.3f",szMove0,space,pq->ml.amMoves[0].rScore);
                strcat(buf,ssz);                
                sprintf(space,"%*c", len-len1, ' ');   //define spacing for 2nd best move
                sprintf(ssz,"2. \n%s%s%1.3f  %1.3f</tt>\n (only 2 moves)",szMove1,space,pq->ml.amMoves[1].rScore,pq->ml.amMoves[1].rScore-pq->ml.amMoves[0].rScore);
                strcat(buf,ssz);      
                // sprintf(ssz,"%s \t%1.3f  %1.3f</tt>\n (only 2 moves)",szMove1,pq->ml.amMoves[1].rScore,pq->ml.amMoves[1].rScore-pq->ml.amMoves[0].rScore);
                // strcat(buf,ssz);      
            } else if (pq->ml.cMoves==1) { //single move
                sprintf(ssz,"<tt>%s  %1.3f</tt>\n (unique move)",szMove0,pq->ml.amMoves[0].rScore);
                strcat(buf,ssz);        
                // sprintf(ssz,"</tt> (unique move)");
                // strcat(buf,ssz);  
            } else {
                sprintf(ssz,"no moves!");
                strcat(buf,ssz);                  
            }                               
            // if (pq->ml.cMoves>1) {
            //     FormatMove(szMove1, (ConstTanBoard) psm->pms->anBoard, pq->ml.amMoves[1].anMove);
            //     sprintf(ssz,"%s\t%1.3f\n",szMove1,pq->ml.amMoves[1].rScore-pq->ml.amMoves[0].rScore);
            //     strcat(buf,ssz);                
            // }
            // if (pq->ml.cMoves>2) {
            //     FormatMove(szMove2, (ConstTanBoard) psm->pms->anBoard, pq->ml.amMoves[2].anMove);
            //     sprintf(ssz,"%s\t%1.3f\n",szMove2,pq->ml.amMoves[2].rScore-pq->ml.amMoves[0].rScore);
            //     strcat(buf,ssz);                
            // }            
        }
        // sprintf(sz,"N/D +%.3f \nD/T +%.3f \nD/P +1.00",pq->ndEquity,pq->dtEquity);
        // sprintf(sz,"  %s \nN/D +%.3f \nD/T +%.3f \nD/P +1.00",pq->decisionString,pq->ndEquity,pq->dtEquity);
        // gtk_widget_set_tooltip_text(pq->pwe, pq->decisionString);

        gtk_widget_set_tooltip_markup(pgq->pContainerWidget, buf); 
    } 
    gtk_widget_queue_draw(pgq->pDrawingAreaWidget); // here we are calling DrawQuadrant() in a hidden way through the "draw" call,
                                                    //      as initially defined in InitQuadrant()
    for (int index=0; index<4; index++) { // draw all 4 borders
        gtk_widget_queue_draw(pgq->pDrawingAreaWidgetBorder[index]);
    }
//     cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(pgq->pDrawingAreaWidgetBorder[0])); //yyy
//     GtkWidget * pw=pgq->pDrawingAreaWidgetBorder[0];
//     GtkAllocation allocation;
//     #if GTK_CHECK_VERSION(3,0,0)
//     {
//         // Paint the drawing area with the appropriate colour
//         float *rval = g_object_get_data(G_OBJECT(pw), "rval");
//         float *gval = g_object_get_data(G_OBJECT(pw), "bval");
//         float *bval = g_object_get_data(G_OBJECT(pw), "gval");
//         guint width, height;

//         width = gtk_widget_get_allocated_width(pw);
//         height = gtk_widget_get_allocated_height(pw);

//         cairo_rectangle(cr, 0, 0, width, height);
//         cairo_set_source_rgb(cr, *rval, *gval, *bval);
//         cairo_fill(cr);

//         gtk_render_frame(gtk_widget_get_style_context(pw), cr, 0, 0, width, height);
//     }
// #else
//     (void) cr;                  /* silence compiler warning */
//     gtk_paint_box(gtk_widget_get_style(pw), gtk_widget_get_window(pw), GTK_STATE_NORMAL, GTK_SHADOW_IN, NULL, NULL, NULL, 0, 0, allocation.width, allocation.height);
// #endif 
//     cairo_destroy(cr);
}


void
UpdateScoreMapVisual(scoremap * psm)
/* Update all the visual components of the score map (called at the outset, or after one of the options changes):
Specifically: (1) The color of each square (2) The hover text of each square (3) the row/col score labels (4) the gauge.
*/
{
    int i, j, i2, j2;
    char szMove[100];


    // start by updating the score labels for each row/col and the color and hover text
    // i2, j2 = indices in the visual table (i.e., in aagQuadrant)
    // i,j = away-scores -2 (i.e., indices in aaQuadrantData)
    for (i2 = 0; i2 < psm->tableSize; ++i2) {                
        // if (psm->cubeScoreMap)
        i = (labelBasedOn == LABEL_AWAY) ? i2 : psm->tableSize-1-i2;
        // else //move scoremap
        //     i = (labelBasedOn == LABEL_AWAY) ? i2 : psm->tableSize-1-i2;
            
        
        /* Colour and apply hover text to all the quadrants */
        for (j2 = 0; j2 < psm->tableSize; ++j2) {
            j = (labelBasedOn == LABEL_AWAY) ? j2 : psm->tableSize-1-j2;
            ColourQuadrant(& psm->aagQuadrant[i2][j2], & psm->aaQuadrantData[i][j], psm);                   
        } // end of: for j2

        /* update score labels */
        // // PROBLEM: how to label when there are many columns and the labels become unreadable?
        // // SOL 1: put odd top labels on a new row
        // if (psm->tableSize < LARGE_TABLE_SIZE || i%2) {
        //     if (labelBasedOn == LABEL_AWAY)  
        //             sprintf(sz, _("%d-away"), i+2); 
        //     else
        //             sprintf(sz, _("%d/%d"), i2,psm->tableSize+1);
        //     gtk_label_set_text(GTK_LABEL(psm->apwRowLabel[i2]), sz);       
        //     gtk_label_set_text(GTK_LABEL(psm->apwColLabel[i2]), sz); 
        // } else {
        //     if (labelBasedOn == LABEL_AWAY) {
        //                 sprintf(sz, _("%d-away"), i+2);
        //                 sprintf(sz2, _("\n%d-away"), i+2);                         
        //     }    else  {
        //                 sprintf(sz, _("%d/%d"), i2,psm->tableSize+1);
        //                                     sprintf(sz2, _("\n%d/%d"), i2,psm->tableSize+1);
        //     }
        //     gtk_label_set_text(GTK_LABEL(psm->apwRowLabel[i2]), sz);       
        //     gtk_label_set_text(GTK_LABEL(psm->apwColLabel[i2]), sz2); 
        // }     
        // // SOL 2: smaller top labels 
        // if (psm->tableSize < LARGE_TABLE_SIZE) {
        //     if (labelBasedOn == LABEL_AWAY)  
        //             sprintf(sz, _("%d-away"), i+2); 
        //     else
        //             sprintf(sz, _("%d/%d"), i2,psm->tableSize+1);
        //     gtk_label_set_text(GTK_LABEL(psm->apwRowLabel[i2]), sz);       
        //     gtk_label_set_text(GTK_LABEL(psm->apwColLabel[i2]), sz); 
        // } else {
        //     // Pango.AttrList attrs = new Pango.AttrList (); 
        //     // attrs.insert (Pango.attr_scale_new (Pango.Scale.SMALL)); 
        //     // label.attributes = attrs;
        //     if (labelBasedOn == LABEL_AWAY) {
        //                 sprintf(sz, _("%d-away"), i+2);
        //                 sprintf(sz2, _("\n%d-away"), i+2);                         
        //     }    else  {
        //                 sprintf(sz, _("%d/%d"), i2,psm->tableSize+1);
        //                                     sprintf(sz2, _("\n%d/%d"), i2,psm->tableSize+1);
        //     }
        //     gtk_label_set_text(GTK_LABEL(psm->apwRowLabel[i2]), sz);       
        //     gtk_label_set_text(GTK_LABEL(psm->apwColLabel[i2]), sz2); 
        //     gtk_widget_modify_font((psm->apwColLabel[i2]), pango_font_description_from_string("sans 6"));
        // }   
        // // SOL 3: spacing and shorten text to just the number
        //if (psm->tableSize < LARGE_TABLE_SIZE) {

        // first we build short, long and hover text; then we assign them to the labels and label hover text
        // based on the table length    

        char sz[100]; 
        char longsz[100]; 
        char hoversz[100]={""}; 
        char empty[100]={""}; 
        if (psm->cubeScoreMap) {
            if (labelBasedOn == LABEL_AWAY)  {
                sprintf(sz, _("%d"), i+2);
                sprintf(longsz, _(" %d-away "), i+2);                 
            } else {
                sprintf(sz, _("%d"), i2);
                sprintf(longsz, _(" %d/%d "), i2,psm->tableSize+1);            
            }                   
        } else {//move scoremap
            if (labelBasedOn == LABEL_AWAY) { 
                if (i2==0) {
                    sprintf(sz, _("1pC")); 
                    sprintf(longsz, _(" 1-away post-C. "));
                    sprintf(hoversz, _(" 1-away post-Crawford "));
                } else if (i2==1) {
                    sprintf(sz, _("1C"));
                   sprintf(longsz, _(" 1-away C. "));
                    sprintf(hoversz, _(" 1-away Crawford "));                   
                } else {
                    sprintf(sz, _("%d"), i);     
                    sprintf(longsz, _(" %d-away "), i); 
                }                                                                                
            } else {
                if (i2<psm->tableSize-2) {
                    sprintf(sz, _("%d"), i2);
                    sprintf(longsz, _(" %d/%d "), i2,psm->matchSize);
                } else if (i2==psm->tableSize-2) {
                    sprintf(sz, _("%dC"), psm->matchSize-1);
                    sprintf(longsz, _(" %d/%d C. "), psm->matchSize-1,psm->matchSize);
                    sprintf(hoversz, _(" %d/%d Crawford "), psm->matchSize-1,psm->matchSize);
                } else {
                    sprintf(sz, _("%dpC"), psm->matchSize-1); 
                    sprintf(longsz, _(" %d/%d post-C. "), psm->matchSize-1,psm->matchSize); 
                    sprintf(hoversz, _(" %d/%d post-Crawford "), psm->matchSize-1,psm->matchSize);                     
                }        
            }
        }
        if (strcmp(hoversz,empty) == 0) //if we haven't already assigned something specific to the hover text, use the long text
            strcpy(hoversz,longsz);
// g_print("\n %s %s %s",sz,longsz,hoversz);

        // now that we build the labels and label hover text, we can assign them
        if (psm->tableSize >= LARGE_TABLE_SIZE) {
            gtk_label_set_text(GTK_LABEL(psm->apwRowLabel[i2]), sz);
            gtk_label_set_text(GTK_LABEL(psm->apwColLabel[i2]), sz); 
        } else {
            gtk_label_set_text(GTK_LABEL(psm->apwRowLabel[i2]), longsz);
            gtk_label_set_text(GTK_LABEL(psm->apwColLabel[i2]), longsz);             
        }
        gtk_widget_set_tooltip_markup(psm->apwRowLabel[i2], hoversz); 
        gtk_widget_set_tooltip_markup(psm->apwColLabel[i2], hoversz); //lll
         


        // } else {
        //     // Pango.AttrList attrs = new Pango.AttrList (); 
        //     // attrs.insert (Pango.attr_scale_new (Pango.Scale.SMALL)); 
        //     // label.attributes = attrs;
        //     if (labelBasedOn == LABEL_AWAY) {
        //                 sprintf(sz, _("%d-away"), i+2);
        //                 sprintf(sz2, _("\n%d-away"), i+2);                         
        //     }    else  {
        //                 sprintf(sz, _("%d/%d"), i2,psm->tableSize+1);
        //                                     sprintf(sz2, _("\n%d/%d"), i2,psm->tableSize+1);
        //     }
        //     gtk_label_set_text(GTK_LABEL(psm->apwRowLabel[i2]), sz);       
        //     gtk_label_set_text(GTK_LABEL(psm->apwColLabel[i2]), sz2); 
        //     gtk_widget_modify_font((psm->apwColLabel[i2]), pango_font_description_from_string("sans 6"));
        // }

    //         description = pango_font_description_from_string("sans");
    //         pango_font_description_set_size(description, MIN(MAX_FONT_SIZE*PANGO_SCALE,allocation.height * PANGO_SCALE / 25));
    //         layout = gtk_widget_create_pango_layout(psm->apwRowLabel[i2], NULL);
    //         pango_layout_set_font_description(layout, description);
    //             // x,y from top left of square from which to start writing
    // int x = 0.2f * allocation.width;//in my test: width=46, height=20
    // // y = 2 + (allocation.height - 4) / 10.0f;
    // int y = 0.2f * allocation.height;
    //         cairo_t * cr;
    //         pango_layout_set_markup(layout, sz,-1);      
    //             gtk_locdef_paint_layout(gtk_widget_get_style(psm->apwRowLabel[i2]), gtk_widget_get_window(psm->apwRowLabel[i2]), cr, GTK_STATE_NORMAL, TRUE, NULL, psm->apwRowLabel[i2], NULL, (int) x, (int) y, layout);

    // g_object_unref(layout); 
    } // end of: for i2
    ColourQuadrant(& psm->moneygQuadrant, & psm->moneyQuadrantData, psm);

    if  (psm->cubeScoreMap){
        /* update gauge */    
        for (i = 0; i < GAUGE_SIZE; i++) { 
            DrawGauge(psm->apwFullGauge[i],i,colourBasedOn);
        }
        /* update labels on gauge */    
        switch (colourBasedOn) {
            case PT:
                gtk_label_set_text(GTK_LABEL(psm->apwGauge[0]), "T");
                gtk_label_set_text(GTK_LABEL(psm->apwGauge[1]), "");
                gtk_label_set_text(GTK_LABEL(psm->apwGauge[2]), "");       
                gtk_label_set_text(GTK_LABEL(psm->apwGauge[3]), "P");
                gtk_widget_set_tooltip_markup(psm->apwGauge[0], "Take"); 
                gtk_widget_set_tooltip_markup(psm->apwGauge[1], ""); 
                gtk_widget_set_tooltip_markup(psm->apwGauge[2], ""); 
                gtk_widget_set_tooltip_markup(psm->apwGauge[3], "Pass");
                break;
            case DND:
                gtk_label_set_text(GTK_LABEL(psm->apwGauge[0]), "ND");
                gtk_label_set_text(GTK_LABEL(psm->apwGauge[1]), "");
                gtk_label_set_text(GTK_LABEL(psm->apwGauge[2]), "");       
                gtk_label_set_text(GTK_LABEL(psm->apwGauge[3]), "D");
                gtk_widget_set_tooltip_markup(psm->apwGauge[0], "No double"); 
                gtk_widget_set_tooltip_markup(psm->apwGauge[1], ""); 
                gtk_widget_set_tooltip_markup(psm->apwGauge[2], ""); 
                gtk_widget_set_tooltip_markup(psm->apwGauge[3], "Double");
                break;
            case ALL:
                gtk_label_set_text(GTK_LABEL(psm->apwGauge[0]), "ND");
                gtk_label_set_text(GTK_LABEL(psm->apwGauge[1]), "D/T");
                gtk_label_set_text(GTK_LABEL(psm->apwGauge[2]), "D/P");       
                gtk_label_set_text(GTK_LABEL(psm->apwGauge[3]), "TGTD");
                gtk_widget_set_tooltip_markup(psm->apwGauge[0], "No double/take"); 
                gtk_widget_set_tooltip_markup(psm->apwGauge[1], "Double/take"); 
                gtk_widget_set_tooltip_markup(psm->apwGauge[2], "Double/pass"); 
                gtk_widget_set_tooltip_markup(psm->apwGauge[3], "Too good to double");
                break;
            default:
                g_assert_not_reached();
        }
    }
}

// Aaron: TODO:continue cleaning up here


specialscore             
UpdateIsSpecialScore(const scoremap * psm, int i, int j, int money)
/* 
- In move ScoreMap, determines whether a quadrant corresponds to a special score like MONEY, DMP, GG, GS; or is just REGULAR.
Note: we only apply a strict definition for cube==1. We could widen the definition if it's too strict: e.g., 
if we are 3-away 4-away and cube=4, then it's practically DMP; etc.
 - In cube ScoreMap, only determines MONEY.
i,j indicate indices in aaQuadrantData.
*/
{
    if (money) //both in cube and move scoremaps
        return MONEY;
    else if (!psm->cubeScoreMap  && psm->nCube == 1) {//i.e. allowed square in move scoremap and the cube is centered 
                    // [we do not check if the square is allowed as there are corner cases
                    // where it's unallowed then allowed again because of a change in settings             
        if (i==0 && j==0)
            return DMP;
        else if ( (psm->pms->fMove && i==1 && j==2 ) || (!psm->pms->fMove && i==2 && j==1 )) //2-away 1-away Crawford, trailing
            return GG;
        else if ( (!psm->pms->fMove && i==1 && j==2 ) || (psm->pms->fMove && i==2 && j==1 )) //2-away 1-away Crawford, leading
            return GS;            
    }
    return REGULAR;
}


scorelike
UpdateIsTrueScore(const scoremap * psm, int i, int j, int money)
/* Determines whether a quadrant corresponds to the true score. 
Does not consider whether the cube value is the same as the true cube value.
*/
{
    int scoreLike;
    // we now want to check whether the i-away and j-away values of a quadrant correspond to the real away values of the game.
    // We distinguish the two cases: cube scoremap and move scoremap, since they display away scores in different quadrants
    
    // in cube scoremap, i,j indicate indices in aaQuadrantData, i.e., they are away-scores-2.
    if (psm->cubeScoreMap) 
        scoreLike=((i+2==psm->pms->nMatchTo-psm->pms->anScore[0]) && (j+2==psm->pms->nMatchTo-psm->pms->anScore[1])) ? 1 : 0;
    //in move scoremap, we first check that a quadrant is allowed. If so, we distinguish the cases: 2-away and more,
    // or Crawford, or post-Crawford. It is possible to compress the formulas, but in this way we see all cases
    else if (!psm->cubeScoreMap && (psm->aaQuadrantData[i][j].isAllowedScore == ALLOWED)) {
        if (psm->pms->nMatchTo - psm->pms->anScore[0] > 1 && psm->pms->nMatchTo - psm->pms->anScore[1] > 1) //no Crawford issue
            scoreLike=((i==psm->pms->nMatchTo-psm->pms->anScore[0]) && (j==psm->pms->nMatchTo-psm->pms->anScore[1])) ? 1 : 0;   
        else if (psm->pms->nMatchTo-psm->pms->anScore[0] == 1 && psm->pms->fPostCrawford==0) 
            scoreLike=((i==1) && (j==psm->pms->nMatchTo-psm->pms->anScore[1])) ? 1 : 0; //i=1 Crawford
        else if (psm->pms->nMatchTo-psm->pms->anScore[1] == 1 && psm->pms->fPostCrawford==0) 
            scoreLike=((j==1) && (i==psm->pms->nMatchTo-psm->pms->anScore[0])) ? 1 : 0; // j=1 Crawford
        else if (psm->pms->nMatchTo-psm->pms->anScore[0] == 1 && psm->pms->fPostCrawford==1) 
            scoreLike=((i==0) && (j==psm->pms->nMatchTo-psm->pms->anScore[1])) ? 1 : 0; // i=0 post-C
        else if (psm->pms->nMatchTo-psm->pms->anScore[0] == 1 && psm->pms->fPostCrawford==1) 
            scoreLike=((j==0) && (i==psm->pms->nMatchTo-psm->pms->anScore[0])) ? 1 : 0; // j=0 post-C                                    
    } else //square is not allowed
        scoreLike=0;

    //in addition, we want to take the cube value into account when deciding whether a quadrant is indeed "score-like"
    // we will require the cube situation (as shown at the bottom in the cube radio buttons) to be the same as in the true game
    if (scoreLike) {//we want to see if we need to cancel b/c the cube is not the same
        //if we are in a cube scoremap, we only check that the cube value is the same
        //if we are in a move scoremap, we also need to check that it's the same player who holds the cube        
        //      In such a case, we use the formula ams.fCubeOwner= (signednCube>0) ? (ams.fMove) : (1-ams.fMove);        
        // (an alternative approach would be to record the signednCube when we start the scoremap, and check that
        //      we still get the same, i.e. psm->initialSignednCube == psm->signednCube)
        if ((psm->cubeScoreMap && psm->nCube == psm->pms->nCube) ||
            (!(psm->cubeScoreMap) && (psm->nCube == psm->pms->nCube) && ((psm->pms->nCube == 1) || 
                                                (psm->signednCube >0 && psm->pms->fCubeOwner==psm->pms->fMove) ||
                                                (psm->signednCube <0 && psm->pms->fCubeOwner!=psm->pms->fMove)))) {
            //then it's OK, same cube value, keep scoreLike
        } else { //not the same cube value
            scoreLike =0;
        }
    }
    //            p0Cube = ((signednCube>1 && psm->pms->fMove == 1) || (signednCube<-1 && psm->pms->fMove != 1));
            // p1Cube = (signednCube<-1 && psm->pms->fMove == 1) || (signednCube>1 && psm->pms->fMove != 1);   

    if (money)
        return (psm->pms->nMatchTo==0) ? TRUE_SCORE : NOT_TRUE_SCORE;
    else if (scoreLike)          
        return (psm->pms->nMatchTo == psm->matchSize) ? TRUE_SCORE : LIKE_TRUE_SCORE;
    else 
        return NOT_TRUE_SCORE;
}


void UpdateCubeInfoArray(scoremap *psm, int signednCube, int updateMoneyOnly) 
/* Creates the cubeinfo array, based on the given cube value -- i.e., updates psm->aaQuadrantData[i][j].ci
We use updateMoneyOnly when a user toggles/untoggles the Jacoby option and we want to recompute the money 
    quadrant only, not the whole scoremap
Also, in the move scoremap, both players can double, so non-1 cube values could be negative to indicate
    the player who doubles (e.g. 2,-2,4,-4 etc.)
*/
{
    psm->nCube=abs(signednCube); // Store the cube value
    psm->signednCube=signednCube;// Store the signed cube value
    matchstate ams=(* psm->pms); // Make a copy of the "master" matchstate  [note: backgammon.h defines an extern ms => using a different name]
    ams.nMatchTo=psm->matchSize; // Set the match length
    ams.nCube=psm->nCube; // Set the cube value
    if (signednCube==1) // Cube value 1: centre the cube
        ams.fCubeOwner=-1; 
    else // Cube value > 1: Uncentre the cube. Ensure that the player on roll owns the cube (and thus can double)
        ams.fCubeOwner= (signednCube>0) ? (ams.fMove) : (1-ams.fMove); 
    
    if (!updateMoneyOnly) //update the whole scoremap in such a case
        for (int i=0; i<psm->tableSize; i++) {
            for (int j=0; j<psm->tableSize; j++) {
                if (isAllowed(i, j, psm, signednCube)) {
                        //"isAllowed()" helps set the unallowed quadrants in move scoremap. Three reasons to forbid
                        //  a quadrant:
                        // 1. we don't allow i=0 i.e. 1-away Crawford with j=1 i.e. 1-away post-Crawford,
                        //          and more generally incompatible scores in i and j
                        // 2 we don't allow doubling in a Crawford game
                        // 3 we don't allow an absurd cubing situation, eg someone leading in Crawford
                        //          with a >1 cube, or someone at matchLength-2 who would double to 4

                    /* cubeinfo */
                    /* NOTE: it is important to change the score and cube value in the match, and not in the 
                    cubeinfo, because the gammon values in cubeinfo need to be set correctly.
                    This is taken care of in GetMatchStateCubeInfo().
                    */
                    if (psm->cubeScoreMap) {
                        // Alter the match score: we want i,j to be away scores - 2 (note match length is tableSize+1)
                        ams.anScore[0]=psm->tableSize-i-1; 
                        ams.anScore[1]=psm->tableSize-j-1;
                    } else { //move ScoreMap
                        // Here we found that there may be a bug with the confusion b/w fCrawford and fPostCrawford
                        // Using fCrawford while we should use the other...
                        // We set ams.fCrawford=1 for the Crawford games and 0 for the post-Crawford games
                        ams.fCrawford=1; //Crawford by default, unless i==0 or j==0
                        // ams.fPostCrawford=0; //Crawford by default, unless i==0 or j==0
                        if (i==0) { //1-away post Crawford  
                            ams.anScore[0]=psm->matchSize-1;
                            ams.fCrawford=0;
                        } else  //i-away Crawford
                            ams.anScore[0]=psm->matchSize-i;
                        if (j==0) { //1-away post Crawford
                            ams.anScore[1]=psm->matchSize-1;
                            ams.fCrawford=0;
                        } else  //j-away
                            ams.anScore[1]=psm->matchSize-j;           
                    }
                    // // Create cube info using this data
                    //     psm->aaQuadrantData[i][j] = (scoremap *) g_malloc(sizeof(scoremap));
                    GetMatchStateCubeInfo(&(psm->aaQuadrantData[i][j].ci), & ams);

                    //we also want to update the "special" quadrants, i.e. those w/ the same score as currently,
                    // or DMP etc.
                    // note that if the cube changes, e.g. 1-away 1-away is not DMP => this depends on the cube value 
                    psm->aaQuadrantData[i][j].isTrueScore=UpdateIsTrueScore(psm, i, j, FALSE);
                    psm->aaQuadrantData[i][j].isSpecialScore=UpdateIsSpecialScore(psm, i, j, FALSE); 
                } else {
                    // we decide to arbitrarily mark all unallowed squares as not special in any way
                    psm->aaQuadrantData[i][j].isTrueScore=NOT_TRUE_SCORE;
                    psm->aaQuadrantData[i][j].isSpecialScore=REGULAR; //if the cube makes the position unallowed
                            // (eg I cannot double to 4 when 1-away...) we don't want to mark any squares as special
                }
            }
        }
    ams.nMatchTo=0;
    ams.fJacoby=moneyJacoby;
    GetMatchStateCubeInfo(&(psm->moneyQuadrantData.ci), & ams); 
    psm->moneyQuadrantData.isTrueScore=UpdateIsTrueScore(psm, 0, 0, TRUE);
    psm->moneyQuadrantData.isSpecialScore=UpdateIsSpecialScore(psm, 0, 0, TRUE);
    psm->moneyQuadrantData.isAllowedScore=ALLOWED;    
}

char *    
CutText(char * decisionString) {
/* This function cuts the decision string so as to display it in two rows whever it is too long
Specifically: when the decision string length is beyond some cutoff value, we look for the first space in 
    the 2nd half of its length to cut it.
*/
    char * pch;
    int cutoff = 10;
    int len=strlen(decisionString);
    char * str;
    char * aux;
    //char str[200];
    char * str2;
    //char aux[100];
    char * test = decisionString+(len+1)/2;

    if (len > cutoff) { //not considering empty or short strings
        // g_print ("\nLooking for a space character in \"%s ... %s\": ",decisionString,test);
        pch=strchr(decisionString+(len+1)/2,' '); //finds the 1st space in the 2nd half of the string  
        if (!pch) {
            return decisionString;
        } else {
            str=g_malloc(200);   
            aux=g_malloc(100); 
            memcpy(str, decisionString, pch-decisionString);// copies the 1st line, i.e. the text until the space
            str[pch-decisionString]= '\0';
            strcat(str,"\n");       // going to the 2nd line
            // g_print("->%s",str);
            memcpy(aux, pch, decisionString+len-pch);
            strcat(str,aux);        //adding the 2nd line
            str[len+1] = '\0';
            return str;
        }
    } else 
        return decisionString;

}

gboolean
DrawQuadrant(GtkWidget * pw, cairo_t * cr, scoremap * psm) {
/* This is called by gtk when the object is drawn (for certain GTK versions)
Alternatively, it is called from ExposeQuadrant, which is called by gtk.
(This is triggered at the end of UpdateScoreMapVisual.)
The function updates the decision text in each square.
*/
    int *pi = (int *) g_object_get_data(G_OBJECT(pw), "user_data");
    const quadrantdata * pq;    
    PangoLayout *layout;
    PangoFontDescription *description;
    int i,j, x,y;
    GtkAllocation allocation;
    gtk_widget_get_allocation(pw, &allocation);
    char buf[200];
    char aux[100]; //if we change the number, need to update CutText input as well
    float fontsize;

#if GTK_CHECK_VERSION(3,0,0)
    {
        // Paint the drawing area with the appropriate colour
        float *rval = g_object_get_data(G_OBJECT(pw), "rval");
        float *gval = g_object_get_data(G_OBJECT(pw), "bval");
        float *bval = g_object_get_data(G_OBJECT(pw), "gval");
        guint width, height;

        width = gtk_widget_get_allocated_width(pw);
        height = gtk_widget_get_allocated_height(pw);

        cairo_rectangle(cr, 0, 0, width, height);
        cairo_set_source_rgb(cr, *rval, *gval, *bval);
        cairo_fill(cr);

        gtk_render_frame(gtk_widget_get_style_context(pw), cr, 0, 0, width, height);
    }
#else
    (void) cr;                  /* silence compiler warning */
    gtk_paint_box(gtk_widget_get_style(pw), gtk_widget_get_window(pw), GTK_STATE_NORMAL, GTK_SHADOW_NONE, NULL, NULL, NULL, 0, 0, allocation.width, allocation.height);
    // gtk_paint_box(gtk_widget_get_style(pw), gtk_widget_get_window(pw), GTK_STATE_NORMAL, GTK_SHADOW_IN, NULL, NULL, NULL, 0, 0, allocation.width, allocation.height);
#endif
    if (pi == NULL)
        return TRUE;

    if (*pi >= 0) {
        i = (*pi) / MAX_TABLE_SIZE;
        j = (*pi) % MAX_TABLE_SIZE;
        // Replace by actual away score
        i = (labelBasedOn == LABEL_AWAY ? i : psm->tableSize-i-1);
        j = (labelBasedOn == LABEL_AWAY ? j : psm->tableSize-j-1);
        pq=&psm->aaQuadrantData[i][j];
    } else {
        pq=&psm->moneyQuadrantData;
    }

    description = pango_font_description_from_string("sans");
    fontsize=MAX(MIN_FONT_SIZE, MIN(MAX_FONT_SIZE, (float)allocation.height/7.0f)); // Set text height as function of the quadrant height
    pango_font_description_set_size(description, (gint)(fontsize*PANGO_SCALE));
    
    layout = gtk_widget_create_pango_layout(pw, NULL);
    pango_layout_set_font_description(layout, description);
    pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER); 

    // build the text that will be displayed in the quadrant 

    // we start by cutting long decision strings into two rows; this is only relevant to the move scoremap
    if (!psm->cubeScoreMap) {
        if (MOVE_CLASSIFY && describeUsing != NUMBERS) {
            sprintf(aux,"%s",CutText(pq->classifiedDecisionString)); //xxx
        } else { // NUMBERS or !MOVE_CLASSIFY
            sprintf(aux,"%s",CutText(pq->decisionString));
        }
        // strcpy(pq->decisionString,CutText(pq->decisionString));
                // g_print("  ->DrawQuadrant: %s",aux);
    } else {
        sprintf(aux,"%s",pq->decisionString);
    } 
    // g_print("->DrawQuadrant: %s",pq->decisionString);
    // sprintf(aux,"<span foreground=\"%s\">", pq->rgbFont); //color font in black or white
    strcpy(buf,"");
    if (pq->isTrueScore) { //emphasize the true-score square //ttt
        // if (*pi >= 0)
        //     g_print("\n *pi %d i %d j %d",(*pi), i,j);
        strcpy(buf,"<b>"); //was:         strcpy(buf,"<b><big>");
        if (pq->ci.nMatchTo==0)
            strcat(buf,"<span size=\"x-small\">(Money)</span>\n"); //"Money (current game):\n" 
        else if (pq->isTrueScore == TRUE_SCORE)
            strcat(buf,"<span size=\"x-small\">(Game score)</span>\n"); // or "(Same score)"
        else if (pq->isTrueScore == LIKE_TRUE_SCORE)
            strcat(buf,"<span size=\"x-small\">(Similar score)</span>\n"); // "like current score" or "like game score" are a bit long  
        strcat(buf,aux);//pq->decisionString);
        strcat(buf,"</b>");
        // pango_layout_set_markup(layout, buf, -1);
    } else if (pq->isSpecialScore != REGULAR) { //the square is special: MONEY, DMP etc. => write in text
        if (pq->isSpecialScore == MONEY)
            strcpy(buf,"Money\n"); //          
        else if (pq->isSpecialScore == DMP)
            strcpy(buf,"DMP\n");           
        else if (pq->isSpecialScore == GG)
            strcpy(buf,"GG\n");          
        else if (pq->isSpecialScore == GS)
            strcpy(buf,"GS\n");          
        strcat(buf,aux);//pq->decisionString);
    } else { //regular (not MONEY, DMP, GG, GS), not true score
        strcpy(buf,aux);//pq->decisionString);
    }
    //     pango_layout_set_markup(layout, buf, -1);
    // } else {
    //     pango_layout_set_text(layout, pq->decisionString, -1);
    // }    
    // strcat(buf,"</span>");    
    pango_layout_set_markup(layout, buf, -1);
    // g_print("\n ... buf: %s",buf);

    int width, height;
    pango_layout_get_size(layout, &width, &height); /* Find the size of the text */
    /* Note these sizes are PANGO_SCALE * number of pixels, whereas allocation.width/height are in pixels. */
    
    if (width/PANGO_SCALE > allocation.width-4 || height/PANGO_SCALE > allocation.height-4) { /* Check if the text is too wide (we'd like at least a 2-pixel border) */
        /* If so, rescale it to achieve a 2-pixel border. */
        /* question: why not do a while instead of the if?
            answer: The code immediately after sets the font size so that it does exactly fit. 
                    No need to keep looping to check again.*/

        float rescaleFactor=fminf((allocation.width-4.0f)*(float)PANGO_SCALE/((float)width),(allocation.height-4.0f)*(float)PANGO_SCALE/((float)height)); 
        pango_font_description_set_size(description, (gint)(fontsize*rescaleFactor*PANGO_SCALE));
        pango_layout_set_font_description(layout,description);
        
        /* Measure size again, so that centering works right. */
        pango_layout_get_size(layout, &width, &height);
    }

    
    /* Compute where the top-right corner should go, so that the text is centred. */
    x=(allocation.width - width/PANGO_SCALE)/2;
    y=(allocation.height - height/PANGO_SCALE)/2;

    gtk_locdef_paint_layout(gtk_widget_get_style(pw), gtk_widget_get_window(pw), cr, GTK_STATE_NORMAL, TRUE, NULL, pw, NULL, (int) x, (int) y, layout);

    g_object_unref(layout);

    return TRUE;
}


gboolean
DrawQuadrant2(GtkWidget * pw, cairo_t * cr, scoremap * psm) {
/* This is called by gtk when the object is drawn (for certain GTK versions)
Alternatively, it is called from ExposeQuadrant, which is called by gtk.
(This is triggered at the end of UpdateScoreMapVisual.)
The function updates the decision text in each square.
*/
    int *pi = (int *) g_object_get_data(G_OBJECT(pw), "user_data");
    const quadrantdata * pq;    
    PangoLayout *layout;
    PangoFontDescription *description;
    int i,j, x,y;
    GtkAllocation allocation;
    gtk_widget_get_allocation(pw, &allocation);
    float fontsize;

#if GTK_CHECK_VERSION(3,0,0)
    {
        // Paint the drawing area with the appropriate colour
        float *rval = g_object_get_data(G_OBJECT(pw), "rval");
        float *gval = g_object_get_data(G_OBJECT(pw), "bval");
        float *bval = g_object_get_data(G_OBJECT(pw), "gval");
        guint width, height;

        width = gtk_widget_get_allocated_width(pw);
        height = gtk_widget_get_allocated_height(pw);

        cairo_rectangle(cr, 0, 0, width, height);
        cairo_set_source_rgb(cr, *rval, *gval, *bval);
        cairo_fill(cr);

        gtk_render_frame(gtk_widget_get_style_context(pw), cr, 0, 0, width, height);
    }
#else
    (void) cr;                  /* silence compiler warning */
    gtk_paint_box(gtk_widget_get_style(pw), gtk_widget_get_window(pw), GTK_STATE_NORMAL, GTK_SHADOW_NONE, NULL, NULL, NULL, 0, 0, allocation.width, allocation.height);
#endif 
 
    return TRUE;
}

#if ! GTK_CHECK_VERSION(3,0,0)
static void
ExposeQuadrant(GtkWidget * pw, GdkEventExpose * UNUSED(pev), scoremap * psm)
/* Called by gtk. */
{
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(pw));
    DrawQuadrant(pw, cr, psm);
    cairo_destroy(cr);
}
#endif

#if ! GTK_CHECK_VERSION(3,0,0)
static void
ExposeQuadrant2(GtkWidget * pw, GdkEventExpose * UNUSED(pev), scoremap * psm)
/* Called by gtk. */
{
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(pw));
    DrawQuadrant2(pw, cr, psm);
    cairo_destroy(cr);
}
#endif


void 
InitQuadrant(gtkquadrant * pq, GtkWidget * ptable, scoremap * psm, const int sizeQuadrant, const int i, const int j, const int money)
/* Initialize the given gtk quadrant: create the gtk components, place them in the table, tell the quadrant where it is.
*/
{

    GtkWidget * pwv;
    GtkWidget * pwh;
    int index;

    // start by creating container in scoremap quadrant (square)
    pq->pContainerWidget = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(pq->pContainerWidget), FALSE);  
#if GTK_CHECK_VERSION(3,4,0) 
        gtk_grid_attach(GTK_GRID(ptable), pq->pContainerWidget, money ? 1 : i + 2, money ? 2 : 1, money ? 1 : j + 2, money ? 2 : 1); // not tested
#else
        gtk_table_attach_defaults(GTK_TABLE(ptable), pq->pContainerWidget, money ? 1 : i + 2, money ? 2 : i + 3, money ? 1 : j + 2, money ? 2 : j + 3);
#endif
    // gtk_container_set_border_width(GTK_CONTAINER(pq->pContainerWidget),2);

        // GdkColor red = {0, 0xffff, 0x0000, 0x0000};
        // gtk_widget_modify_bg(pq->pContainerWidget, GTK_STATE_NORMAL, &red);

    /* next, put the borders and the main quadrant in the container:
        |-------  border[0] -----------|
        |border[1]  main     border [2]|
        |-------  border[3] -----------|

    */
#if GTK_CHECK_VERSION(3,0,0)
        pwv = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
#else
        pwv = gtk_vbox_new(FALSE, 0);
#endif
    gtk_container_add(GTK_CONTAINER(pq->pContainerWidget), pwv);
    // gtk_container_set_border_width(GTK_CONTAINER(pwh), 2);

    pq->pDrawingAreaWidgetBorder[0] = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(pwv), pq->pDrawingAreaWidgetBorder[0], FALSE, FALSE, BORDER_PADDING);//0 
    // gtk_container_add(GTK_CONTAINER(pq->pContainerWidget), pq->pDrawingAreaWidget);
    gtk_widget_set_size_request(pq->pDrawingAreaWidgetBorder[0], BORDER_SIZE, BORDER_SIZE); //[the size does not seem to make a difference...]

#if GTK_CHECK_VERSION(3,0,0)
    pwh = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    pwh = gtk_hbox_new(FALSE, 0);//parameters: homogeneous; spacing
#endif     
    gtk_box_pack_start(GTK_BOX(pwv), pwh, TRUE, TRUE, 0);//parameters: expand, fill, padding

    pq->pDrawingAreaWidgetBorder[1] = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(pwh), pq->pDrawingAreaWidgetBorder[1], FALSE, FALSE, BORDER_PADDING);
    gtk_widget_set_size_request(pq->pDrawingAreaWidgetBorder[1], BORDER_SIZE, BORDER_SIZE); 

    //main
    pq->pDrawingAreaWidget = gtk_drawing_area_new();
     gtk_box_pack_start(GTK_BOX(pwh), pq->pDrawingAreaWidget, TRUE, TRUE, 0);//0  
    // gtk_container_add(GTK_CONTAINER(pq->pContainerWidget), pq->pDrawingAreaWidget);
    gtk_widget_set_size_request(pq->pDrawingAreaWidget, sizeQuadrant, sizeQuadrant); 

    pq->pDrawingAreaWidgetBorder[2] = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(pwh), pq->pDrawingAreaWidgetBorder[2], FALSE, FALSE, BORDER_PADDING);
    gtk_widget_set_size_request(pq->pDrawingAreaWidgetBorder[2], BORDER_SIZE, BORDER_SIZE); 

    pq->pDrawingAreaWidgetBorder[3] = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(pwv), pq->pDrawingAreaWidgetBorder[3], FALSE, FALSE, BORDER_PADDING);
    gtk_widget_set_size_request(pq->pDrawingAreaWidgetBorder[3], BORDER_SIZE, BORDER_SIZE); 
    
    // Set user_data to an encoding of which grid entry this is
    int * pi = (int *) g_malloc(sizeof(int));
    *pi = money ? -1 : i * MAX_TABLE_SIZE + j;
   
    g_object_set_data_full(G_OBJECT(pq->pDrawingAreaWidget), "user_data", pi, g_free); 
#if GTK_CHECK_VERSION(3,0,0)
        gtk_style_context_add_class(gtk_widget_get_style_context(pq->pDrawingAreaWidget), "gnubg-score-map-quadrant");
        g_signal_connect(G_OBJECT(pq->pDrawingAreaWidget), "draw", G_CALLBACK(DrawQuadrant), psm);
#else
        g_signal_connect(G_OBJECT(pq->pDrawingAreaWidget), "expose_event", G_CALLBACK(ExposeQuadrant), psm);
#endif

//     // int * piArr[4];
//     for (index=0; index<4; index++) {
//     // // Set user_data to an encoding of which grid entry this is
//     //     piArr[index] = (int *) g_malloc(sizeof(int));
//     //     *piArr[index] = money ? -1 : i * MAX_TABLE_SIZE + j; //zzz
   
//         g_object_set_data(G_OBJECT(pq->pDrawingAreaWidgetBorder[index]), "user_data", NULL); 
// #if GTK_CHECK_VERSION(3,0,0)
//             gtk_style_context_add_class(gtk_widget_get_style_context(pq->pDrawingAreaWidgetBorder[index]), "gnubg-score-map-quadrant");
//             g_signal_connect(G_OBJECT(pq->pDrawingAreaWidgetBorder[index]), "draw", G_CALLBACK(DrawQuadrant), psm);
// #else
//             g_signal_connect(G_OBJECT(pq->pDrawingAreaWidgetBorder[index]), "expose_event", G_CALLBACK(ExposeQuadrant), psm);
// #endif
//     }

    int * piArr[4];
    for (index=0; index<4; index++) {
    // Set user_data to an encoding of which grid entry this is
        piArr[index] = (int *) g_malloc(sizeof(int));
        *piArr[index] = money ? -1 : i * MAX_TABLE_SIZE + j;
   
        g_object_set_data_full(G_OBJECT(pq->pDrawingAreaWidgetBorder[index]), "user_data", piArr[index], g_free); 
#if GTK_CHECK_VERSION(3,0,0)
            gtk_style_context_add_class(gtk_widget_get_style_context(pq->pDrawingAreaWidgetBorder[index]), "gnubg-score-map-quadrant");
            g_signal_connect(G_OBJECT(pq->pDrawingAreaWidgetBorder[index]), "draw", G_CALLBACK(DrawQuadrant2), psm);
#else
            g_signal_connect(G_OBJECT(pq->pDrawingAreaWidgetBorder[index]), "expose_event", G_CALLBACK(ExposeQuadrant2), psm);
#endif
    }

   
    


    // gtk_box_set_homogeneous (GTK_BOX(pwh),FALSE);
}

void
BuildTableContainer(scoremap * psm)
/* Creates the gtk table.
A pointer to the table is put into psm->pwTable, and it is placed in the container psm->pwTableContainer (to be displayed).
*/
{
    int tableSize=psm->tableSize;
// g_print("within build: match size=%d, table size=%d\n",psm->matchSize,psm->tableSize); 
    int sizeQuadrant=MIN(MAX_SIZE_QUADRANT, MAX_TABLE_SCREENSIZE/tableSize);
    int i,j;
    GtkWidget * pw; 
 
    //    gtk_container_remove(GTK_CONTAINER(psm->pwTableContainer), psm->pwTable);
#if GTK_CHECK_VERSION(3,4,0)
        psm->pwTable = gtk_grid_new(tableSize+2, tableSize+2); // not tested!
#else
        psm->pwTable = gtk_table_new(tableSize+2, tableSize+2, ((psm->cubeScoreMap) ? FALSE : TRUE)); //last parameter: homogeneous
#endif

#if GTK_CHECK_VERSION(3,4,0) 
        gtk_grid_set_row_spacing (GTK_GRID(psm->pwTable), TABLE_SPACING); // not tested
        gtk_grid_set_col_spacing (GTK_GRID(psm->pwTable), TABLE_SPACING); // not tested
#else
        gtk_table_set_row_spacings (GTK_TABLE(psm->pwTable), TABLE_SPACING);
        gtk_table_set_col_spacings (GTK_TABLE(psm->pwTable), TABLE_SPACING);
#endif

    //    gtk_container_add(GTK_CONTAINER(psm->pwvcnt), psm->pwTable);
       
    //    gtk_container_set_border_width (GTK_CONTAINER(psm->pwvcnt),10);
        // gtk_box_pack_start(GTK_BOX(psm->pwv), psm->pwTable, FALSE, FALSE, 0);
        // gtk_widget_show_all(psm->pwv);

    /* drawing areas */ 
   
    for (i = 0; i < tableSize; ++i) {
        for (j = 0; j < tableSize; ++j)
        { 
            InitQuadrant(& psm->aagQuadrant[i][j], psm->pwTable, psm, sizeQuadrant, i, j, FALSE);
            psm->aaQuadrantData[i][j].decisionString="";
            psm->aaQuadrantData[i][j].classifiedDecisionString="";
        }
 
        /* score labels */
        pw = gtk_label_new("");
        psm->apwRowLabel[i]=pw; // remember it to update later
#if GTK_CHECK_VERSION(3,4,0) 
            gtk_grid_attach(GTK_GRID(psm->pwTable), pw, 1, 1, i + 2, 1); // not tested
#else
            gtk_table_attach(GTK_TABLE(psm->pwTable), pw, 1, 2, i + 2, i + 3, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0); //at the end: xpadding, ypadding; was 3,0
#endif
        pw = gtk_label_new("");
        psm->apwColLabel[i]=pw;            
#if GTK_CHECK_VERSION(3,4,0) 
            gtk_grid_attach(GTK_GRID(psm->pwTable), pw, i + 2, 1, 1, 1); // not tested
#else
            gtk_table_attach_defaults(GTK_TABLE(psm->pwTable), pw, i + 2, i + 3, 1, 2); 
#endif
    }

    InitQuadrant( & psm->moneygQuadrant, psm->pwTable, psm, sizeQuadrant, 0, 0, TRUE); 
    psm->moneyQuadrantData.decisionString="";
    //g_signal_connect(G_OBJECT(psm->pwTable), "draw", G_CALLBACK(DrawQuadrant), psm);

        // gtk_box_pack_start(GTK_BOX(psm->pwv), psm->pwTable, FALSE, FALSE, 0);
        // gtk_widget_show_all(psm->pwv);

    // Player labels: first top, then left
    gchar * sz;
    if (!psm->cubeScoreMap || psm->pms->fMove)
        sz=g_strdup_printf(_("%s"), ap[0].szName);
    else
        sz=g_strdup_printf(_("%s can double"), ap[0].szName);
        
    pw = gtk_label_new(sz);
    g_free(sz);
//        psm->apwPlayerTop=pw; // remember it to update later

    gtk_label_set_justify(GTK_LABEL(pw), GTK_JUSTIFY_CENTER);
    gtk_table_attach_defaults(GTK_TABLE(psm->pwTable), pw, 2, tableSize+2, 0, 1);

    if (!psm->cubeScoreMap || !psm->pms->fMove)
        sz=g_strdup_printf(_("%s"), ap[1].szName);
    else
        sz=g_strdup_printf(_("%s can double"), ap[1].szName);
    pw = gtk_label_new(sz);
    gtk_label_set_angle(GTK_LABEL(pw), 90);
    gtk_label_set_justify(GTK_LABEL(pw), GTK_JUSTIFY_CENTER);
    gtk_table_attach(GTK_TABLE(psm->pwTable), pw, 0, 1, 2, tableSize+2, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 3, 0); 


    gtk_box_pack_start(GTK_BOX(psm->pwTableContainer), psm->pwTable, TRUE, TRUE, 0);   
    // gtk_box_pack_start(GTK_BOX(psm->pwTableContainer), psm->pwTable, FALSE, FALSE, 0);
    gtk_widget_show_all(psm->pwTableContainer);

    g_free(sz);   
}

//bbb
void
BuildCubeFrame(scoremap * psm)
/* Creates and/or updates the frame with all the cube toggle buttons.
A pointer to the containing box is kept in psm->pwCubeBox.
*/
{

    int *pi;
    int i, j, m;
    int matchSizeIndex;
    char sz[100];     
    char szLabel[100]; 
    char current [20];
    GtkWidget *pwFrame;
    GtkWidget *pwv;
    GtkWidget *pw;
    GtkWidget *pwh;
    GtkWidget *pwx;
    GtkWidget *pwv2;    
    // GtkWidget *pwh2;    
    GtkWidget *pwLabel;
    GtkWidget *pwhtop;    
    GtkWidget *pwhbottom;


    /* Cube value */
    // pwFrame=gtk_frame_new(_("Cube value"));
    // gtk_box_pack_start(GTK_BOX(psm->pwCubeBox), pwFrame, FALSE, FALSE, 0);
   
#if GTK_CHECK_VERSION(3,0,0)
    psm->pwCubeBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
#else
    psm->pwCubeBox = gtk_hbox_new(FALSE, 8);
#endif     


    // in cube scoremap, we attach everything to the same row
    // while in move scoremap, we attach the "1" to an initial row,
    //      then the first set of cubes >=2 to the top right row  
    //      and the 2nd set of cubes >=2 to the bottom right row  
    //      To distinguish, we provide positive and negative numbers to the (internal) cube values 
    if (psm->cubeScoreMap) {    //========== cube scoremap

        for (i=1; i<= psm->matchSize-1; i=2*i) { 
            // Iterate i=allowed cube values. (Only powers of two, up to and including the max score in the table)
            // <----------- but if i=matchSize (theoretically), what cube decision is there to take? => let's set up to matchSize-1
            if (i==psm->pms->nCube) {
                strcpy(current," (current)");
            } else {
                strcpy(current,"");
            }
            sprintf(sz,"%d%s",i,current);            
            
            if (i==1)
                pw = pwx = gtk_radio_button_new_with_label(NULL, _(sz)); // (centered)"));
            else {

                pw = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pwx),sz);
            }
            gtk_box_pack_start(GTK_BOX(psm->pwCubeBox), pw, FALSE, FALSE, 0);
            pi = (int *)g_malloc(sizeof(int)); 
            *pi=i;
            g_object_set_data_full(G_OBJECT(pw), "user_data", pi, g_free);
            g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(CubeValToggled), psm); 
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pw), i==psm->pms->nCube);
        }

    } else {                    //========== move scoremap
                // // first find max allowed cube i.e roughly 2^ {\floor (\log_2 (matchSize))}
                // for (m=1; m<= psm->matchSize; m=2*m) { 
                //     maxCube = m;
                // };
        for (j=0; j<=1; j++) { 
            for (m=1; m<= 2*(psm->matchSize-1); m=2*m) { 
                // Iterate through allowed cube values. (Only powers of two)
                // Max value that makes sense in move scoremap: thoretically we can be (matchSize-1)-away and double
                // (e.g. 2-away in a match of 3, so we get a 4-cube) => we can theoretically get up to 2(matchSize-1)
                i = (j==0) ? (m) : (-m); // we create two sets of "i": positive for the top, negative for the bottom
                //    we exploit the formula:    psm->signednCube =  (pms->fCubeOwner==pms->fMove) ? (pms->nCube) : (-pms->nCube);
                if (m==psm->pms->nCube && (m==1 || ((i>1 && psm->pms->fMove==psm->pms->fCubeOwner) || 
                            (i<-1 && psm->pms->fMove!=psm->pms->fCubeOwner) ))) { 
                    strcpy(current," (current)"); 
                } else {
                    strcpy(current,"");
                }
                sprintf(sz,"%d%s",m,current);   //i.e. show abs(i)

                if (i==-1) {    //do nothing, not relevant  
                    continue;            
                } else if (i==1) {
                    
                    pw = pwx = gtk_radio_button_new_with_label(NULL, _(sz));
                    gtk_box_pack_start(GTK_BOX(psm->pwCubeBox), pw, FALSE, FALSE, 0);

                    // new sub-column (using vertical box) within the cube-value frame
#if GTK_CHECK_VERSION(3,0,0)
    pwv2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
#else
    pwv2 = gtk_vbox_new(FALSE, 6); //gtk_vbox_new (gboolean homogeneous, gint spacing);
#endif
                    gtk_box_pack_start(GTK_BOX(psm->pwCubeBox), pwv2, FALSE, FALSE, 0);
        
                    // 1st line of new sub-column
#if GTK_CHECK_VERSION(3,0,0)
    pwhtop = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
    pwhtop = gtk_hbox_new(FALSE, 24);
#endif
                    gtk_box_pack_start(GTK_BOX(pwv2), pwhtop, FALSE, FALSE, 0);

                    // below: 2 options: "X holds cube: 2" or "Y doubled to: 2"; 
                    // for now "Y doubled to: 2" sounds more intuitive
                    // sprintf(sz,"%s holds cube (%s doubled): ",((psm->pms->fMove) ? (ap[1].szName) : (ap[0].szName)),
                    //     ((psm->pms->fMove) ? (ap[0].szName) : (ap[1].szName)));     
                    sprintf(szLabel,"%s doubled to: ", ((psm->pms->fMove) ? (ap[0].szName) : (ap[1].szName)));          
                    pwLabel = gtk_label_new(_(szLabel));
#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_halign(pwLabel, GTK_ALIGN_START);
    gtk_widget_set_valign(pwLabel, GTK_ALIGN_LEFT);
#else
    gtk_misc_set_alignment(GTK_MISC(pwLabel), 0, 0);
#endif
                    gtk_box_pack_start(GTK_BOX(pwhtop), pwLabel, FALSE, FALSE, 0); 

                    //2nd line of new sub-column
#if GTK_CHECK_VERSION(3,0,0)
    pwhbottom = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
    pwhbottom = gtk_hbox_new(FALSE, 24);
#endif
                    gtk_box_pack_start(GTK_BOX(pwv2), pwhbottom, FALSE, FALSE, 0); 

                    sprintf(szLabel,"%s doubled to: ", ((!psm->pms->fMove) ? (ap[0].szName) : (ap[1].szName)));                 
                    pwLabel = gtk_label_new(_(szLabel));
#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_halign(pwLabel, GTK_ALIGN_START);
    gtk_widget_set_valign(pwLabel, GTK_ALIGN_LEFT);
#else
    gtk_misc_set_alignment(GTK_MISC(pwLabel), 0, 0);
#endif
                    gtk_box_pack_start(GTK_BOX(pwhbottom), pwLabel, FALSE, FALSE, 0);    
                } else { // m >=2, i=+/-m
                    //sprintf(sz,"%d",m); //i.e. show abs(i)
                    pw = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pwx),sz);
                    gtk_box_pack_start(GTK_BOX((i>0) ? (pwhtop) : (pwhbottom)), pw, FALSE, FALSE, 0);  
                }
                // we have built a radio button for each relevant i, now we build the attached functions   
                
                pi = (int *)g_malloc(sizeof(int)); 
                *pi=i;
                g_object_set_data_full(G_OBJECT(pw), "user_data", pi, g_free);
                g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(CubeValToggled), psm); 
                //move scoremap: check whether cube owner is the player who moves
                // if (psm->pms->signednCube == 1)
                //     gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pw), i==1);
                // else
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pw), (i==psm->signednCube));
                        // ((psm->pms->fCubeOwner==psm->pms->fMove) ? (i ==psm->pms->nCube) : (i == - psm->pms->nCube) ));  
            }
        }
    } 
    gtk_container_add(GTK_CONTAINER(psm->pwCubeFrame), psm->pwCubeBox);              
    // gtk_box_pack_start(GTK_BOX(psm->pwTableContainer), psm->pwTable, TRUE, TRUE, 0);   
        // gtk_container_remove(GTK_CONTAINER(psm->pwCubeFrame), psm->pwCubeBox); 
        //    gtk_box_pack_start(GTK_BOX(pwv), psm->pwCubeFrame, FALSE, FALSE, 0); 
    gtk_widget_show_all(psm->pwCubeFrame);
} //bbb

// void
// UpdateTableSize(scoremap * psm)
// /* Changes the table size when the window is already open.
// */
// {
//     gtk_container_remove(GTK_CONTAINER(psm->pwv), psm->pwTable);
//     BuildTableContainer(psm);
// }

static void
ScoreMapPlyToggled(GtkWidget * pw, scoremap * psm)
/* This is called by gtk when the user clicks on one of the ply radio buttons.
*/
{
    int *pi = (int *) g_object_get_data(G_OBJECT(pw), "user_data");

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pw))) {

        /* recalculate equities */

        psm->ec.nPlies = *pi;

        if (CalcScoreMapEquities(psm,0,FALSE))
            return;

        UpdateScoreMapVisual(psm);
    }

}

static void
DescribeUsingToggled(GtkWidget * pw, scoremap * psm)
/* This is called by gtk when the user clicks on one of the "describe moves using" radio buttons.
If end up in this function, we already assume that English descriptions are allowed and that we are in 
a move scoremap.
*/
{
    int *pi = (int *) g_object_get_data(G_OBJECT(pw), "user_data");
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pw))) {
    // if (describeUsing != gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pw))) {
        describeUsing=(describeoptions)(*pi);
        UpdateScoreMapVisual(psm); 
    }
}

static void
ColourByToggled(GtkWidget * pw, scoremap * psm)
/* This is called by gtk when the user clicks on one of the "colour based on" radio buttons.
*/
{
    int *pi = (int *) g_object_get_data(G_OBJECT(pw), "user_data");
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pw))) {
        colourBasedOn=(colourbasedonoptions)(*pi);
        UpdateScoreMapVisual(psm); // also updates gauge and gauge labels
    }
}
static void
LabelByToggled(GtkWidget * pw, scoremap * psm)
/* This is called by gtk when the user clicks on one of the "colour based on" radio buttons.
*/
{
    int *pi = (int *) g_object_get_data(G_OBJECT(pw), "label");
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pw))) {
        labelBasedOn=(labelbasedonoptions)(*pi);
        UpdateScoreMapVisual(psm);
    }
}

static void
JacobyToggled(GtkWidget * pw, scoremap * psm)
/* This is called by gtk when the user clicks on the Jacoby checkbox.
*/
{
    // int *pi = (int *) g_object_get_data(G_OBJECT(pw), "label");
    if (moneyJacoby != gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pw)) ) {
        moneyJacoby = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pw));
        UpdateCubeInfoArray(psm, psm->signednCube, TRUE);  //Apply the Jacoby option and update the money quadrant only
        if (CalcScoreMapEquities(psm,psm->tableSize,TRUE)) // Recalculate equities.
            return;
        UpdateScoreMapVisual(psm); // Update square colours. 
    }
}

static void
CubeValToggled(GtkWidget * pw, scoremap * psm) 
/* This is called by gtk when the user clicks on one of the "cube value" radio buttons.
*/
{
    int *pi = (int *) g_object_get_data(G_OBJECT(pw), "user_data");
    
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pw))) {
        UpdateCubeInfoArray(psm, *pi, FALSE);  // Change all the cubeinfos to use the selected signednCube value
        if (CalcScoreMapEquities(psm,0,FALSE)) // Recalculate all equities
            return;
        UpdateScoreMapVisual(psm); // Update square colours. (Also updates gauge and gauge labels)
    }
}//bbb

static void
MatchLengthToggled(GtkWidget * pw, scoremap * psm)
/* This is called by gtk when the user picks a new match length.  
*/
{
    //int *pi = (int *) g_object_get_data(G_OBJECT(pw), "user_data");
    int index = gtk_combo_box_get_active(GTK_COMBO_BOX(pw));
    int newMatchSize=MATCH_SIZE_OPTIONS[index];
    // int signednCube;
    if (psm->matchSize!=newMatchSize) {
        /* recalculate equities */
        psm->matchSize=newMatchSize;
        int oldTableSize=psm->tableSize; 
        psm->tableSize = (psm->cubeScoreMap)? psm->matchSize-1 : psm->matchSize+1; 
        if (abs(psm->signednCube) >= 2*newMatchSize) { //i.e. we set a big cube then decrease too much the match size
            psm->signednCube=psm->nCube=1;
        }
        // signednCube=psm->signednCube; //defining it in case destroying the cube radio button affects the value
        gtk_container_remove(GTK_CONTAINER(psm->pwTableContainer), psm->pwTable);
        gtk_container_remove(GTK_CONTAINER(psm->pwCubeFrame), psm->pwCubeBox);        
        BuildTableContainer(psm);
// g_print("within MatchLengthToggled: match size=%d, table size=%d\n",psm->matchSize,psm->tableSize); 
        // psm->signednCube=signednCube;         
        BuildCubeFrame(psm);
        UpdateCubeInfoArray(psm, psm->signednCube, FALSE);
        CalcScoreMapEquities(psm, oldTableSize,FALSE); 
        UpdateScoreMapVisual(psm);
    }
    //}

}

static void
DestroyDialog(gpointer p, GObject * UNUSED(obj))
/* Called by gtk when the score map window is closed.
Allows garbage collection.
*/
{
    scoremap *psm = (scoremap *) p;

    /* garbage collect */

    g_free(psm);
}

// movelist *
// PrintBestMoves(scoremap *psm){
//     movelist * pml;
//     positionkey * pkey; //unsigned int data[7];
//     cubeinfo ci;
//     int findRes;

//     matchstate ams=(* psm->pms); // Make a copy of the "master" matchstate  [note: backgammon.h defines an extern ms => using a different name]
//     // ams.fCubeOwner=-1; 
//     ams.nMatchTo=0;
//     ams.fJacoby=MONEY_JACOBY;
//     // GetMatchStateCubeInfo(&(psm->moneyQuadrantData.ci), & ams); 
//     GetMatchStateCubeInfo(&ci, psm->pms); 

//     pml = (movelist *) g_malloc(sizeof(movelist));
//     // GenerateMoves(pml, psm->pms->anBoard, 0,0, FALSE);                    
//     //GenerateMoves(pml, ams.anBoard, (int) ams.anDice[0], (int) ams.anDice[1], FALSE);
//     FindnSaveBestMoves(pml,ams.anDice[0],ams.anDice[1], (ConstTanBoard) ams.anBoard, NULL, //or pkey
//         0.0f, //arSkillLevel[SKILL_DOUBTFUL], 
//         &(psm->moneyQuadrantData.ci), &psm->ec, aamfAnalysis); 
//         // movelist: from eval.h: typedef struct {unsigned int cMoves;unsigned int cMaxMoves, cMaxPips;int iMoveBest;float rBestScore;
//         //                  move *amMoves;} movelist;
//         // move: from eval.h: typedef struct {int anMove[8]; positionkey key; unsigned int cMoves, cPips; float rScore, rScore2; 
//         //                  float arEvalMove[NUM_ROLLOUT_OUTPUTS]; float arEvalStdDev[NUM_ROLLOUT_OUTPUTS]; evalsetup esMove;
//         //                  CMark cmark; } move;

//     //     FindnSaveBestMoves(pml, int nDice0, int nDice1, const TanBoard anBoard, positionkey * keyMove, const
//     //                    float rThr, const cubeinfo * pci, const evalcontext * pec,
//     //                    movefilter aamf[MAX_FILTER_PLIES][MAX_FILTER_PLIES])
// // g_print("FindnSaveBestMoves returned %d, %d, %1.3f",pml->cMoves,pml->cMaxMoves, pml->rBestScore);
// //     move m=pml->amMoves->anMove;

// //     typedef struct _scoremap {

// //     const matchstate *pms; // The *true* match state 
// //     evalcontext ec; // The eval context (takes into account the selected ply)
// //     int tableSize; // This actually represents the max away score minus 1 (since we never show score=1)
// //     int nCube; // The selected cube value

// //     quadrantdata aaQuadrantData[MAX_TABLE_SIZE][MAX_TABLE_SIZE];
// //     quadrantdata moneyQuadrantData;
    
// //     gtkquadrant aagQuadrant[MAX_TABLE_SIZE][MAX_TABLE_SIZE];
// //     gtkquadrant moneygQuadrant;
    
// //     GtkWidget *apwRowLabel[MAX_TABLE_SIZE];   // Row and column score labels                                                         
// //     GtkWidget *apwColLabel[MAX_TABLE_SIZE];  
// //     GtkWidget *apwFullGauge[GAUGE_SIZE];
// //     GtkWidget *apwGauge[4];
// //     GtkWidget *pwTable;
// //     GtkWidget *pwTableContainer;

// //     colourbasedonoptions colourBasedOn; // whether to colour squares based on D/ND or T/P decision, or both
// // //    labelbasedonoptions labelBasedOn;   // whether to label by away score or true score
// // } scoremap;
//     return pml;
// }

extern 
void
GTKShowScoreMap(const matchstate * pms, int cube) //gchar * UNUSED(aszTitle[]), const int UNUSED(fInvert))
/* Opens the score map window.
Create the GUI; calculate equities and update colours; run the dialog window.
int cube==1 means we want a scoremap for the cube eval, ==0 for the move eval
*/
{

    scoremap *psm;
    int *pi;
    int i, j, m;
    int matchSizeIndex;

    GtkWidget *pwDialog;
    GtkWidget *pwGaugeTable = NULL;
    GtkWidget *pwFrame;
    GtkWidget *pwv;
    GtkWidget *pw;
    GtkWidget *pwh;
    GtkWidget *pwx = NULL;
    // GtkWidget *pwy = NULL;
    GtkWidget *pwv2;    
    GtkWidget *pwh2;    
    float nd,dt,dp=1.0;
    GtkWidget *pwTable2 = NULL;
    GtkWidget *pwLabel;
    char sz[100]; 
    GtkWidget *pwhtop;    
    GtkWidget *pwhbottom;
    GtkWidget *pwDescribeDefault = NULL;  //pointer to default describeUsing radio button so we can toggle it at the initialization  
    GtkWidget *pwLabelDefault = NULL;  //pointer to default labelBasedOn radio button so we can toggle it at the initialization  
    GtkWidget *pwColourDefault = NULL;  //pointer to default colourBasedOn radio button so we can toggle it at the initialization  


/* Layout 1: options on right side (LAYOUT_HORIZONTAL==TRUE)
-- pwh -----------------------------------------------------
| --- pwv ----------------- ---pwv (reused)--------------- |
| | psm->pwTableContainer | | -pwFrame (multiple times)- | |
| |                       | | | --pwv2 (mult. times)-- | | |
| |                       | | | | pw (options)       | | | |
| | pwGaugeTable          | | | ---------------------- | | |
| |                       | | -------------------------- | |
| ------------------------- ------------------------------ |
------------------------------------------------------------


Layout 2: options on bottom (LAYOUT_HORIZONTAL==FALSE)
--- pwv ---------------
| psm->pwv             |
| (contains the table) |
| pwGaugeTable         |
| - pwh (mult. times)- |
| | pw (options)     | |
| -------------------- |
------------------------
*/

    /* dialog */
    // making it non-modal, i.e. we can access the window below and move the Score Map window
    // pwDialog = GTKCreateDialog(_("Score Map - Eval at Different Scores"),
    //                            DT_INFO, NULL, DIALOG_FLAG_NONE, NULL, NULL);
    // pwDialog = GTKCreateDialog(_("Score Map - Eval at Different Scores"),
    //                            DT_INFO, NULL, DIALOG_FLAG_MODAL, NULL, NULL);
    pwDialog = GTKCreateDialog(_("Score Map - Eval at Different Scores"),
                               DT_INFO, NULL, DIALOG_FLAG_MINMAXBUTTONS, NULL, NULL); //| DIALOG_FLAG_NOTIDY

    psm = (scoremap *) g_malloc(sizeof(scoremap));

    psm->cubeScoreMap = cube;   // throughout this file: determines whether we want a cube scoremap or a move scoremap
    //colourBasedOn=ALL;     //default gauge; see also the option to set the starting gauge at the bottom
    // psm->describeUsing=DEFAULT_DESCRIPTION; //default description mode: NUMBERS, ENGLISH, BOTH -> moved to static variable
    psm->pms = pms;
    psm->ec.fCubeful=TRUE;
    psm->ec.nPlies=0;
    psm->ec.fUsePrune=TRUE; // FALSE;
    psm->ec.fDeterministic=TRUE;
    psm->ec.rNoise=0.0;
    // moneyJacoby=MONEY_JACOBY;
    psm->nCube=pms->nCube;
    if (pms->nCube==1) //need to define here to set the default cube radio button
        psm->signednCube=1;
    else {
        psm->signednCube =  (pms->fCubeOwner==pms->fMove) ? (pms->nCube) : (-pms->nCube);
    }

    // Find closest appropriate table size among the options
    // In the next expression, the ";" at the end means that it gets directly to finding the right index, and doesn't execute 
    //      anything in the loop
    // Since we restrict to the pre-defined options, there is no reason to define MIN/MAX TABLE SIZE

    // for (matchSizeIndex=0; matchSizeIndex<NUM_MATCH_SIZE_OPTIONS-1 && (MATCH_SIZE_OPTIONS[matchSizeIndex] < MIN_TABLE_SIZE || MATCH_SIZE_OPTIONS[matchSizeIndex] < pms->nMatchTo); ++matchSizeIndex); 
    int preferredIndex = (cube) ? (PREFERRED_CUBE_MATCH_SIZE_INDEX) : (PREFERRED_MOVE_MATCH_SIZE_INDEX); 
    if (preferredIndex >=0)
        matchSizeIndex = preferredIndex;
    else
        for (matchSizeIndex=0; matchSizeIndex<NUM_MATCH_SIZE_OPTIONS-1 && (MATCH_SIZE_OPTIONS[matchSizeIndex] < pms->nMatchTo); ++matchSizeIndex); 
    psm->matchSize = MATCH_SIZE_OPTIONS[matchSizeIndex];
    // if psm->cubeScoreMap, we show away score from 2-away to (psm->matchSize)-away
    // if not, we show away score at 1-away twice (w/ and post crawford), then 2-away through (psm->matchSize)-away
    psm->tableSize = (psm->cubeScoreMap)? psm->matchSize-1 : psm->matchSize+1; 
    // g_print("finally: match size=%d, table size=%d\n",psm->matchSize,psm->tableSize);    
#if GTK_CHECK_VERSION(3,0,0)
    psm->pwTableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
    psm->pwTableContainer = gtk_vbox_new(FALSE, 0); // paremeters: homogeneous, spacing
#endif 

    BuildTableContainer(psm);
// g_print("post-build: match size=%d, table size=%d\n",psm->matchSize,psm->tableSize);  

    /* gauge */ 
    if (psm->cubeScoreMap) {
        pwGaugeTable = gtk_table_new(2, GAUGE_SIZE, FALSE); 
                    //"gtk_table_new has been deprecated since version 3.4 and should not be used in newly-written code. Use gtk_grid_new()."
                    // [or gtk_box for a single line or column]
                    // .... but I got "undefined reference to `gtk_grid_new'", "undefined reference to `gtk_grid_attach'" when compiling

        for (i = 0; i < GAUGE_SIZE; i++) { 
            pw = gtk_drawing_area_new();
            gtk_widget_set_size_request(pw, 8, 20); // [*widget, width, height]
            gtk_table_attach_defaults(GTK_TABLE(pwGaugeTable), pw, i, i + 1, 1, 2);
            g_object_set_data(G_OBJECT(pw), "user_data", NULL);

    #if GTK_CHECK_VERSION(3,0,0)
            gtk_style_context_add_class(gtk_widget_get_style_context(pw), "gnubg-score-map-quadrant");
            g_signal_connect(G_OBJECT(pw), "draw", G_CALLBACK(DrawQuadrant), NULL);
    #else
            g_signal_connect(G_OBJECT(pw), "expose_event", G_CALLBACK(ExposeQuadrant), NULL);
    #endif //zzz
            DrawGauge(pw,i,ALL);
            psm->apwFullGauge[i]=pw;
        }
        for (i = 0; i < 4; ++i) {
            psm->apwGauge[i] = gtk_label_new("");
            gtk_table_attach_defaults(GTK_TABLE(pwGaugeTable), psm->apwGauge[i], GAUGE_SIXTH_SIZE * 2 * i, GAUGE_SIXTH_SIZE * 2 * i + 1, 0, 1);
        }
    }
// *************************************************************************************************


if (LAYOUT_HORIZONTAL) {
/* Toggle between the two possible layouts.
   Warning: excessive code duplicaton; this is meant to be temporary, until we settle on a preferred layout.
*/

    // horizontal box to put table+gauge on the left, buttons on the right
#if GTK_CHECK_VERSION(3,0,0)
    pwh = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
    pwh = gtk_hbox_new(FALSE, 4);
#endif
    gtk_container_set_border_width(GTK_CONTAINER(pwh), 6);
    gtk_container_add(GTK_CONTAINER(DialogArea(pwDialog, DA_MAIN)), pwh);

    /* left vbox: holds table and gauge */
#if GTK_CHECK_VERSION(3,0,0)
    pwv = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
#else
    pwv = gtk_vbox_new(FALSE, 6); //gtk_vbox_new (gboolean homogeneous, gint spacing);
#endif

    gtk_box_pack_start(GTK_BOX(pwh), pwv, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(pwv), psm->pwTableContainer, TRUE, TRUE, 0);   


#if GTK_CHECK_VERSION(3,0,0)
    gtk_box_pack_start(GTK_BOX(pwv), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 0);
#else
    gtk_box_pack_start(GTK_BOX(pwv), gtk_hseparator_new(), FALSE, FALSE, 0);
#endif


    /* gauge */ 
    if (psm->cubeScoreMap)
        gtk_box_pack_start(GTK_BOX(pwv), pwGaugeTable, FALSE, FALSE, 0);

     /* separator */

#if GTK_CHECK_VERSION(3,0,0)
    gtk_box_pack_start(GTK_BOX(pwh), gtk_separator_new(GTK_ORIENTATION_VERTICAL), FALSE, FALSE, 0);
#else
    gtk_box_pack_start(GTK_BOX(pwh), gtk_vseparator_new(), FALSE, FALSE, 0);
#endif

    // Vertical box to contain option frames
#if GTK_CHECK_VERSION(3,0,0)
    pwv = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
#else
    pwv = gtk_vbox_new(FALSE, 6); //gtk_vbox_new (gboolean homogeneous, gint spacing);
#endif
    gtk_box_pack_start(GTK_BOX(pwh), pwv, TRUE, TRUE, 0);

    /* eval ply */
#if GTK_CHECK_VERSION(3,0,0)
    pwh = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
    pwh = gtk_hbox_new(FALSE, 4);
#endif
    gtk_box_pack_start(GTK_BOX(pwv), pwh, FALSE, FALSE, 0);

    pwFrame=gtk_frame_new(_("Eval"));
    gtk_box_pack_start(GTK_BOX(pwv), pwFrame, FALSE, FALSE, 0);

#if GTK_CHECK_VERSION(3,0,0)
        pwv2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
#else
        pwv2 = gtk_vbox_new(FALSE, 4);
#endif
    gtk_container_add(GTK_CONTAINER(pwFrame), pwv2);
     
    for (i = 0; i < 5; ++i) {
        gchar *sz = g_strdup_printf(_("%d-ply"), i);
        if (i == 0)
            pw = pwx = gtk_radio_button_new_with_label(NULL, sz);
        else
            pw = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pwx), sz);
        g_free(sz);
        
        gtk_box_pack_start(GTK_BOX(pwv2), pw, FALSE, FALSE, 0);

        pi = (int *) g_malloc(sizeof(int));
        *pi = i;

        g_object_set_data_full(G_OBJECT(pw), "user_data", pi, g_free);

        g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(ScoreMapPlyToggled), psm);

    }
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pwx), TRUE);

   /* Colour by */
    pwFrame=gtk_frame_new(_("Colour by"));
    gtk_box_pack_start(GTK_BOX(pwv), pwFrame, FALSE, FALSE, 0);
    if (!psm->cubeScoreMap)
        gtk_widget_set_sensitive(pwFrame, FALSE);

#if GTK_CHECK_VERSION(3,0,0)
        pwv2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
#else
        pwv2 = gtk_vbox_new(FALSE, 4);
#endif
    gtk_container_add(GTK_CONTAINER(pwFrame), pwv2);
     

    pw = pwx = gtk_radio_button_new_with_label(NULL, _("All"));// was: ND - D/T - D/P - TGTD")); // First radio button
    gtk_box_pack_start(GTK_BOX(pwv2), pw, FALSE, FALSE, 0);
    pi = (int *)g_malloc(sizeof(int)); //It is freed by gtk (that's why we use "g_free" as one of the arguments to g_object_set_data_full).
    *pi=(int)ALL;
    g_object_set_data_full(G_OBJECT(pw), "user_data", pi, g_free);
    g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(ColourByToggled), psm); 
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pw), 1);

    pw =  gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pwx), _("ND vs D")); // Associate this to the other radio button
    gtk_box_pack_start(GTK_BOX(pwv2), pw, FALSE, FALSE, 0);
    pi = (int *)g_malloc(sizeof(int));
    *pi=(int)DND;
    g_object_set_data_full(G_OBJECT(pw), "user_data", pi, g_free);
    g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(ColourByToggled), psm);

    pw =  gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pwx), _("T vs P")); // Associate this to the other radio button
    gtk_box_pack_start(GTK_BOX(pwv2), pw, FALSE, FALSE, 0);
    pi = (int *)g_malloc(sizeof(int));
    *pi=(int)PT;
    g_object_set_data_full(G_OBJECT(pw), "user_data", pi, g_free);
    g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(ColourByToggled), psm); 

    /* Label by toggle */
    // This button offers the choice between the display of true scores and away scores.
    pwFrame=gtk_frame_new(_("Label by"));
    gtk_box_pack_start(GTK_BOX(pwv), pwFrame, FALSE, FALSE, 0);

#if GTK_CHECK_VERSION(3,0,0)
        pwv2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
#else
        pwv2 = gtk_vbox_new(FALSE, 4);
#endif
    gtk_container_add(GTK_CONTAINER(pwFrame), pwv2);
     
    pw = pwx = gtk_radio_button_new_with_label(NULL, _("True score")); 
    gtk_box_pack_start(GTK_BOX(pwv2), pw, FALSE, FALSE, 0);
    pi = (int *)g_malloc(sizeof(int));
    *pi=(int)LABEL_SCORE;
    g_object_set_data_full(G_OBJECT(pw), "label", pi, g_free);
    g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(LabelByToggled), psm);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pw), labelBasedOn==LABEL_SCORE);

    pw =  gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pwx), _("Away score"));
    gtk_box_pack_start(GTK_BOX(pwv2), pw, FALSE, FALSE, 0);
    pi = (int *)g_malloc(sizeof(int)); 
    *pi=(int)LABEL_AWAY;
    g_object_set_data_full(G_OBJECT(pw), "label", pi, g_free);
    g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(LabelByToggled), psm); 
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pw), labelBasedOn==LABEL_AWAY);

    /* Cube value */
    pwFrame=gtk_frame_new(_("Cube value"));
    gtk_box_pack_start(GTK_BOX(pwv), pwFrame, FALSE, FALSE, 0);
   
#if GTK_CHECK_VERSION(3,0,0)
        pwv2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
#else
        pwv2 = gtk_vbox_new(FALSE, 4);
#endif
    gtk_container_add(GTK_CONTAINER(pwFrame), pwv2);
     
    for (i=1; i<= psm->matchSize; i=2*i) { // Iterate i=allowed cube values. (Only powers of two, up to and including the max score in the table)
        if (i==1)
            pw = pwx = gtk_radio_button_new_with_label(NULL, _("1 (centered)"));
        else {
            char sz[20]; 
            sprintf(sz,"%d",i);
            pw = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pwx),sz);
        }
        gtk_box_pack_start(GTK_BOX(pwv2), pw, FALSE, FALSE, 0);
        pi = (int *)g_malloc(sizeof(int)); 
        *pi=i;
        g_object_set_data_full(G_OBJECT(pw), "user_data", pi, g_free);
        g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(CubeValToggled), psm); 
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pw), i==pms->nCube);
    }
 
    /* Match length */
    pwFrame=gtk_frame_new(_("Match length:"));
    gtk_box_pack_start(GTK_BOX(pwv), pwFrame, FALSE, FALSE, 0);
#if GTK_CHECK_VERSION(3,0,0)
        pwv2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
#else
        pwv2 = gtk_vbox_new(FALSE, 4);
#endif
    gtk_container_add(GTK_CONTAINER(pwFrame), pwv2);

#if GTK_CHECK_VERSION(3,0,0)
    pwh = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
#else
    pwh = gtk_hbox_new(FALSE, 8);
#endif
    // pw=gtk_label_new(_("Match length: "));
    // gtk_box_pack_start(GTK_BOX(pwv), pwh, FALSE, FALSE, 0);
    // gtk_box_pack_start(GTK_BOX(pwh), pw, FALSE, FALSE, 0);
   gtk_box_pack_start(GTK_BOX(pwv2), pwh, FALSE, FALSE, 0);

    pw= gtk_combo_box_text_new();
    gtk_box_pack_start(GTK_BOX(pwh), pw, FALSE, FALSE, 0);

    for (i=0; i<NUM_MATCH_SIZE_OPTIONS; i++) {
        char sz[20]; 
        sprintf(sz,"%d",MATCH_SIZE_OPTIONS[i]);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pw), _(sz));
    }    
    g_signal_connect(G_OBJECT(pw), "changed", G_CALLBACK(MatchLengthToggled), psm); 
    gtk_combo_box_set_active(GTK_COMBO_BOX(pw), matchSizeIndex); 

   /* Jacoby */
    pwFrame=gtk_frame_new(_("Jacoby"));
    gtk_box_pack_start(GTK_BOX(pwv), pwFrame, FALSE, FALSE, 0);
   
#if GTK_CHECK_VERSION(3,0,0)
        pwv2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
#else
        pwv2 = gtk_vbox_new(FALSE, 4);
#endif
    gtk_container_add(GTK_CONTAINER(pwFrame), pwv2);
     
    pw = gtk_check_button_new_with_label ("Jacoby"); 
    gtk_box_pack_start(GTK_BOX(pwv2), pw, FALSE, FALSE, 0);    
    g_signal_connect(G_OBJECT (pw), "toggled", G_CALLBACK(JacobyToggled), psm); 
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pw), moneyJacoby);
    gtk_widget_set_tooltip_text(pw, _("Toggle Jacoby option for money play"));



} else if (LAYOUT_VERTICAL1)//**************************************************************************
{
   // Vertical box to hold everything.
#if GTK_CHECK_VERSION(3,0,0)
    pwv = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
#else
    pwv = gtk_vbox_new(FALSE, 6); //gtk_vbox_new (gboolean homogeneous, gint spacing);
#endif
    gtk_container_add(GTK_CONTAINER(DialogArea(pwDialog, DA_MAIN)), pwv);
    
    gtk_box_pack_start(GTK_BOX(pwv), psm->pwTableContainer, TRUE, TRUE, 0);   //gtk_box_pack_start (GtkBox *box, GtkWidget *child, 
                                        // gboolean expand,  gboolean fill, guint padding);

    /* separator */

#if GTK_CHECK_VERSION(3,0,0)
    gtk_box_pack_start(GTK_BOX(pwv), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 0);
#else
    gtk_box_pack_start(GTK_BOX(pwv), gtk_hseparator_new(), FALSE, FALSE, 0);
#endif


    /* gauge */ 
    if (psm->cubeScoreMap)
        gtk_box_pack_start(GTK_BOX(pwv), pwGaugeTable, FALSE, FALSE, 0);

     /* separator */

#if GTK_CHECK_VERSION(3,0,0)
    gtk_box_pack_start(GTK_BOX(pwv), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 0);
#else
    gtk_box_pack_start(GTK_BOX(pwv), gtk_vseparator_new(), FALSE, FALSE, 0);
#endif

    /* eval ply */

#if GTK_CHECK_VERSION(3,0,0)
    pwh = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
    pwh = gtk_hbox_new(FALSE, 4);
#endif
    gtk_box_pack_start(GTK_BOX(pwv), pwh, FALSE, FALSE, 0);

    pw=gtk_label_new(_("Eval:  "));
    gtk_box_pack_start(GTK_BOX(pwh), pw, FALSE, FALSE, 0);

    for (i = 0; i < 5; ++i) {
        gchar *sz = g_strdup_printf(_("%d-ply"), i);
        if (i == 0)
            pw = pwx = gtk_radio_button_new_with_label(NULL, sz);
        else
            pw = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pwx), sz);
        g_free(sz);
        
        gtk_box_pack_start(GTK_BOX(pwh), pw, FALSE, FALSE, 0);

        pi = (int *) g_malloc(sizeof(int));
        *pi = i;

        g_object_set_data_full(G_OBJECT(pw), "user_data", pi, g_free);
        g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(ScoreMapPlyToggled), psm);
    }
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pwx), TRUE);

   /* Colour by */
#if GTK_CHECK_VERSION(3,0,0)
        pwh = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
        pwh = gtk_hbox_new(FALSE, 4);
#endif
    gtk_box_pack_start(GTK_BOX(pwv), pwh, FALSE, FALSE, 0);
    // if (!psm->cubeScoreMap)
    //     gtk_widget_set_sensitive(pwh, FALSE);

    pw=gtk_label_new(_("Colour by:  "));
    gtk_box_pack_start(GTK_BOX(pwh), pw, FALSE, FALSE, 0);

#if GTK_CHECK_VERSION(3,0,0)
        gtk_widget_set_halign(pw, GTK_ALIGN_START);
        gtk_widget_set_valign(pw, GTK_ALIGN_START);
#else
        gtk_misc_set_alignment(GTK_MISC(pw), 0, 0);
#endif

    pw = pwx = gtk_radio_button_new_with_label(NULL, _("All")); //ND - D/T - D/P - TGTD")); // First radio button
    gtk_box_pack_start(GTK_BOX(pwh), pw, FALSE, FALSE, 0);
    pi = (int *)g_malloc(sizeof(int)); //shouldn't we free it at some point? It is freed by gtk (that's why we use "g_free" as one of the arguments to g_object_set_data_full).
    *pi=(int)ALL;
    g_object_set_data_full(G_OBJECT(pw), "user_data", pi, g_free);
    g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(ColourByToggled), psm); 
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pw), 1);

    pw =  gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pwx), _("ND vs D")); // Associate this to the other radio button
    gtk_box_pack_start(GTK_BOX(pwh), pw, FALSE, FALSE, 0);
    pi = (int *)g_malloc(sizeof(int));
    *pi=(int)DND;
    g_object_set_data_full(G_OBJECT(pw), "user_data", pi, g_free);
    g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(ColourByToggled), psm);

    pw =  gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pwx), _("T vs P")); // Associate this to the other radio button
    gtk_box_pack_start(GTK_BOX(pwh), pw, FALSE, FALSE, 0);
    pi = (int *)g_malloc(sizeof(int));
    *pi=(int)PT;
    g_object_set_data_full(G_OBJECT(pw), "user_data", pi, g_free);
    g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(ColourByToggled), psm); 

    /* Label by toggle */
    // This button offers the choice between the display of true scores and away scores.
#if GTK_CHECK_VERSION(3,0,0)
        pwh = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
        pwh = gtk_hbox_new(FALSE, 4);
#endif
    gtk_box_pack_start(GTK_BOX(pwv), pwh, FALSE, FALSE, 0);

    pw=gtk_label_new(_("Label by: "));
    gtk_box_pack_start(GTK_BOX(pwh), pw, FALSE, FALSE, 0);
    
#if GTK_CHECK_VERSION(3,0,0)
        gtk_widget_set_halign(pw, GTK_ALIGN_END);
        gtk_widget_set_valign(pw, GTK_ALIGN_START);
#else
        gtk_misc_set_alignment(GTK_MISC(pw), 0, 0);
#endif

    pw = pwx = gtk_radio_button_new_with_label(NULL, _("True score")); 
    gtk_box_pack_start(GTK_BOX(pwh), pw, FALSE, FALSE, 0);
    pi = (int *)g_malloc(sizeof(int));
    *pi=(int)LABEL_SCORE;
    g_object_set_data_full(G_OBJECT(pw), "label", pi, g_free);
    g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(LabelByToggled), psm);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pw), labelBasedOn==LABEL_SCORE);

    pw =  gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pwx), _("Away score"));
    gtk_box_pack_start(GTK_BOX(pwh), pw, FALSE, FALSE, 0);
    pi = (int *)g_malloc(sizeof(int)); 
    *pi=(int)LABEL_AWAY;
    g_object_set_data_full(G_OBJECT(pw), "label", pi, g_free);
    g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(LabelByToggled), psm); 
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pw), labelBasedOn==LABEL_AWAY);

//     /* Match length */
//     pw=gtk_label_new(_("\t\tMatch length: "));
//     gtk_box_pack_start(GTK_BOX(pwh), pw, FALSE, FALSE, 0);
    
// #if GTK_CHECK_VERSION(3,0,0)
//         gtk_widget_set_halign(pw, GTK_ALIGN_END);
//         gtk_widget_set_valign(pw, GTK_ALIGN_START);
// #else
//         gtk_misc_set_alignment(GTK_MISC(pw), 0, 0);
// #endif

//     pw= gtk_combo_box_text_new();
//     gtk_box_pack_start(GTK_BOX(pwh), pw, FALSE, FALSE, 0);
//     for (i=0; i<NUM_MATCH_SIZE_OPTIONS; i++) {
//         char sz[20]; 
//         sprintf(sz,"%d",MATCH_SIZE_OPTIONS[i]);
//         gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pw), _(sz));
//     }    
//     g_signal_connect(G_OBJECT(pw), "changed", G_CALLBACK(MatchLengthToggled), psm); 
//     gtk_combo_box_set_active(GTK_COMBO_BOX(pw), matchSizeIndex); 

    /* Cube value */
#if GTK_CHECK_VERSION(3,0,0)
        pwh = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
        pwh = gtk_hbox_new(FALSE, 4);
#endif
    gtk_box_pack_start(GTK_BOX(pwv), pwh, FALSE, FALSE, 0);

    pw=gtk_label_new(_("Cube value: "));
    gtk_box_pack_start(GTK_BOX(pwh), pw, FALSE, FALSE, 0);
    
#if GTK_CHECK_VERSION(3,0,0)
        gtk_widget_set_halign(pw, GTK_ALIGN_END);
        gtk_widget_set_valign(pw, GTK_ALIGN_START);
#else
        gtk_misc_set_alignment(GTK_MISC(pw), 0, 0);
#endif
    
    for (i=1; i<= psm->matchSize; i=2*i) { // Iterate i=allowed cube values. (Only powers of two, up to and including the max score in the table)
        if (i==1)
            pw = pwx = gtk_radio_button_new_with_label(NULL, _("1 (centered)"));
        else {
            char sz[20]; 
            sprintf(sz,"%d",i);
            pw = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pwx),sz);
        }
        gtk_box_pack_start(GTK_BOX(pwh), pw, FALSE, FALSE, 0);
        pi = (int *)g_malloc(sizeof(int)); 
        *pi=i;
        g_object_set_data_full(G_OBJECT(pw), "user_data", pi, g_free);
        g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(CubeValToggled), psm); 
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pw), i==pms->nCube);
    }

    /* Match length */
#if GTK_CHECK_VERSION(3,0,0)
        pwh = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
        pwh = gtk_hbox_new(FALSE, 4);
#endif
    gtk_box_pack_start(GTK_BOX(pwv), pwh, FALSE, FALSE, 0);    
    pw=gtk_label_new(_("Match length: "));
    gtk_box_pack_start(GTK_BOX(pwh), pw, FALSE, FALSE, 0);
    
#if GTK_CHECK_VERSION(3,0,0)
        gtk_widget_set_halign(pw, GTK_ALIGN_END);
        gtk_widget_set_valign(pw, GTK_ALIGN_START);
#else
        gtk_misc_set_alignment(GTK_MISC(pw), 0, 0);
#endif

    pw= gtk_combo_box_text_new();
    gtk_box_pack_start(GTK_BOX(pwh), pw, FALSE, FALSE, 0);
    for (i=0; i<NUM_MATCH_SIZE_OPTIONS; i++) {
        char sz[20]; 
        sprintf(sz,"%d",MATCH_SIZE_OPTIONS[i]);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pw), _(sz));
    }    
    g_signal_connect(G_OBJECT(pw), "changed", G_CALLBACK(MatchLengthToggled), psm); 
    gtk_combo_box_set_active(GTK_COMBO_BOX(pw), matchSizeIndex); 
 
    /* Jacoby */
#if GTK_CHECK_VERSION(3,0,0)
        pwh = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
        pwh = gtk_hbox_new(FALSE, 4);
#endif
    gtk_box_pack_start(GTK_BOX(pwv), pwh, FALSE, FALSE, 0);

    pw=gtk_label_new(_("Jacoby: "));
    gtk_box_pack_start(GTK_BOX(pwh), pw, FALSE, FALSE, 0);
    
#if GTK_CHECK_VERSION(3,0,0)
        gtk_widget_set_halign(pw, GTK_ALIGN_END);
        gtk_widget_set_valign(pw, GTK_ALIGN_START);
#else
        gtk_misc_set_alignment(GTK_MISC(pw), 0, 0);
#endif
     
    pw = gtk_check_button_new_with_label ("Jacoby"); 
    gtk_box_pack_start(GTK_BOX(pwh), pw, FALSE, FALSE, 0);    
    g_signal_connect(G_OBJECT (pw), "toggled", G_CALLBACK(JacobyToggled), psm); 
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pw), moneyJacoby);
    gtk_widget_set_tooltip_text(pw, _("Toggle Jacoby option for money play"));

} else if (LAYOUT_VERTICAL2) { // start VERTICAL2
   // **************************************************************************************************
   // Vertical box to hold everything.
#if GTK_CHECK_VERSION(3,0,0)
    pwv = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
#else
    pwv = gtk_vbox_new(FALSE, 2); //gtk_vbox_new (gboolean homogeneous, gint spacing);
#endif
    gtk_container_add(GTK_CONTAINER(DialogArea(pwDialog, DA_MAIN)), pwv);
    
    gtk_box_pack_start(GTK_BOX(pwv), psm->pwTableContainer, TRUE, TRUE, 0);   //gtk_box_pack_start (GtkBox *box, GtkWidget *child, 
                                        // gboolean expand,  gboolean fill, guint padding);

    /* separator */

#if GTK_CHECK_VERSION(3,0,0)
    gtk_box_pack_start(GTK_BOX(pwv), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 0);
#else
    gtk_box_pack_start(GTK_BOX(pwv), gtk_hseparator_new(), FALSE, FALSE, 0);
#endif


    /* gauge */ 
    if (psm->cubeScoreMap)
        gtk_box_pack_start(GTK_BOX(pwv), pwGaugeTable, FALSE, FALSE, 0);

     /* separator */

#if GTK_CHECK_VERSION(3,0,0)
    gtk_box_pack_start(GTK_BOX(pwv), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 0);
#else
    gtk_box_pack_start(GTK_BOX(pwv), gtk_vseparator_new(), FALSE, FALSE, 0);
#endif

  
    // Holder for both columns
#if GTK_CHECK_VERSION(3,0,0)
    pwh = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
    pwh = gtk_hbox_new(FALSE, 24);
#endif
    gtk_box_pack_start(GTK_BOX(pwv), pwh, FALSE, FALSE, 0);

   // First column (using vertical box)
#if GTK_CHECK_VERSION(3,0,0)
    pwv = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
#else
    pwv = gtk_vbox_new(FALSE, 6); //gtk_vbox_new (gboolean homogeneous, gint spacing);
#endif
    gtk_box_pack_start(GTK_BOX(pwh), pwv, TRUE, TRUE, 0);

 /* eval ply */
    pwFrame=gtk_frame_new(_("Eval"));
    gtk_box_pack_start(GTK_BOX(pwv), pwFrame, FALSE, FALSE, 0);

// #if GTK_CHECK_VERSION(3,0,0)
//         pwv2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
// #else
//         pwv2 = gtk_vbox_new(FALSE, 4);
// #endif
//     gtk_container_add(GTK_CONTAINER(pwFrame), pwv2);
#if GTK_CHECK_VERSION(3,0,0)
    pwh2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
#else
    pwh2 = gtk_hbox_new(FALSE, 8);
#endif     
    gtk_container_add(GTK_CONTAINER(pwFrame), pwh2);

    for (i = 0; i < 5; ++i) {
        gchar *sz = g_strdup_printf(_("%d-ply"), i);
        if (i == 0)
            pw = pwx = gtk_radio_button_new_with_label(NULL, sz);
        else
            pw = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pwx), sz);
        g_free(sz);
        
        gtk_box_pack_start(GTK_BOX(pwh2), pw, FALSE, FALSE, 0);

        pi = (int *) g_malloc(sizeof(int));
        *pi = i;

        g_object_set_data_full(G_OBJECT(pw), "user_data", pi, g_free);

        g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(ScoreMapPlyToggled), psm);

    }
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pwx), TRUE);

    // We now insert the radio buttons of the "colour by" scheme, in (a) cube scoremap, or 
    // (b) move scoremap in "beta version" i.e. with MOVE_CLASSIFY unallowed, 
    // but in (b) we disable the buttons since they are only relevant to the cube scoremap, or 
    // a "describe by" in "alpha version" i.e. with MOVE_CLASSIFY allowed.
    
   /* Colour by */
   if (psm->cubeScoreMap || !MOVE_CLASSIFY) {
        pwFrame=gtk_frame_new(_("Colour by"));
        gtk_box_pack_start(GTK_BOX(pwv), pwFrame, FALSE, FALSE, 0);
        if (!psm->cubeScoreMap)
            gtk_widget_set_sensitive(pwFrame, FALSE);

    #if GTK_CHECK_VERSION(3,0,0)
        pwh2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    #else
        pwh2 = gtk_hbox_new(FALSE, 8);
    #endif     
        gtk_container_add(GTK_CONTAINER(pwFrame), pwh2);
        

        pw = pwx = gtk_radio_button_new_with_label(NULL, _("All"));// was: ND - D/T - D/P - TGTD")); // First radio button
        gtk_box_pack_start(GTK_BOX(pwh2), pw, FALSE, FALSE, 0);
        pi = (int *)g_malloc(sizeof(int)); //It is freed by gtk (that's why we use "g_free" as one of the arguments to g_object_set_data_full).
        *pi=(int)ALL;
        g_object_set_data_full(G_OBJECT(pw), "user_data", pi, g_free);
        g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(ColourByToggled), psm); 
        // gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pw), 1);
        if (colourBasedOn==ALL)
            pwColourDefault = pw; //we set this to toggle it on in case it's the default option


        pw =  gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pwx), _("ND vs D")); // Associate this to the other radio button
        gtk_box_pack_start(GTK_BOX(pwh2), pw, FALSE, FALSE, 0);
        pi = (int *)g_malloc(sizeof(int));
        *pi=(int)DND;
        g_object_set_data_full(G_OBJECT(pw), "user_data", pi, g_free);
        g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(ColourByToggled), psm);
        if (colourBasedOn==DND)
            pwColourDefault = pw; //we set this to toggle it on in case it's the default option

        pw =  gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pwx), _("T vs P")); // Associate this to the other radio button
        gtk_box_pack_start(GTK_BOX(pwh2), pw, FALSE, FALSE, 0);
        pi = (int *)g_malloc(sizeof(int));
        *pi=(int)PT;
        g_object_set_data_full(G_OBJECT(pw), "user_data", pi, g_free);
        g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(ColourByToggled), psm); 
        if (colourBasedOn==PT)
            pwColourDefault = pw; //we set this to toggle it on in case it's the default option
   } else { // MOVE_CLASSIFY in move scoremap
        pwFrame=gtk_frame_new(_("Describe moves using"));
        gtk_box_pack_start(GTK_BOX(pwv), pwFrame, FALSE, FALSE, 0);

    #if GTK_CHECK_VERSION(3,0,0)
        pwh2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    #else
        pwh2 = gtk_hbox_new(FALSE, 8);
    #endif     
        gtk_container_add(GTK_CONTAINER(pwFrame), pwh2);

        pw = pwx = gtk_radio_button_new_with_label(NULL, _("Numbers"));// First radio button
        gtk_box_pack_start(GTK_BOX(pwh2), pw, FALSE, FALSE, 0);
        pi = (int *)g_malloc(sizeof(int)); 
        *pi=(int)NUMBERS;
        g_object_set_data_full(G_OBJECT(pw), "user_data", pi, g_free);
        g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(DescribeUsingToggled), psm); 
        if (describeUsing==NUMBERS)
            pwDescribeDefault = pw; //we set this to toggle it on in case it's the default describeUsing option
        // gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pw), 0);
        // if (-1==NUMBERS)
        //     gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pw), 1);
        // gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pw), DEFAULT_DESCRIPTION==NUMBERS);

        pw =  gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pwx), _("English")); // Associate this to the other radio button
        gtk_box_pack_start(GTK_BOX(pwh2), pw, FALSE, FALSE, 0);
        pi = (int *)g_malloc(sizeof(int));
        *pi=(int)ENGLISH;
        g_object_set_data_full(G_OBJECT(pw), "user_data", pi, g_free);
        g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(DescribeUsingToggled), psm);
        if (describeUsing==ENGLISH)
            pwDescribeDefault = pw; //we set this to toggle it on in case it's the default describeUsing option
        // if (-1==ENGLISH)
        //     gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pw), 1);

        pw = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pwx), _("Both")); // Associate this to the other radio button
        gtk_box_pack_start(GTK_BOX(pwh2), pw, FALSE, FALSE, 0);
        pi = (int *)g_malloc(sizeof(int));
        *pi=(int)BOTH;
        g_object_set_data_full(G_OBJECT(pw), "user_data", pi, g_free);
        g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(DescribeUsingToggled), psm); 
        if (describeUsing==BOTH)
            pwDescribeDefault = pw; //we set this to toggle it on in case it's the default describeUsing option
        // if (BOTH==BOTH)
        //     gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pw), 1);
        // gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pwy), 1);
   }

  /* Label by toggle */
    // This button offers the choice between the display of true scores and away scores.
    pwFrame=gtk_frame_new(_("Label by"));
    gtk_box_pack_start(GTK_BOX(pwv), pwFrame, FALSE, FALSE, 0);

#if GTK_CHECK_VERSION(3,0,0)
    pwh2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
#else
    pwh2 = gtk_hbox_new(FALSE, 8);
#endif     
    gtk_container_add(GTK_CONTAINER(pwFrame), pwh2);
     
    pw = pwx = gtk_radio_button_new_with_label(NULL, _("True score")); 
    gtk_box_pack_start(GTK_BOX(pwh2), pw, FALSE, FALSE, 0);
    pi = (int *)g_malloc(sizeof(int));
    *pi=(int)LABEL_SCORE;
    g_object_set_data_full(G_OBJECT(pw), "label", pi, g_free);
    g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(LabelByToggled), psm);
    if (labelBasedOn==LABEL_SCORE)
        pwLabelDefault = pw; //we set this to toggle it on in case it's the default option
    // gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pw), labelBasedOn==LABEL_SCORE);

    pw =  gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pwx), _("Away score"));
    gtk_box_pack_start(GTK_BOX(pwh2), pw, FALSE, FALSE, 0);
    pi = (int *)g_malloc(sizeof(int)); 
    *pi=(int)LABEL_AWAY;
    g_object_set_data_full(G_OBJECT(pw), "label", pi, g_free);
    g_signal_connect(G_OBJECT(pw), "toggled", G_CALLBACK(LabelByToggled), psm); 
    if (labelBasedOn==LABEL_AWAY)
        pwLabelDefault = pw; //we set this to toggle it on in case it's the default option
    // gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pw), labelBasedOn==LABEL_AWAY);

   // Second column (using vertical box)
#if GTK_CHECK_VERSION(3,0,0)
    pwv = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
#else
    pwv = gtk_vbox_new(FALSE, 6); //gtk_vbox_new (gboolean homogeneous, gint spacing);
#endif
    gtk_box_pack_start(GTK_BOX(pwh), pwv, TRUE, TRUE, 0);

    /* Match length */
    pwFrame=gtk_frame_new(_("Match length"));
    gtk_box_pack_start(GTK_BOX(pwv), pwFrame, FALSE, FALSE, 0);
#if GTK_CHECK_VERSION(3,0,0)
    pwh2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
#else
    pwh2 = gtk_hbox_new(FALSE, 8);
#endif     
    gtk_container_add(GTK_CONTAINER(pwFrame), pwh2);

// #if GTK_CHECK_VERSION(3,0,0)
//     pwh = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
// #else
//     pwh = gtk_hbox_new(FALSE, 8);
// #endif
//     // pw=gtk_label_new(_("Match length: "));
//     // gtk_box_pack_start(GTK_BOX(pwv), pwh, FALSE, FALSE, 0);
//     // gtk_box_pack_start(GTK_BOX(pwh), pw, FALSE, FALSE, 0);
//    gtk_box_pack_start(GTK_BOX(pwv2), pwh, FALSE, FALSE, 0);

// #if GTK_CHECK_VERSION(3,0,0)
//         gtk_widget_set_halign(pw, GTK_ALIGN_START);
//         gtk_widget_set_valign(pw, GTK_ALIGN_START);
// #else
//         gtk_misc_set_alignment(GTK_MISC(pw), 0, 0);
// #endif

    pw= gtk_combo_box_text_new();
    gtk_box_pack_start(GTK_BOX(pwh2), pw, FALSE, FALSE, 0);

    for (i=0; i<NUM_MATCH_SIZE_OPTIONS; i++) {
        char sz[20]; 
        sprintf(sz,"%d",MATCH_SIZE_OPTIONS[i]);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pw), _(sz));
    }    
    g_signal_connect(G_OBJECT(pw), "changed", G_CALLBACK(MatchLengthToggled), psm); 
    gtk_combo_box_set_active(GTK_COMBO_BOX(pw), matchSizeIndex); 

    /*cube*/

    psm->pwCubeFrame=gtk_frame_new(_("Cube value"));
    gtk_box_pack_start(GTK_BOX(pwv), psm->pwCubeFrame, FALSE, FALSE, 0);
   
    BuildCubeFrame(psm);
//bbb

    /* Jacoby */
    pwFrame=gtk_frame_new(_("Jacoby"));
    gtk_box_pack_start(GTK_BOX(pwv), pwFrame, FALSE, FALSE, 0);
   
#if GTK_CHECK_VERSION(3,0,0)
    pwh2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
#else
    pwh2 = gtk_hbox_new(FALSE, 8);
#endif     
    gtk_container_add(GTK_CONTAINER(pwFrame), pwh2);
     
    pw = gtk_check_button_new_with_label ("Jacoby"); 
    gtk_box_pack_start(GTK_BOX(pwh2), pw, FALSE, FALSE, 0);    
    g_signal_connect(G_OBJECT (pw), "toggled", G_CALLBACK(JacobyToggled), psm); 
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pw), moneyJacoby);
    gtk_widget_set_tooltip_text(pw, _("Toggle Jacoby option for money play"));


    
} // end VERTICAL2

// **************************************************************************************************
    /* calculate values and set colours/text in the table */
    UpdateCubeInfoArray(psm, psm->signednCube, FALSE); // fills psm->aaQuadrantData[i][j].ci for each i,j
    CalcScoreMapEquities(psm,0,FALSE);    //Find equities at each score, and use these to set the text for the corresponding box.
    UpdateScoreMapVisual(psm);      //Update: (1) The color of each square (2) The hover text of each square 
                                    //      (3) the row/col score labels (4) the gauge.

    if (pwDescribeDefault)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pwDescribeDefault), 1);
    if (pwLabelDefault)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pwLabelDefault), 1);
    if (pwColourDefault)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pwColourDefault), 1);

    
    /* modality */

    gtk_window_set_default_size(GTK_WINDOW(pwDialog), 400, 500);
    g_object_weak_ref(G_OBJECT(pwDialog), DestroyDialog, psm);

    GTKRunDialog(pwDialog);


}


/*

====== DONE: ===== postponed to the very end of the file

white/black font based on background
"current" in double frame
cube scoremap all in grey?
no doubling in Crawford
isaac doubled -> bottom not left
typedef enum isAllowed
take negative equity into account for len in hover text
allowed cube beyond matchSize in options!? cf up to 2 matchsize-1? (eg cube 4 w/ size=3 is allowed b/c previous was 2<size )
recompute max allowed cube for big tables and redraw buttons?
put in grey the squares that are impossible with cube
cube: 2 for player 1 vs 2 for player 2, maybe encode +/-nCube
checkbox for Jacoby?
bury cubeScoreMap
add spacing for 2-line quadrants
add equity to hover
add isspecial to hover
show GG, GS etc
money text bug
// fixed-sze cols/rows & variable font size on col/row labels?
***check why cube tab doesn't work
Aaron email crawford
fill hover with constant-length(text) to align; or build inside table? (<tr><td>...)
bug 54 in last game: 2 colors???
add hover on label with the full "Crawford" name?
=== older? from merge:
- Player names on axes (we'll want the name rotated for the vertical axis..) - done
- Change dice to away-scores - done
- Change gauge: white, somewhere in the middle, should be labelled by "0.000". The ends should be labelled both by the value and the correct decision they represent (e.g., "Take")
- Remove "Show best move" and "Show equities" check boxes - done
- Add radio buttons for "Colour based on ..." - done (but not yet operational)
- Change how colouring is applied
- multi-color scheme; 1) redraw gauge for ND/D, T/P 2) maybe make gauge to stop at SKILL_VERYBAD? 3) make DP more blue specifically?
- The current version is based on the gnubg release code. However the code has been updated since, so we need to clean up
    and merge with the latest files.
- Display n-away vs score
- Make non-modal
- Emphasize current score even more
- Add options to change 
    * the table size 
    * and cube value 
=======


*/

