/*
 * Copyright (C) 2020 Aaron Tikuisis <Aaron.Tikuisis@uottawa.ca>
 * Copyright (C) 2020 Isaac Keslassy <keslassy@gmail.com>
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
 * $Id: gtkscoremap.h,v 1.3 2022/10/22 10:47:46 plm Exp $
 */

#ifndef GTKSCOREMAP_H
#define GTKSCOREMAP_H

#include "gnubg-types.h"        /* for matchstate */
//#include "gtkoptions.h"    

typedef enum {
    ZERO_PLY, ONE_PLY, TWO_PLY, THREE_PLY, FOUR_PLY,
    NUM_PLY
} scoreMapPly;

extern scoreMapPly scoreMapPlyDefault;
extern const char* aszScoreMapPly[NUM_PLY];
extern const char* aszScoreMapPlyCommands[NUM_PLY];

typedef enum {
    LENGTH_THREE, LENGTH_FIVE, LENGTH_SEVEN, LENGTH_NINE, LENGTH_ELEVEN, LENGTH_FIFTEEN, LENGTH_TWENTY_ONE,
    VAR_LENGTH,
    NUM_MATCH_LENGTH
} scoreMapMatchLength;

extern scoreMapMatchLength scoreMapMatchLengthDefIdx;
extern const int FIXED_MATCH_LENGTH_OPTIONS[NUM_MATCH_LENGTH];
extern const char* aszScoreMapMatchLength[NUM_MATCH_LENGTH];
extern const char* aszScoreMapatchLengthCommands[NUM_MATCH_LENGTH];
 

extern void GTKShowScoreMap(const matchstate ams[], int cube);
extern void GTKShowMoveScoreMapInfo(GtkWidget* pw, GtkWidget* pwParent);
extern void GTKShowCubeScoreMapInfo(GtkWidget* pw, GtkWidget* pwParent);


#endif                          /* GTKSCOREMAP_H */
