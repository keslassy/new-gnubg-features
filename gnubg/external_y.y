%{
/*
 * external_y.y -- command parser for external interface
 *
 * by Michael Petch <mpetch@gnubg.org>, 2014.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 */

#ifndef EXTERNAL_Y_H
#define EXTERNAL_Y_H

#define MERGE_(a,b)  a##b
#define LABEL_(a,b) MERGE_(a, b)

#define YY_PREFIX(variable) MERGE_(ext_,variable)

#define yymaxdepth YY_PREFIX(maxdepth)
#define yyparse YY_PREFIX(parse)
#define yylex   YY_PREFIX(lex)
#define yyerror YY_PREFIX(error)
#define yylval  YY_PREFIX(lval)
#define yychar  YY_PREFIX(char)
#define yydebug YY_PREFIX(debug)
#define yypact  YY_PREFIX(pact)
#define yyr1    YY_PREFIX(r1)
#define yyr2    YY_PREFIX(r2)
#define yydef   YY_PREFIX(def)
#define yychk   YY_PREFIX(chk)
#define yypgo   YY_PREFIX(pgo)
#define yyact   YY_PREFIX(act)
#define yyexca  YY_PREFIX(exca)
#define yyerrflag YY_PREFIX(errflag)
#define yynerrs YY_PREFIX(nerrs)
#define yyps    YY_PREFIX(ps)
#define yypv    YY_PREFIX(pv)
#define yys     YY_PREFIX(s)
#define yy_yys  YY_PREFIX(yys)
#define yystate YY_PREFIX(state)
#define yytmp   YY_PREFIX(tmp)
#define yyv     YY_PREFIX(v)
#define yy_yyv  YY_PREFIX(yyv)
#define yyval   YY_PREFIX(val)
#define yylloc  YY_PREFIX(lloc)
#define yyreds  YY_PREFIX(reds)
#define yytoks  YY_PREFIX(toks)
#define yylhs   YY_PREFIX(yylhs)
#define yylen   YY_PREFIX(yylen)
#define yydefred YY_PREFIX(yydefred)
#define yydgoto  YY_PREFIX(yydgoto)
#define yysindex YY_PREFIX(yysindex)
#define yyrindex YY_PREFIX(yyrindex)
#define yygindex YY_PREFIX(yygindex)
#define yytable  YY_PREFIX(yytable)
#define yycheck  YY_PREFIX(yycheck)
#define yyname   YY_PREFIX(yyname)
#define yyrule   YY_PREFIX(yyrule)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "glib-ext.h"
#include "external.h"
#include "backgammon.h"
#include "external_y.h"

#define extcmd ext_get_extra(scanner)  

int YY_PREFIX(get_column)  (void * yyscanner );
void YY_PREFIX(set_column) (int column_no, void * yyscanner );
extern int YY_PREFIX(lex) (YYSTYPE * yylval_param, scancontext *scanner);         
extern scancontext *YY_PREFIX(get_extra) (void *yyscanner );
extern void StartParse(void *scancontext);
extern void yyerror(scancontext *scanner, const char *str);

void yyerror(scancontext *scanner, const char *str)
{
    if (extcmd->ExtErrorHandler)
        extcmd->ExtErrorHandler(extcmd, str);
    else
        fprintf(stderr,"Error: %s\n",str);
}

#endif

%}
%pure-parser
%file-prefix "y"
%lex-param   {void *scanner}
%parse-param {scancontext *scanner}
     
%defines
%error-verbose
%token-table

%union {
    gboolean bool;
    gchar character;
    gdouble floatnum;
    gint intnum;
    GString *str;
    GValue *gv;
    GList *list;
    commandinfo *cmd;
}

%{
%}

%token EOL EXIT DISABLED EXTVERSION
%token EXTSTRING EXTCHARACTER EXTINTEGER EXTFLOAT EXTBOOLEAN
%token FIBSBOARD FIBSBOARDEND EVALUATION
%token CRAWFORDRULE JACOBYRULE
%token CUBE CUBEFUL CUBELESS DETERMINISTIC NOISE PLIES PRUNE

%type <bool>        EXTBOOLEAN
%type <str>         EXTSTRING
%type <character>   EXTCHARACTER
%type <intnum>      EXTINTEGER
%type <floatnum>    EXTFLOAT

%type <gv>          float_type
%type <gv>          integer_type
%type <gv>          string_type
%type <gv>          boolean_type
%type <gv>          basic_types

%type <gv>          list_type
%type <gv>          list_element
%type <gv>          board_element

%type <list>        list
%type <list>        list_elements
%type <list>        board
%type <list>        board_elements

%type <list>        evaloptions
%type <list>        evaloption
%type <list>        sessionoption
%type <list>        sessionoptions

%type <gv>          evalcommand
%type <gv>          boardcommand
%type <cmd>         command

%destructor { if ($$) g_string_free($$, TRUE); } <str>
%destructor { if ($$) g_list_free($$); } <list>
%destructor { if ($$) { g_value_unsetfree($$); }} <gv>
%destructor { if ($$) { g_free($$); }} <cmd>
%%

commands:
    /* Empty */
    EOL
    {
        extcmd->ct = COMMAND_NONE;
        YYACCEPT;
    }
    |
    EXTVERSION EOL
        {
            extcmd->ct = COMMAND_VERSION;
            YYACCEPT;
        }
    |
    EXIT EOL
        {
            extcmd->ct = COMMAND_EXIT;
            YYACCEPT;
        }
    |
    command EOL
        {
            if ($1->cmdType == COMMAND_LIST) {
                g_value_unsetfree($1->pvData);
                extcmd->ct = $1->cmdType;
                YYACCEPT;
            } else {
                GMap *optionsmap = (GMap *)g_value_get_boxed((GValue *)g_list_nth_data(g_value_get_boxed($1->pvData), 1));
                GList *boarddata = (GList *)g_value_get_boxed((GValue *)g_list_nth_data(g_value_get_boxed($1->pvData), 0));
                extcmd->ct = $1->cmdType;

                if (g_list_length(boarddata) <= 52) {
                    GVALUE_CREATE(G_TYPE_INT, int, 0, gvfalse); 
                    GVALUE_CREATE(G_TYPE_INT, int, 1, gvtrue); 
                    GVALUE_CREATE(G_TYPE_DOUBLE, double, 0.0, gvfloatzero); 

                    extcmd->bi.gsName = g_string_new(g_value_get_gstring_gchar(boarddata->data));
                    extcmd->bi.gsOpp = g_string_new(g_value_get_gstring_gchar(g_list_nth_data(boarddata, 1)));

                    GList *curbrdpos = g_list_nth(boarddata, 2);
                    int *curarraypos = extcmd->anList;
                    while (curbrdpos != NULL) {
                        *curarraypos++ = g_value_get_int(curbrdpos->data);                
                        curbrdpos = g_list_next(curbrdpos);
                    }

                    extcmd->nPlies = g_value_get_int(str2gv_map_get_key_value(optionsmap, "plies", gvfalse));
                    extcmd->fCrawfordRule = g_value_get_int(str2gv_map_get_key_value(optionsmap, "crawfordrule", gvfalse));
                    extcmd->fJacobyRule = g_value_get_int(str2gv_map_get_key_value(optionsmap, "jacobyrule", gvfalse));
                    extcmd->fUsePrune = g_value_get_int(str2gv_map_get_key_value(optionsmap, "prune", gvfalse));
                    extcmd->fCubeful =  g_value_get_int(str2gv_map_get_key_value(optionsmap, "cubeful", gvfalse));
                    extcmd->rNoise = g_value_get_double(str2gv_map_get_key_value(optionsmap, "noise", gvfloatzero));
                    extcmd->fDeterministic = g_value_get_int(str2gv_map_get_key_value(optionsmap, "deterministic", gvtrue));

                    g_value_unsetfree(gvtrue);
                    g_value_unsetfree(gvfalse);
                    g_value_unsetfree(gvfloatzero);
                    g_value_unsetfree($1->pvData);                
                    g_free($1);
                    
                    YYACCEPT;
                } else {
                    yyerror(scanner, "Invalid board. Maximum number of elements is 52");
                    g_value_unsetfree($1->pvData);                
                    g_free($1);
                    YYERROR;
                }
            }
        }
        ;
        
command:
    boardcommand
        {
            commandinfo *cmdInfo = g_malloc0(sizeof(commandinfo));
            cmdInfo->pvData = $1;
            cmdInfo->cmdType = COMMAND_FIBSBOARD;
            $$ = cmdInfo;
        }
    |
    evalcommand
        {
            commandinfo *cmdInfo = g_malloc0(sizeof(commandinfo));
            cmdInfo->pvData = $1;
            cmdInfo->cmdType = COMMAND_EVALUATION;
            $$ = cmdInfo;
        }
    |
    DISABLED list 
        { 
            GVALUE_CREATE(G_TYPE_BOXED_GLIST_GV, boxed, $2, gvptr);
            g_list_free($2);
            commandinfo *cmdInfo = g_malloc0(sizeof(commandinfo));
            cmdInfo->pvData = gvptr;
            cmdInfo->cmdType = COMMAND_LIST;
            $$ = cmdInfo;
        }
    ;
    
board_element:
    integer_type
    ;

board_elements: 
    board_element 
        { 
            $$ = g_list_prepend(NULL, $1); 
        }
    |
    board_elements ':' board_element 
        { 
            $$ = g_list_prepend($1, $3); 
        }
    ;
    
endboard:
    FIBSBOARDEND
    ;

sessionoption:
    JACOBYRULE boolean_type 
        { 
            $$ = create_str2gvalue_tuple ("jacobyrule", $2); 
        }
    | 
    CRAWFORDRULE boolean_type
        { 
            $$ = create_str2gvalue_tuple ("crawfordrule", $2);
        }
    ;
    
evaloption:
    PLIES integer_type
        { 
            $$ = create_str2gvalue_tuple ("plies", $2); 
        }
    | 
    NOISE float_type
        {
            $$ = create_str2gvalue_tuple ("noise", $2); 
        }
    |
    NOISE integer_type
        {
            double doubleval = g_value_get_int($2) / 10000.0L;
            GVALUE_CREATE(G_TYPE_DOUBLE, double, doubleval, gvdouble); 
            $$ = create_str2gvalue_tuple ("noise", gvdouble); 
            g_value_unsetfree($2);
        }
    |
    PRUNE
        { 
            $$ = create_str2int_tuple ("prune", TRUE);
        }
    |
    PRUNE boolean_type 
        { 
            $$ = create_str2gvalue_tuple ("prune", $2);
        }
    |
    DETERMINISTIC
        { 
            $$ = create_str2int_tuple ("deterministic", TRUE);
        }
    |
    DETERMINISTIC boolean_type 
        { 
            $$ = create_str2gvalue_tuple ("deterministic", $2);
        }
    |
    CUBE boolean_type 
        { 
            $$ = create_str2gvalue_tuple ("cubeful", $2);
        }
    |
    CUBEFUL 
        { 
            $$ = create_str2int_tuple ("cubeful", TRUE); 
        }
    |
    CUBELESS
        { 
            $$ = create_str2int_tuple ("cubeful", FALSE); 
        }
    ;

sessionoptions:
    /* Empty */
        { 
            /* Setup the defaults */
            STR2GV_MAPENTRY_CREATE("jacobyrule", fJacoby, G_TYPE_INT, 
                                    int, jacobyentry);
            STR2GV_MAPENTRY_CREATE("crawfordrule", TRUE, G_TYPE_INT, 
                                    int, crawfordentry);

            GList *defaults = 
                g_list_prepend(g_list_prepend(NULL, jacobyentry), crawfordentry);
            $$ = defaults;
        }
    |
    sessionoptions sessionoption
        { 
            STR2GV_MAP_ADD_ENTRY($1, $2, $$); 
        }
    ;

evaloptions:
    /* Empty */
        { 
            /* Setup the defaults */

            STR2GV_MAPENTRY_CREATE("jacobyrule", fJacoby, G_TYPE_INT, 
                                    int, jacobyentry);
            
            STR2GV_MAPENTRY_CREATE("crawfordrule", TRUE, G_TYPE_INT, 
                                    int, crawfordentry);

            GList *defaults = 
                g_list_prepend(g_list_prepend(NULL, jacobyentry), crawfordentry);

            $$ = defaults;
            
        }
    |
    evaloptions evaloption
        { 
            STR2GV_MAP_ADD_ENTRY($1, $2, $$); 
        }
    |
    evaloptions sessionoption
        { 
            STR2GV_MAP_ADD_ENTRY($1, $2, $$); 
        }
    ;

boardcommand:
    board sessionoptions
        {
            GVALUE_CREATE(G_TYPE_BOXED_GLIST_GV, boxed, $1, gvptr1);
            GVALUE_CREATE(G_TYPE_BOXED_MAP_GV, boxed, $2, gvptr2);
            GList *newList = g_list_prepend(g_list_prepend(NULL, gvptr2), gvptr1);  
            GVALUE_CREATE(G_TYPE_BOXED_GLIST_GV, boxed, newList, gvnewlist);
            $$ = gvnewlist;
            g_list_free(newList);
            g_list_free($1);
            g_list_free($2);
        }
    ;

evalcommand:
    EVALUATION FIBSBOARD board evaloptions
        {
            GVALUE_CREATE(G_TYPE_BOXED_GLIST_GV, boxed, $3, gvptr1);
            GVALUE_CREATE(G_TYPE_BOXED_MAP_GV, boxed, $4, gvptr2);
            
            GList *newList = g_list_prepend(g_list_prepend(NULL, gvptr2), gvptr1);  
            GVALUE_CREATE(G_TYPE_BOXED_GLIST_GV, boxed, newList, gvnewlist);
            $$ = gvnewlist;
            g_list_free(newList);
            g_list_free($3);
            g_list_free($4);
        }
    ;
        
board:
    FIBSBOARD EXTSTRING ':' EXTSTRING ':' board_elements endboard
        {
            GVALUE_CREATE(G_TYPE_GSTRING, boxed, $4, gvstr1); 
            GVALUE_CREATE(G_TYPE_GSTRING, boxed, $2, gvstr2); 
            $6 = g_list_reverse($6);
            $$ = g_list_prepend(g_list_prepend($6, gvstr1), gvstr2); 
            g_string_free($4, TRUE);
            g_string_free($2, TRUE);
        }
    ;
    
float_type:
    EXTFLOAT 
        { 
            GVALUE_CREATE(G_TYPE_DOUBLE, double, $1, gvfloat); 
            $$ = gvfloat; 
        }
    ;

string_type:
    EXTSTRING 
        { 
            GVALUE_CREATE(G_TYPE_GSTRING, boxed, $1, gvstr); 
            g_string_free ($1, TRUE); 
            $$ = gvstr; 
        }
    ;

integer_type:
    EXTINTEGER 
        { 
            GVALUE_CREATE(G_TYPE_INT, int, $1, gvint); 
            $$ = gvint; 
        }
    ;

boolean_type:
    EXTBOOLEAN 
        { 
            GVALUE_CREATE(G_TYPE_INT, int, $1, gvint); 
            $$ = gvint; 
        }
    ;

list_type:
    list 
        { 
            GVALUE_CREATE(G_TYPE_BOXED_GLIST_GV, boxed, $1, gvptr);
            g_list_free($1);
            $$ = gvptr;
        } 
    ;
    
    
basic_types:
    boolean_type | string_type | float_type | integer_type
    ;
    
    
list: 
    '(' list_elements ')' 
        { 
            $$ = g_list_reverse($2);
        }
    ;
    
list_element: 
    basic_types | list_type 
    ;
    
list_elements: 
    /* Empty */ 
        { 
            $$ = NULL; 
        }
    | 
    list_element 
        { 
            $$ = g_list_prepend(NULL, $1);
        }
    |
    list_elements ',' list_element 
        { 
            $$ = g_list_prepend($1, $3); 
        }
    ;
%%

#ifdef EXTERNAL_TEST

int main()
{
    scancontext scanctx;
    memset(&scanctx, 0, sizeof(scanctx));
    g_type_init ();
    StartParse(&scanctx);
    g_string_free(scanctx.bi.gsName, TRUE);
    g_string_free(scanctx.bi.gsOpp, TRUE);
    return 0;
}

#endif
