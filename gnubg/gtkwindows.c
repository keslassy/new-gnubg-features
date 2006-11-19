/*
 * gtkwindows.c
 * by Jon Kinsey, 2006
 *
 * functions to create windows
 *
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

#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "gtkwindows.h"
#include "gtkgame.h"
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>
#include "gtktoolbar.h"

static char *aszStockItem[ NUM_DIALOG_TYPES ] =
{
	GTK_STOCK_DIALOG_INFO,
	GTK_STOCK_DIALOG_QUESTION,
	GTK_STOCK_DIALOG_WARNING,
	GTK_STOCK_DIALOG_WARNING,
	GTK_STOCK_DIALOG_ERROR,
	GTK_STOCK_DIALOG_GNU,
	GTK_STOCK_DIALOG_GNU_QUESTION
};

void quitter(GtkWidget *widget, GtkWidget *parent)
{
  gtk_main_quit();
  if (parent)
	  gtk_window_present(GTK_WINDOW(parent));
}

typedef struct _CallbackStruct
{
	void (*DialogFun)(GtkWidget*, void*);
	gpointer data;
} CallbackStruct;

static void DialogResponse(GtkWidget *dialog, gint response, CallbackStruct *data)
{
	if (response == GTK_RESPONSE_OK || response == GTK_RESPONSE_CLOSE)
	{
		if (data->DialogFun)
			data->DialogFun(dialog, data->data);
		else
			OK(dialog, data->data);
	}
	else if (response == GTK_RESPONSE_CANCEL)
	{
		gtk_widget_destroy(dialog);
	}
	else
	{	/* Ignore response */
	}
	if (data)
		free(data);
}

extern GtkWidget *GTKCreateDialog(const char *szTitle, const dialogtype dt, 
	 GtkWidget *parent, int flags, GtkSignalFunc okFun, void *okFunData)
{
	CallbackStruct* cbData;
    GtkWidget *pwDialog, *pwHbox, *pwPixmap;
    GtkAccelGroup *pag;
    int fQuestion = (dt == DT_QUESTION || dt == DT_AREYOUSURE);

	pwDialog = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(pwDialog), szTitle);

	if (parent == NULL)
		parent = GTKGetCurrentParent();
	if (!GTK_IS_WINDOW(parent))
		parent = gtk_widget_get_toplevel(parent);
	if (parent && !GTK_WIDGET_REALIZED(parent))
		parent = NULL;
	if (parent != NULL && (flags & DIALOG_FLAG_MODAL))
	{
		if ((flags & DIALOG_FLAG_NOTIDY) == 0)
			gtk_signal_connect(GTK_OBJECT(pwDialog), "destroy", GTK_SIGNAL_FUNC(quitter), parent);
		gtk_window_set_transient_for(GTK_WINDOW(pwDialog), GTK_WINDOW(parent));
	}

	pag = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(pwDialog), pag);

	pwHbox = gtk_hbox_new(FALSE, 0);
    gtk_container_add( GTK_CONTAINER( GTK_DIALOG( pwDialog )->vbox ), pwHbox );

	if (flags & DIALOG_FLAG_CUSTOM_PICKMAP)
	{
		pwPixmap = image_from_xpm_d ((char**)okFunData, pwDialog);
		okFunData = NULL;	/* Don't use pixmap below */
	}
	else
	    pwPixmap = gtk_image_new_from_stock( aszStockItem[ dt ], GTK_ICON_SIZE_DIALOG );
    gtk_misc_set_padding( GTK_MISC( pwPixmap ), 8, 8 );
	gtk_box_pack_start(GTK_BOX(pwHbox), pwPixmap, FALSE, FALSE, 0);

	cbData = (CallbackStruct*)malloc(sizeof(CallbackStruct));
	cbData->DialogFun = (void*)okFun;
	cbData->data = okFunData;
	g_signal_connect(pwDialog, "response", GTK_SIGNAL_FUNC(DialogResponse), cbData);

	if ((flags & DIALOG_FLAG_NOOK) == 0)
	{
		int OkButton = (flags & DIALOG_FLAG_MODAL && (flags & DIALOG_FLAG_CLOSEBUTTON) == 0);

		gtk_dialog_add_button(GTK_DIALOG( pwDialog ), OkButton ? _("OK") : _("Close"), OkButton ? GTK_RESPONSE_OK : GTK_RESPONSE_CLOSE);
		gtk_dialog_set_default_response(GTK_DIALOG( pwDialog ), OkButton ? GTK_RESPONSE_OK : GTK_RESPONSE_CLOSE);
	}

    if( fQuestion )
		gtk_dialog_add_button(GTK_DIALOG( pwDialog ), _("Cancel"), GTK_RESPONSE_CANCEL);

	if (flags & DIALOG_FLAG_MODAL)
		gtk_window_set_modal(GTK_WINDOW(pwDialog), TRUE);

    return pwDialog;
}

extern GtkWidget *DialogArea( GtkWidget *pw, dialogarea da )
{
    GList *pl;
    GtkWidget *pwChild;

	switch( da ) {
    case DA_MAIN:
		pl = gtk_container_children(GTK_CONTAINER(GTK_DIALOG(pw)->vbox));
		pwChild = pl->data;
		g_list_free( pl );
		return pwChild;

    case DA_BUTTONS:
		return GTK_DIALOG( pw )->action_area;

    case DA_OK:
		pl = gtk_container_children( GTK_CONTAINER( GTK_DIALOG( pw )->action_area ) );
		while (pl)
		{
			pwChild = pl->data;
			if (!strcmp(gtk_button_get_label(GTK_BUTTON(pwChild)), _("OK")))
				break;
			pl = pl->next;
		}
		g_list_free( pl );
		return pwChild;
	
    default:
		abort();
    }
}

/* Use to temporarily set the parent dialog for nested dialogs
	Note that passing a control of a window is ok (and common) */
GtkWidget *pwCurrentParent = NULL;
void GTKSetCurrentParent(GtkWidget *parent)
{
	pwCurrentParent = parent;
}
GtkWidget *GTKGetCurrentParent()
{
	if (pwCurrentParent)
	{
		GtkWidget *current = pwCurrentParent;
		pwCurrentParent = NULL;	/* Single set/get usage */
        return current;
	}
	else
		return pwMain;
}

extern int 
GTKMessage( char *sz, dialogtype dt )
{
    int f = FALSE, fRestoreNextTurn;
    static char *aszTitle[ NUM_DIALOG_TYPES - 1 ] = {
	N_("GNU Backgammon - Message"),
	N_("GNU Backgammon - Question"),
	N_("GNU Backgammon - Warning"), /* are you sure */
	N_("GNU Backgammon - Warning"),
	N_("GNU Backgammon - Error"),
	N_("GNU Backgammon - Question"),
    };
    GtkWidget *pwDialog = GTKCreateDialog( gettext( aszTitle[ dt ] ),
					   dt, GTKGetCurrentParent(), DIALOG_FLAG_MODAL, NULL, &f );
    GtkWidget *psw;
    GtkWidget *pwPrompt = gtk_label_new( sz );
    GtkRequisition req;

    gtk_misc_set_padding( GTK_MISC( pwPrompt ), 8, 8 );
    gtk_label_set_justify( GTK_LABEL( pwPrompt ), GTK_JUSTIFY_LEFT );
    gtk_label_set_line_wrap( GTK_LABEL( pwPrompt ), TRUE );

    gtk_widget_size_request( GTK_WIDGET( pwPrompt ), &req );

    if ( req.height > 500 ) {
	psw = gtk_scrolled_window_new( NULL, NULL );
	gtk_container_add( GTK_CONTAINER( DialogArea( pwDialog, DA_MAIN ) ),
			   psw );
	gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW( psw ),
					       pwPrompt );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( psw ),
					GTK_POLICY_NEVER, GTK_POLICY_ALWAYS );
	gtk_window_set_default_size( GTK_WINDOW( pwDialog ),
				     req.width, req.height );
	gtk_window_set_policy( GTK_WINDOW( pwDialog ), FALSE, TRUE, TRUE );
    }
    else {
    gtk_container_add( GTK_CONTAINER( DialogArea( pwDialog, DA_MAIN ) ),
		       pwPrompt );
    gtk_window_set_policy( GTK_WINDOW( pwDialog ), FALSE, FALSE, FALSE );
    }

    gtk_widget_show_all( pwDialog );

    /* This dialog should be REALLY modal -- disable "next turn" idle
       processing and stdin handler, to avoid reentrancy problems. */
    if( ( fRestoreNextTurn = nNextTurn ) )
	gtk_idle_remove( nNextTurn );
	    
    GTKDisallowStdin();
    gtk_main();
    GTKAllowStdin();

    if( fRestoreNextTurn )
	nNextTurn = gtk_idle_add( NextTurnNotify, NULL );
    
    return f;
}

extern int GTKGetInputYN( char *szPrompt )
{
    return GTKMessage( szPrompt, DT_AREYOUSURE );
}

char* inputString;
static void GetInputOk( GtkWidget *pw, GtkWidget *pwEntry )
{
	inputString = strdup(gtk_entry_get_text(GTK_ENTRY(pwEntry)));
    gtk_widget_destroy(gtk_widget_get_toplevel(pw));
}

extern char* GTKGetInput(char* title, char* prompt)
{
	GtkWidget *pwDialog, *pwHbox, *pwEntry;
	pwEntry = gtk_entry_new();
	inputString = NULL;
	pwDialog = GTKCreateDialog(title, DT_QUESTION, NULL, DIALOG_FLAG_MODAL, GTK_SIGNAL_FUNC(GetInputOk), pwEntry );

	gtk_container_add(GTK_CONTAINER(DialogArea(pwDialog, DA_MAIN)), 
		pwHbox = gtk_hbox_new(FALSE, 0));

	gtk_box_pack_start(GTK_BOX(pwHbox), gtk_label_new(prompt), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(pwHbox), pwEntry, FALSE, FALSE, 0);
	gtk_widget_grab_focus(pwEntry);

	gtk_widget_show_all( pwDialog );

	GTKDisallowStdin();
	gtk_main();
	GTKAllowStdin();
	return inputString;
}
