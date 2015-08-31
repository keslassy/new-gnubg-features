/*
 * mt19937int.h
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
