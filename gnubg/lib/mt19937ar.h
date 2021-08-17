/*
 * Copyright (C) 2002 Gary Wong <gtw@gnu.org>
 * Copyright (C) 2004-2015 the AUTHORS
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
 * $Id$
 */

#ifndef MT19937AR_H
#define MT19937AR_H

#define MT_ARRAY_N 624

extern void init_genrand(unsigned long s, int *mti, unsigned long mt[MT_ARRAY_N]);
extern unsigned long genrand_int32(int *mti, unsigned long mt[MT_ARRAY_N]);
void init_by_array(unsigned long init_key[], int key_length, int *mti, unsigned long mt[MT_ARRAY_N]);

#endif
