/*
 * gtkgame.h
 *
 * by Gary Wong <gtw@gnu.org>, 2000, 2002.
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

#ifndef _GTKGAME_H_
#define _GTKGAME_H_

#include <stdio.h>

#include "backgammon.h"
#include "rollout.h"

#if !HAVE_GTK_OPTION_MENU_GET_HISTORY
extern gint gtk_option_menu_get_history (GtkOptionMenu *option_menu);
#endif

typedef enum _dialogarea {
    DA_MAIN,
    DA_BUTTONS,
    DA_OK
} dialogarea;

typedef enum _dialogtype {
    DT_INFO,
    DT_QUESTION,
    DT_AREYOUSURE,
    DT_WARNING,
    DT_ERROR,
    DT_GNU,
    NUM_DIALOG_TYPES
} dialogtype;

typedef enum _filedialogtype { 
  FDT_NONE=0, FDT_SAVE, FDT_EXPORT, FDT_IMPORT, FDT_EXPORT_FULL
} filedialogtype;

typedef enum _warnings { 
  WARN_FULLSCREEN_EXIT=0, WARN_QUICKDRAW_MODE, WARN_SET_SHADOWS, 
	  WARN_UNACCELERATED, WARN_NUM_WARNINGS
} warnings;

extern void GTKShowWarning(warnings warning);
extern char* warningStrings[WARN_NUM_WARNINGS];
extern char* warningNames[WARN_NUM_WARNINGS];
extern int warningEnabled[WARN_NUM_WARNINGS];

extern GtkWidget *pwMain, *pwMenuBar;
extern GtkWidget *pwToolbar;
extern GtkTooltips *ptt;

extern GtkWidget *pwGrab;
extern GtkWidget *pwOldGrab;

extern int lastImportType, lastExportType;
extern int fEndDelay;

extern void ShowGameWindow( void );

extern void GTKAddMoveRecord( moverecord *pmr );
extern void GTKPopMoveRecord( moverecord *pmr );
extern void GTKSetMoveRecord( moverecord *pmr );
extern void GTKClearMoveRecord( void );
extern void GTKAddGame( moverecord *pmr );
extern void GTKPopGame( int c );
extern void GTKSetGame( int i );
extern void GTKRegenerateGames( void );

extern void GTKFreeze( void );
extern void GTKThaw( void );

extern void GTKSuspendInput( monitor *pm );
extern void GTKResumeInput( monitor *pm );

#if USE_TIMECONTROL
extern void GTKUpdateClock();
#endif

extern int InitGTK( int *argc, char ***argv );
extern void RunGTK( GtkWidget *pwSplash );
extern void GTKAllowStdin( void );
extern void GTKDisallowStdin( void );
extern void GTKDelay( void );
extern void ShowList( char *asz[], char *szTitle );
extern void GTKUpdateScores();

extern GtkWidget *GTKCreateDialog( const char *szTitle, const dialogtype dt, 
                                   GtkSignalFunc pf, void *p );
extern GtkWidget *DialogArea( GtkWidget *pw, dialogarea da );
    
extern int GTKGetInputYN( char *szPrompt );
extern void GTKOutput( char *sz );
extern void GTKOutputErr( char *sz );
extern void GTKOutputX( void );
extern void GTKOutputNew( void );
extern void GTKProgressStart( char *sz );
extern void GTKProgress( void );
extern void GTKProgressEnd( void );
extern void
GTKProgressStartValue( char *sz, int iMax );
extern void
GTKProgressValue ( int fValue );
extern void GTKBearoffProgress( int i );

extern void GTKDumpStatcontext( int game );
extern void GTKEval( char *szOutput );
extern void 
GTKHint( movelist *pmlOrig, const int iMove );
extern void GTKCubeHint( float aarOutput[ 2 ][ NUM_ROLLOUT_OUTPUTS ], 
			 float aarStdDev[ 2 ][ NUM_ROLLOUT_OUTPUTS ], 
			 const evalsetup *pes );

extern void GTKSet( void *p );
extern void GTKUpdateAnnotations( void );
extern int GTKGetManualDice( int an[ 2 ] );
extern void GTKShowVersion( void );
extern void GTKShowCalibration( void );
extern void *GTKCalibrationStart( void ),
    GTKCalibrationUpdate( void *context, float rEvalsPerSec ),
    GTKCalibrationEnd( void *context );
extern void GTKDumpRolloutResults(GtkWidget *widget, gpointer data);
extern void GTKWinCopy( GtkWidget *widget, gpointer data);
extern void GTKResign( gpointer *p, guint n, GtkWidget *pw);
extern void
GTKResignHint( float arOutput[], float rEqBefore, float rEqAfter,
               cubeinfo *pci, int fMWC );
extern void GTKSaveSettings( void );
extern void GTKSetCube( gpointer *p, guint n, GtkWidget *pw );
extern void GTKSetDice( gpointer *p, guint n, GtkWidget *pw );
extern void GTKHelp( char *sz );
extern void GTKMatchInfo( void );

extern void SetEvaluation( gpointer *p, guint n, GtkWidget *pw );
extern void SetRollouts( gpointer *p, guint n, GtkWidget *pw );
extern void SetMET( GtkWidget *pw, gpointer p );

extern void HintMove( GtkWidget *pw, GtkWidget *pwMoves );

extern int fTTY;

extern int 
GtkTutor ( char *sz );

extern void
GTKCopy ( void );
extern void
GTKNew ( void );

extern void
UpdateGeometry ( const gnubgwindow gw );

extern void
RefreshGeometries ( void );

extern int
GTKGetMove ( int anMove[ 8 ] );

extern void GTKRecordShow( FILE *pfIn, char *sz, char *szPlayer );

extern void
GTKTextToClipboard( const char *sz );

extern char *
GTKChangeDisk( const char *szMsg, const int fChange, 
               const char *szMissingFile );

extern int 
GTKMessage( char *sz, dialogtype dt );

extern int 
GTKReadNumber( char *szTitle, char *szPrompt, int nDefault,
               int nMin, int nMax, int nInc );

extern void GTKFileCommand( char *szPrompt, char *szDefault, char *szCommand,
                            char *szPath, filedialogtype fdt );

#if USE_TIMECONTROL
extern void GTKAddTimeControl( char *szName) ;
extern void GTKRemoveTimeControl( char *szName) ;
extern void GTKCheckTimeControl( char *szName) ;
#endif

#endif
