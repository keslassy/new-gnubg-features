
#ifndef _TIMECONTROL_H_
#define  _TIMECONTROL_H_

#include <stdio.h>
#include <time.h>
#if HAVE_SYS_TIME
#include <sys/time.h>
#endif

#if USE_GTK
#include <gtk/gtk.h>
#endif

#include "backgammon.h"

extern timecontrol tc;


/* Initialise GameClock at start of match
 * @param pgc pointer to gameclock to initialise
 * @param ptc pointer to timecontrol to use
 * @param nPoints length of match
 */
extern void InitGameClock(gameclock *pgc, timecontrol *ptc, int nPoints);

/* Start the game clock for fPlayer 
 * Proper sequence for hitting the clock is :
 * - call CheckGameClock to update the game clock and check for timeout
 * -- apply any penalties that apply
 * - resolve any action done (move/double/drop. etc ... ) including
 *   updating ms.fTurn to its proper value.
 * - call HitGameClock
 * @param pms pointer to matchstate 
 */
extern void HitGameClock(matchstate *pms);


/* Updates the Game clock and checks whether the time has run out.
 * for the player whose clock is running
 * @param tvp timestamp for hit
 * @return how many penalty points to apply
 */
extern int CheckGameClock(matchstate *pms, struct timeval *tvp);

/* Make a formatted string for the given player.
 * The string is allocated in separate static buffers for each player,
 * and a call to FormatClock(0) will not destroy the contents of FormatClock(1)
 * @param fPlayer
 * @return Remaining clock time on the format h:mm:ss 
 *	in addition an 'F' will be added if the flag has fallen,
 *	or 'Fx2', 'Fx3' etc. if the flag has fallen again ...
 */
extern char *FormatClock(/* matchstate *pms, */ int fPlayer);

/* Force a (real time) update to the clock which again will 
 * issue a dummy MOVE_TIME record which again will generate
 * a CheckGameClock call - which then will generate a proper
 * MOVE_TIME record if time has run out
 */ 
#if USE_GUI
#if USE_GTK
extern gboolean UpdateClockNotify(gpointer *p); 
#else
extern int UpdateClockNotify(event *pev, void *p); 
#endif
#else
extern int UpdateClockNotify(void *p)
#endif


/* Save time settings
 * @param open settings file
 */

extern void SaveTimeControlSettings( FILE *pf );
extern void CommandShowTCTutorial ();

#endif /* _TIMECONTROL_H_ */
