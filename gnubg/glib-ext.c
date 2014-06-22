/*
 * glib-ext.c -- Map/GList extensions and utility functions for GLIB
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "glib-ext.h"

GLIBEXT_DEFINE_BOXED_TYPE(GListBoxed, g_list_boxed, g_list_copy, g_list_gv_boxed_free)
GLIBEXT_DEFINE_BOXED_TYPE(GMapBoxed, g_map_boxed, g_list_copy, g_list_gv_boxed_free)
GLIBEXT_DEFINE_BOXED_TYPE(GMapEntryBoxed, g_mapentry_boxed, g_list_copy, g_list_gv_boxed_free)

#if ! GLIB_CHECK_VERSION(2,28,0)
void
g_list_free_full(GList *list, GDestroyNotify free_func)
{
    g_list_foreach(list, (GFunc) free_func, NULL);
    g_list_free(list);
}
#endif

void
g_value_unsetfree(GValue * gv)
{
    g_value_unset(gv);
    g_free(gv);
}

GMapEntry *
str2gv_map_has_key(GMap * map, GString * key)
{
    int item;

    for (item = 0; item < g_list_length(map); item++) {
        GString *cmpkey = g_value_get_gstring(g_list_nth_data(g_value_get_boxed(g_list_nth_data(map, item)), 0));
        if (g_string_equal(cmpkey, key))
            return g_list_nth(map, item);
    }

    return NULL;
}

GValue *
str2gv_map_get_key_value(GMap * map, gchar * key, GValue * defaultgv)
{
    GString *gstrkey = g_string_new(key);
    GMapEntry *entry = str2gv_map_has_key(map, gstrkey);
    GValue *gv;

    if (entry)
        gv = g_list_nth_data(g_value_get_boxed(entry->data), 1);
    else
        gv = defaultgv;

    g_string_free(gstrkey, TRUE);

    return gv;
}

void
g_list_gv_free_full(gpointer data)
{
    g_assert(G_IS_VALUE(data));

    if (G_IS_VALUE(data))
        g_value_unsetfree(data);
}

void
g_list_gv_boxed_free(GList * list)
{
    g_list_free_full(list, (GDestroyNotify) g_list_gv_free_full);
}

GList *
create_str2int_tuple(char *str, int value)
{
    GString *tmpstr = g_string_new(str);
    GVALUE_CREATE(G_TYPE_INT, int, value, gvint);
    GVALUE_CREATE(G_TYPE_GSTRING, boxed, tmpstr, gvstr);
    g_string_free(tmpstr, TRUE);
    return g_list_prepend(g_list_prepend(NULL, gvint), gvstr);
}

GList *
create_str2gvalue_tuple(char *str, GValue * gv)
{
    GString *tmpstr = g_string_new(str);
    GVALUE_CREATE(G_TYPE_GSTRING, boxed, tmpstr, gvstr);
    g_string_free(tmpstr, TRUE);
    return g_list_prepend(g_list_prepend(NULL, gv), gvstr);
}

GList *
create_str2double_tuple(char *str, double value)
{
    GString *tmpstr = g_string_new(str);
    GVALUE_CREATE(G_TYPE_DOUBLE, float, value, gvdouble);
    GVALUE_CREATE(G_TYPE_GSTRING, boxed, tmpstr, gvstr);
    g_string_free(tmpstr, TRUE);
    return g_list_prepend(g_list_prepend(NULL, gvdouble), gvstr);
}

void
free_strmap_tuple(GList * tuple)
{
    g_string_free(g_list_nth_data(tuple, 0), TRUE);
    g_free(g_list_nth_data(tuple, 1));
    g_list_free(tuple);
}

void
g_value_print_list(GList * list, int depth)
{
    GList *cur = list;
    while (cur != NULL) {
        g_value_print(cur->data, depth);
        if ((cur = g_list_next(cur)))
            printf(", ");
    }
}

void
g_value_print(GValue * gv, int depth)
{
    if (!gv)
        return;
    g_assert(G_IS_VALUE(gv));

    if (G_VALUE_HOLDS_INT(gv)) {
        printf("%d", g_value_get_int(gv));
    } else if (G_VALUE_HOLDS_DOUBLE(gv)) {
        printf("%lf", g_value_get_double(gv));
    } else if (G_VALUE_HOLDS_GSTRING(gv)) {
        printf("\"%s\"", g_value_get_gstring_gchar(gv));
    } else if (G_VALUE_HOLDS_BOXED_GLIST_GV(gv)) {
        printf("(");
        g_value_print_list(g_value_get_boxed(gv), depth + 1);
        printf(")");
    } else if (G_VALUE_HOLDS_BOXED_MAP_GV(gv)) {
        printf("[");
        g_value_print_list(g_value_get_boxed(gv), depth + 1);
        printf("]");
    } else if (G_VALUE_HOLDS_BOXED_MAPENTRY_GV(gv)) {
        g_value_print(g_list_nth_data(g_value_get_boxed(gv), 0), depth + 1);
        printf(" : ");
        g_value_print(g_list_nth_data(g_value_get_boxed(gv), 1), depth + 1);
    }
}
