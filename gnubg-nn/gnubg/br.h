// -*- C++ -*-

/*
 * br.h
 *
 * by Joseph Heled <joseph@gnubg.org>, 2000
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
 */

#if !defined( BR_H )
#define BR_H

#if defined( __GNUG__ )
#pragma interface
#endif

//  bearoff info
struct B {
  // number of probabilities
  unsigned int len;

  // p[0] is probablity of bearing off in 'start' rolls
  unsigned int start;

  // array of probabilities.
  float p[32];
};


#if defined( __GNUG__ )
extern "C" {
#endif

  void getBearoffProbs(unsigned int n, int p[32]);

  unsigned int getBearoffProbs2(unsigned int n, unsigned int nOpp);

  void getBearoff(unsigned int n, struct B* b);
  
#if defined( __GNUG__ )
}
#endif

#endif
