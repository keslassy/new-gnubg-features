/*
 * Copyright (C) 1999-2003 Joern Thyssen <jth@gnubg.org>
 * Copyright (C) 2000-2013 the AUTHORS
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
 * $Id: matchequity.h,v 1.32 2022/01/19 22:50:41 plm Exp $
 */

#ifndef MATCHEQUITY_H
#define MATCHEQUITY_H

#include "eval.h"

#define MAXSCORE      64
#define MAXCUBELEVEL  7

/* Structure for information about match equity table */

typedef struct {
    gchar *szName;              /* Name of match equity table */
    gchar *szFileName;          /* File name of MET */
    gchar *szDescription;       /* Description of MET */
    int nLength;                /* Native length of MET or -1 if pure calculated table */
} metinfo;

/* macros for getting match equities */

#define GET_MET(i,j,aafMET) ( ( (i) < 0 ) ? 1.0f : ( ( (j) < 0 ) ? 0.0f : \
						 (aafMET [ i ][ j ]) ) )
#define GET_METPostCrawford(i,afBtilde) ( (i) < 0 ? 1.0f : afBtilde [ i ] )

/* current match equity table used by gnubg */

extern float aafMET[MAXSCORE][MAXSCORE];
extern float aafMETPostCrawford[2][MAXSCORE];

/* gammon prices (calculated once for efficiency) */

extern float aaaafGammonPrices[MAXCUBELEVEL]
    [MAXSCORE][MAXSCORE][4];
extern float aaaafGammonPricesPostCrawford[MAXCUBELEVEL]
    [MAXSCORE][2][4];

extern metinfo miCurrent;

extern float
getME(const int nScore0, const int nScore1, const int nMatchTo,
      const int fPlayer,
      const int nPoints, const int fWhoWins,
      const int fCrawford, float aafMET[MAXSCORE][MAXSCORE], float aafMETPostCrawford[2][MAXSCORE]);

extern float
getMEAtScore(const int nScore0, const int nScore1, const int nMatchTo,
             const int fPlayer, const int fCrawford,
             float aafMET[MAXSCORE][MAXSCORE], float aafMETPostCrawford[2][MAXSCORE]);

/* Initialise match equity table */

void
 InitMatchEquity(const char *szFileName);

/* Get double points */

extern int
 GetPoints(float arOutput[5], const cubeinfo * pci, float arCP[2]);

extern float
 GetDoublePointDeadCube(float arOutput[5], cubeinfo * pci);

extern void
 invertMET(void);

/* enums for the entries in the arrays returned by getMEMultiple 
 * DoublePass, DoubleTakeWin, DoubleTakeWinGammon... for the first 8
 * then the same values using CubePrimeValues 
 * DP = Double/Pass, DTWG = double/take/wing gammon, etc
 * DPP = double/Pass with CubePrime values, etc.
 */

typedef enum {
    /* player 0 wins, first cube value */
    DP = 0, NDW = 0, DTW = 1, NDWG = 1, NDWB, DTWG, DTWB,
    /* player 0 loses, first cube value */
    NDL, DTL = 6, NDLG = 6, NDLB, DTLG, DTLB,
    /* player 0 wins, 2nd cube value */
    DPP0, DTWP0, NDWBP0, DTWGP0, DTWBP0,
    /* player 0 loses, 2nd cube value */
    NDLP0, DTLP0, NDLBP0, DTLGP0, DTLBP0,
    /* player 0 wins, 3rd cube value */
    DPP1, DTWP1, NDWBP1, DTWGP1, DTWBP1,
    /* player 0 loses, 3rd cube value */
    NDLP1, DTLP1, NDLBP1, DTLGP1, DTLBP1
} met_indices;

extern void
getMEMultiple(const int nScore0, const int nScore1, const int nMatchTo,
              const int nCube,
              const int nCubePrime0, const int nCubePrime1,
              const int fCrawford,
              float aafMET[MAXSCORE][MAXSCORE], float aafMETPostCrawford[2][MAXSCORE], float *player0, float *player1);

#endif
