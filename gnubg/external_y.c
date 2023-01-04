/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.5.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "external_y.y"

/*
 * Copyright (C) 2014-2015 Michael Petch <mpetch@gnubg.org>
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
 * $Id: external_y.y,v 1.36 2020/02/20 21:19:52 plm Exp $
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

#include "common.h"
#include "glib-ext.h"
#include "external.h"
#include "backgammon.h"
#include "external_y.h"

/* Resolve a warning on older GLIBC/GNU systems that have stpcpy */
#if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE && !defined(__USE_XOPEN2K8)
extern char *stpcpy(char *s1, const char *s2);
#endif

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


#line 175 "external_y.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_YY_EXTERNAL_Y_H_INCLUDED
# define YY_YY_EXTERNAL_Y_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    EOL = 258,
    EXIT = 259,
    DISABLED = 260,
    INTERFACEVERSION = 261,
    DEBUG = 262,
    SET = 263,
    NEW = 264,
    OLD = 265,
    OUTPUT = 266,
    E_INTERFACE = 267,
    HELP = 268,
    PROMPT = 269,
    E_STRING = 270,
    E_CHARACTER = 271,
    E_INTEGER = 272,
    E_FLOAT = 273,
    E_BOOLEAN = 274,
    FIBSBOARD = 275,
    FIBSBOARDEND = 276,
    EVALUATION = 277,
    CRAWFORDRULE = 278,
    JACOBYRULE = 279,
    RESIGNATION = 280,
    BEAVERS = 281,
    CUBE = 282,
    CUBEFUL = 283,
    CUBELESS = 284,
    DETERMINISTIC = 285,
    NOISE = 286,
    PLIES = 287,
    PRUNE = 288
  };
#endif
/* Tokens.  */
#define EOL 258
#define EXIT 259
#define DISABLED 260
#define INTERFACEVERSION 261
#define DEBUG 262
#define SET 263
#define NEW 264
#define OLD 265
#define OUTPUT 266
#define E_INTERFACE 267
#define HELP 268
#define PROMPT 269
#define E_STRING 270
#define E_CHARACTER 271
#define E_INTEGER 272
#define E_FLOAT 273
#define E_BOOLEAN 274
#define FIBSBOARD 275
#define FIBSBOARDEND 276
#define EVALUATION 277
#define CRAWFORDRULE 278
#define JACOBYRULE 279
#define RESIGNATION 280
#define BEAVERS 281
#define CUBE 282
#define CUBEFUL 283
#define CUBELESS 284
#define DETERMINISTIC 285
#define NOISE 286
#define PLIES 287
#define PRUNE 288

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 114 "external_y.y"

    gboolean bool;
    gchar character;
    gfloat floatnum;
    gint intnum;
    GString *str;
    GValue *gv;
    GList *list;
    commandinfo *cmd;

#line 304 "external_y.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int yyparse (scancontext *scanner);

#endif /* !YY_YY_EXTERNAL_Y_H_INCLUDED  */

/* Second part of user prologue.  */
#line 125 "external_y.y"


#line 322 "external_y.c"


#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))

/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  25
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   74

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  38
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  23
/* YYNRULES -- Number of rules.  */
#define YYNRULES  55
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  85

#define YYUNDEFTOK  2
#define YYMAXUTOK   288


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      35,    36,     2,     2,    37,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    34,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   173,   173,   179,   186,   192,   198,   204,   258,   263,
     269,   275,   282,   290,   298,   310,   314,   319,   326,   330,
     335,   340,   345,   352,   357,   362,   370,   375,   380,   385,
     390,   395,   400,   408,   425,   433,   450,   455,   462,   476,
     491,   503,   511,   520,   528,   536,   546,   546,   546,   546,
     551,   558,   558,   563,   567,   572
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "EOL", "EXIT", "DISABLED",
  "INTERFACEVERSION", "DEBUG", "SET", "NEW", "OLD", "OUTPUT",
  "E_INTERFACE", "HELP", "PROMPT", "E_STRING", "E_CHARACTER", "E_INTEGER",
  "E_FLOAT", "E_BOOLEAN", "FIBSBOARD", "FIBSBOARDEND", "EVALUATION",
  "CRAWFORDRULE", "JACOBYRULE", "RESIGNATION", "BEAVERS", "CUBE",
  "CUBEFUL", "CUBELESS", "DETERMINISTIC", "NOISE", "PLIES", "PRUNE", "':'",
  "'('", "')'", "','", "$accept", "commands", "setcommand", "command",
  "board_element", "board_elements", "endboard", "sessionoption",
  "evaloption", "sessionoptions", "evaloptions", "boardcommand",
  "evalcommand", "board", "float_type", "string_type", "integer_type",
  "boolean_type", "list_type", "basic_types", "list", "list_element",
  "list_elements", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,    58,    40,    41,    44
};
# endif

#define YYPACT_NINF (-49)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
       3,   -49,     7,   -23,    15,    10,    26,     0,    19,    32,
      41,   -49,   -49,   -49,   -49,   -14,   -49,   -49,    27,    18,
      34,    44,   -49,    16,    43,   -49,   -49,    12,   -49,   -49,
     -49,   -49,   -49,   -49,   -49,   -49,   -49,   -49,   -49,   -49,
       4,   -49,   -49,   -49,   -49,   -49,    49,   -49,    27,    27,
      48,    27,   -49,   -49,   -14,    33,    29,   -49,   -49,   -49,
     -49,   -49,    48,    27,   -49,   -49,    27,    25,    48,    27,
     -49,   -49,   -49,    -8,   -49,   -49,   -49,   -49,   -49,   -49,
     -49,   -49,    48,   -49,   -49
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     2,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,    33,     6,    53,    14,     4,     0,     0,
       0,     0,     5,     0,     0,     1,     7,    38,    42,    43,
      41,    44,    48,    47,    49,    46,    52,    51,    45,    54,
       0,     8,     9,    10,    11,     3,     0,    35,     0,     0,
       0,     0,    34,    50,     0,     0,    39,    20,    19,    21,
      22,    55,     0,     0,    31,    32,    28,     0,     0,    26,
      37,    36,    16,     0,    15,    30,    29,    24,    25,    23,
      27,    18,     0,    40,    17
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -49,   -49,   -49,   -49,   -16,   -49,   -49,    13,   -49,   -49,
     -49,   -49,   -49,    46,     1,    51,   -48,   -18,   -49,   -49,
      69,    20,   -49
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     9,    21,    10,    72,    73,    83,    52,    71,    27,
      56,    11,    12,    13,    32,    33,    34,    35,    36,    37,
      38,    39,    40
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      41,    28,    59,    29,    30,    31,     1,     2,     3,     4,
      14,     5,    15,    81,    74,    23,     6,    18,    17,    78,
      79,    15,    19,     7,    20,     8,    82,    42,    43,    22,
      57,    58,    25,    60,    74,    48,    49,    50,    51,    24,
      53,    54,    29,    30,    26,    75,    31,    45,    76,    28,
      46,    80,    48,    49,    50,    51,    63,    64,    65,    66,
      67,    68,    69,     7,    55,    29,    84,    62,    77,    70,
      47,    44,    16,     0,    61
};

static const yytype_int8 yycheck[] =
{
      18,    15,    50,    17,    18,    19,     3,     4,     5,     6,
       3,     8,    35,    21,    62,    15,    13,     7,     3,    67,
      68,    35,    12,    20,    14,    22,    34,     9,    10,     3,
      48,    49,     0,    51,    82,    23,    24,    25,    26,    20,
      36,    37,    17,    18,     3,    63,    19,     3,    66,    15,
      34,    69,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    20,    15,    17,    82,    34,    67,    56,
      24,    20,     3,    -1,    54
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     4,     5,     6,     8,    13,    20,    22,    39,
      41,    49,    50,    51,     3,    35,    58,     3,     7,    12,
      14,    40,     3,    15,    20,     0,     3,    47,    15,    17,
      18,    19,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    55,     9,    10,    53,     3,    34,    51,    23,    24,
      25,    26,    45,    36,    37,    15,    48,    55,    55,    54,
      55,    59,    34,    27,    28,    29,    30,    31,    32,    33,
      45,    46,    42,    43,    54,    55,    55,    52,    54,    54,
      55,    21,    34,    44,    42
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    38,    39,    39,    39,    39,    39,    39,    40,    40,
      40,    40,    41,    41,    41,    42,    43,    43,    44,    45,
      45,    45,    45,    46,    46,    46,    46,    46,    46,    46,
      46,    46,    46,    47,    47,    48,    48,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    57,    57,    57,
      58,    59,    59,    60,    60,    60
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     3,     2,     2,     2,     2,     2,     2,
       2,     2,     1,     1,     2,     1,     1,     3,     1,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     1,     2,
       2,     1,     1,     0,     2,     0,     2,     2,     2,     4,
       7,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     1,     1,     0,     1,     3
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (scanner, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, scanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep, scancontext *scanner)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  YYUSE (scanner);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep, scancontext *scanner)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyo, yytype, yyvaluep, scanner);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, int yyrule, scancontext *scanner)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[+yyssp[yyi + 1 - yynrhs]],
                       &yyvsp[(yyi + 1) - (yynrhs)]
                                              , scanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, scanner); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
#  else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                yy_state_t *yyssp, int yytoken)
{
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Actual size of YYARG. */
  int yycount = 0;
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[+*yyssp];
      YYPTRDIFF_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
      yysize = yysize0;
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYPTRDIFF_T yysize1
                    = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    /* Don't count the "%s"s in the final size, but reserve room for
       the terminator.  */
    YYPTRDIFF_T yysize1 = yysize + (yystrlen (yyformat) - 2 * yycount) + 1;
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, scancontext *scanner)
{
  YYUSE (yyvaluep);
  YYUSE (scanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yytype)
    {
    case 15: /* E_STRING  */
#line 166 "external_y.y"
            { if (((*yyvaluep).str)) g_string_free(((*yyvaluep).str), TRUE); }
#line 1285 "external_y.c"
        break;

    case 40: /* setcommand  */
#line 167 "external_y.y"
            { if (((*yyvaluep).list)) g_list_free(((*yyvaluep).list)); }
#line 1291 "external_y.c"
        break;

    case 41: /* command  */
#line 169 "external_y.y"
            { if (((*yyvaluep).cmd)) { g_free(((*yyvaluep).cmd)); }}
#line 1297 "external_y.c"
        break;

    case 42: /* board_element  */
#line 168 "external_y.y"
            { if (((*yyvaluep).gv)) { g_value_unsetfree(((*yyvaluep).gv)); }}
#line 1303 "external_y.c"
        break;

    case 43: /* board_elements  */
#line 167 "external_y.y"
            { if (((*yyvaluep).list)) g_list_free(((*yyvaluep).list)); }
#line 1309 "external_y.c"
        break;

    case 45: /* sessionoption  */
#line 167 "external_y.y"
            { if (((*yyvaluep).list)) g_list_free(((*yyvaluep).list)); }
#line 1315 "external_y.c"
        break;

    case 46: /* evaloption  */
#line 167 "external_y.y"
            { if (((*yyvaluep).list)) g_list_free(((*yyvaluep).list)); }
#line 1321 "external_y.c"
        break;

    case 47: /* sessionoptions  */
#line 167 "external_y.y"
            { if (((*yyvaluep).list)) g_list_free(((*yyvaluep).list)); }
#line 1327 "external_y.c"
        break;

    case 48: /* evaloptions  */
#line 167 "external_y.y"
            { if (((*yyvaluep).list)) g_list_free(((*yyvaluep).list)); }
#line 1333 "external_y.c"
        break;

    case 49: /* boardcommand  */
#line 168 "external_y.y"
            { if (((*yyvaluep).gv)) { g_value_unsetfree(((*yyvaluep).gv)); }}
#line 1339 "external_y.c"
        break;

    case 50: /* evalcommand  */
#line 168 "external_y.y"
            { if (((*yyvaluep).gv)) { g_value_unsetfree(((*yyvaluep).gv)); }}
#line 1345 "external_y.c"
        break;

    case 51: /* board  */
#line 167 "external_y.y"
            { if (((*yyvaluep).list)) g_list_free(((*yyvaluep).list)); }
#line 1351 "external_y.c"
        break;

    case 52: /* float_type  */
#line 168 "external_y.y"
            { if (((*yyvaluep).gv)) { g_value_unsetfree(((*yyvaluep).gv)); }}
#line 1357 "external_y.c"
        break;

    case 53: /* string_type  */
#line 168 "external_y.y"
            { if (((*yyvaluep).gv)) { g_value_unsetfree(((*yyvaluep).gv)); }}
#line 1363 "external_y.c"
        break;

    case 54: /* integer_type  */
#line 168 "external_y.y"
            { if (((*yyvaluep).gv)) { g_value_unsetfree(((*yyvaluep).gv)); }}
#line 1369 "external_y.c"
        break;

    case 55: /* boolean_type  */
#line 168 "external_y.y"
            { if (((*yyvaluep).gv)) { g_value_unsetfree(((*yyvaluep).gv)); }}
#line 1375 "external_y.c"
        break;

    case 56: /* list_type  */
#line 168 "external_y.y"
            { if (((*yyvaluep).gv)) { g_value_unsetfree(((*yyvaluep).gv)); }}
#line 1381 "external_y.c"
        break;

    case 57: /* basic_types  */
#line 168 "external_y.y"
            { if (((*yyvaluep).gv)) { g_value_unsetfree(((*yyvaluep).gv)); }}
#line 1387 "external_y.c"
        break;

    case 58: /* list  */
#line 167 "external_y.y"
            { if (((*yyvaluep).list)) g_list_free(((*yyvaluep).list)); }
#line 1393 "external_y.c"
        break;

    case 59: /* list_element  */
#line 168 "external_y.y"
            { if (((*yyvaluep).gv)) { g_value_unsetfree(((*yyvaluep).gv)); }}
#line 1399 "external_y.c"
        break;

    case 60: /* list_elements  */
#line 167 "external_y.y"
            { if (((*yyvaluep).list)) g_list_free(((*yyvaluep).list)); }
#line 1405 "external_y.c"
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (scancontext *scanner)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYPTRDIFF_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, scanner);
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2:
#line 174 "external_y.y"
        {
            extcmd->ct = COMMAND_NONE;
            YYACCEPT;
        }
#line 1682 "external_y.c"
    break;

  case 3:
#line 180 "external_y.y"
        {
            extcmd->pCmdData = (yyvsp[-1].list);
            extcmd->ct = COMMAND_SET;
            YYACCEPT;
        }
#line 1692 "external_y.c"
    break;

  case 4:
#line 187 "external_y.y"
        {
            extcmd->ct = COMMAND_VERSION;
            YYACCEPT;
        }
#line 1701 "external_y.c"
    break;

  case 5:
#line 193 "external_y.y"
        {
            extcmd->ct = COMMAND_HELP;
            YYACCEPT;
        }
#line 1710 "external_y.c"
    break;

  case 6:
#line 199 "external_y.y"
        {
            extcmd->ct = COMMAND_EXIT;
            YYACCEPT;
        }
#line 1719 "external_y.c"
    break;

  case 7:
#line 205 "external_y.y"
        {
            if ((yyvsp[-1].cmd)->cmdType == COMMAND_LIST) {
                g_value_unsetfree((yyvsp[-1].cmd)->pvData);
                extcmd->ct = (yyvsp[-1].cmd)->cmdType;
                YYACCEPT;
            } else {
                GMap *optionsmap = (GMap *)g_value_get_boxed((GValue *)g_list_nth_data(g_value_get_boxed((yyvsp[-1].cmd)->pvData), 1));
                GList *boarddata = (GList *)g_value_get_boxed((GValue *)g_list_nth_data(g_value_get_boxed((yyvsp[-1].cmd)->pvData), 0));
                extcmd->ct = (yyvsp[-1].cmd)->cmdType;
                extcmd->pCmdData = (yyvsp[-1].cmd)->pvData;

                if (g_list_length(boarddata) < MAX_RFBF_ELEMENTS) {
                    GVALUE_CREATE(G_TYPE_INT, int, 0, gvfalse); 
                    GVALUE_CREATE(G_TYPE_INT, int, 1, gvtrue); 
                    GVALUE_CREATE(G_TYPE_FLOAT, float, 0.0, gvfloatzero); 

                    extcmd->bi.gsName = g_string_new(g_value_get_gstring_gchar(boarddata->data));
                    extcmd->bi.gsOpp = g_string_new(g_value_get_gstring_gchar(g_list_nth_data(boarddata, 1)));

                    GList *curbrdpos = g_list_nth(boarddata, 2);
                    int *curarraypos = extcmd->anList;
                    while (curbrdpos != NULL) {
                        *curarraypos++ = g_value_get_int(curbrdpos->data);                
                        curbrdpos = g_list_next(curbrdpos);
                    }

                    extcmd->nPlies = g_value_get_int(str2gv_map_get_key_value(optionsmap, KEY_STR_PLIES, gvfalse));
                    extcmd->fCrawfordRule = g_value_get_int(str2gv_map_get_key_value(optionsmap, KEY_STR_CRAWFORDRULE, gvfalse));
                    extcmd->fJacobyRule = g_value_get_int(str2gv_map_get_key_value(optionsmap, KEY_STR_JACOBYRULE, gvfalse));
                    extcmd->fUsePrune = g_value_get_int(str2gv_map_get_key_value(optionsmap, KEY_STR_PRUNE, gvfalse));
                    extcmd->fCubeful =  g_value_get_int(str2gv_map_get_key_value(optionsmap, KEY_STR_CUBEFUL, gvfalse));
                    extcmd->rNoise = g_value_get_float(str2gv_map_get_key_value(optionsmap, KEY_STR_NOISE, gvfloatzero));
                    extcmd->fDeterministic = g_value_get_int(str2gv_map_get_key_value(optionsmap, KEY_STR_DETERMINISTIC, gvtrue));
                    extcmd->nResignation = g_value_get_int(str2gv_map_get_key_value(optionsmap, KEY_STR_RESIGNATION, gvfalse));
                    extcmd->fBeavers = g_value_get_int(str2gv_map_get_key_value(optionsmap, KEY_STR_BEAVERS, gvtrue));

                    g_value_unsetfree(gvtrue);
                    g_value_unsetfree(gvfalse);
                    g_value_unsetfree(gvfloatzero);
                    g_free((yyvsp[-1].cmd));
                    
                    YYACCEPT;
                } else {
                    yyerror(scanner, "Invalid board. Maximum number of elements is 52");
                    g_value_unsetfree((yyvsp[-1].cmd)->pvData);                
                    g_free((yyvsp[-1].cmd));
                    YYERROR;
                }
            }
        }
#line 1774 "external_y.c"
    break;

  case 8:
#line 259 "external_y.y"
        {
            (yyval.list) = create_str2gvalue_tuple (KEY_STR_DEBUG, (yyvsp[0].gv));
        }
#line 1782 "external_y.c"
    break;

  case 9:
#line 264 "external_y.y"
        {
            GVALUE_CREATE(G_TYPE_INT, int, 1, gvint); 
            (yyval.list) = create_str2gvalue_tuple (KEY_STR_NEWINTERFACE, gvint);
        }
#line 1791 "external_y.c"
    break;

  case 10:
#line 270 "external_y.y"
        {
            GVALUE_CREATE(G_TYPE_INT, int, 0, gvint); 
            (yyval.list) = create_str2gvalue_tuple (KEY_STR_NEWINTERFACE, gvint);
        }
#line 1800 "external_y.c"
    break;

  case 11:
#line 276 "external_y.y"
        {
            (yyval.list) = create_str2gvalue_tuple (KEY_STR_PROMPT, (yyvsp[0].gv));
        }
#line 1808 "external_y.c"
    break;

  case 12:
#line 283 "external_y.y"
        {
            commandinfo *cmdInfo = g_malloc0(sizeof(commandinfo));
            cmdInfo->pvData = (yyvsp[0].gv);
            cmdInfo->cmdType = COMMAND_FIBSBOARD;
            (yyval.cmd) = cmdInfo;
        }
#line 1819 "external_y.c"
    break;

  case 13:
#line 291 "external_y.y"
        {
            commandinfo *cmdInfo = g_malloc0(sizeof(commandinfo));
            cmdInfo->pvData = (yyvsp[0].gv);
            cmdInfo->cmdType = COMMAND_EVALUATION;
            (yyval.cmd) = cmdInfo;
        }
#line 1830 "external_y.c"
    break;

  case 14:
#line 299 "external_y.y"
        { 
            GVALUE_CREATE(G_TYPE_BOXED_GLIST_GV, boxed, (yyvsp[0].list), gvptr);
            g_list_free((yyvsp[0].list));
            commandinfo *cmdInfo = g_malloc0(sizeof(commandinfo));
            cmdInfo->pvData = gvptr;
            cmdInfo->cmdType = COMMAND_LIST;
            (yyval.cmd) = cmdInfo;
        }
#line 1843 "external_y.c"
    break;

  case 16:
#line 315 "external_y.y"
        { 
            (yyval.list) = g_list_prepend(NULL, (yyvsp[0].gv)); 
        }
#line 1851 "external_y.c"
    break;

  case 17:
#line 320 "external_y.y"
        { 
            (yyval.list) = g_list_prepend((yyvsp[-2].list), (yyvsp[0].gv)); 
        }
#line 1859 "external_y.c"
    break;

  case 19:
#line 331 "external_y.y"
        { 
            (yyval.list) = create_str2gvalue_tuple (KEY_STR_JACOBYRULE, (yyvsp[0].gv)); 
        }
#line 1867 "external_y.c"
    break;

  case 20:
#line 336 "external_y.y"
        { 
            (yyval.list) = create_str2gvalue_tuple (KEY_STR_CRAWFORDRULE, (yyvsp[0].gv));
        }
#line 1875 "external_y.c"
    break;

  case 21:
#line 341 "external_y.y"
        { 
            (yyval.list) = create_str2gvalue_tuple (KEY_STR_RESIGNATION, (yyvsp[0].gv));
        }
#line 1883 "external_y.c"
    break;

  case 22:
#line 346 "external_y.y"
        { 
            (yyval.list) = create_str2gvalue_tuple (KEY_STR_BEAVERS, (yyvsp[0].gv));
        }
#line 1891 "external_y.c"
    break;

  case 23:
#line 353 "external_y.y"
        { 
            (yyval.list) = create_str2gvalue_tuple (KEY_STR_PLIES, (yyvsp[0].gv)); 
        }
#line 1899 "external_y.c"
    break;

  case 24:
#line 358 "external_y.y"
        {
            (yyval.list) = create_str2gvalue_tuple (KEY_STR_NOISE, (yyvsp[0].gv)); 
        }
#line 1907 "external_y.c"
    break;

  case 25:
#line 363 "external_y.y"
        {
            float floatval = (float) g_value_get_int((yyvsp[0].gv)) / 10000.0f;
            GVALUE_CREATE(G_TYPE_FLOAT, float, floatval, gvfloat); 
            (yyval.list) = create_str2gvalue_tuple (KEY_STR_NOISE, gvfloat); 
            g_value_unsetfree((yyvsp[0].gv));
        }
#line 1918 "external_y.c"
    break;

  case 26:
#line 371 "external_y.y"
        { 
            (yyval.list) = create_str2int_tuple (KEY_STR_PRUNE, TRUE);
        }
#line 1926 "external_y.c"
    break;

  case 27:
#line 376 "external_y.y"
        { 
            (yyval.list) = create_str2gvalue_tuple (KEY_STR_PRUNE, (yyvsp[0].gv));
        }
#line 1934 "external_y.c"
    break;

  case 28:
#line 381 "external_y.y"
        { 
            (yyval.list) = create_str2int_tuple (KEY_STR_DETERMINISTIC, TRUE);
        }
#line 1942 "external_y.c"
    break;

  case 29:
#line 386 "external_y.y"
        { 
            (yyval.list) = create_str2gvalue_tuple (KEY_STR_DETERMINISTIC, (yyvsp[0].gv));
        }
#line 1950 "external_y.c"
    break;

  case 30:
#line 391 "external_y.y"
        { 
            (yyval.list) = create_str2gvalue_tuple (KEY_STR_CUBEFUL, (yyvsp[0].gv));
        }
#line 1958 "external_y.c"
    break;

  case 31:
#line 396 "external_y.y"
        { 
            (yyval.list) = create_str2int_tuple (KEY_STR_CUBEFUL, TRUE); 
        }
#line 1966 "external_y.c"
    break;

  case 32:
#line 401 "external_y.y"
        { 
            (yyval.list) = create_str2int_tuple (KEY_STR_CUBEFUL, FALSE); 
        }
#line 1974 "external_y.c"
    break;

  case 33:
#line 408 "external_y.y"
        { 
            /* Setup the defaults */
            STR2GV_MAPENTRY_CREATE(KEY_STR_JACOBYRULE, fJacoby, G_TYPE_INT, 
                                    int, jacobyentry);
            STR2GV_MAPENTRY_CREATE(KEY_STR_CRAWFORDRULE, TRUE, G_TYPE_INT, 
                                    int, crawfordentry);
            STR2GV_MAPENTRY_CREATE(KEY_STR_RESIGNATION, FALSE, G_TYPE_INT, 
                                    int, resignentry);
            STR2GV_MAPENTRY_CREATE(KEY_STR_BEAVERS, TRUE, G_TYPE_INT, 
                                    int, beaversentry);

            GList *defaults = 
                g_list_prepend(g_list_prepend(g_list_prepend(g_list_prepend(NULL, jacobyentry), crawfordentry), \
                               resignentry), beaversentry);
            (yyval.list) = defaults;
        }
#line 1995 "external_y.c"
    break;

  case 34:
#line 426 "external_y.y"
        { 
            STR2GV_MAP_ADD_ENTRY((yyvsp[-1].list), (yyvsp[0].list), (yyval.list)); 
        }
#line 2003 "external_y.c"
    break;

  case 35:
#line 433 "external_y.y"
        { 
            /* Setup the defaults */
            STR2GV_MAPENTRY_CREATE(KEY_STR_JACOBYRULE, fJacoby, G_TYPE_INT, 
                                    int, jacobyentry);
            STR2GV_MAPENTRY_CREATE(KEY_STR_CRAWFORDRULE, TRUE, G_TYPE_INT, 
                                    int, crawfordentry);
            STR2GV_MAPENTRY_CREATE(KEY_STR_RESIGNATION, FALSE, G_TYPE_INT, 
                                    int, resignentry);
            STR2GV_MAPENTRY_CREATE(KEY_STR_BEAVERS, TRUE, G_TYPE_INT, 
                                    int, beaversentry);

            GList *defaults = 
                g_list_prepend(g_list_prepend(g_list_prepend(g_list_prepend(NULL, jacobyentry), crawfordentry), \
                               resignentry), beaversentry);
            (yyval.list) = defaults;
        }
#line 2024 "external_y.c"
    break;

  case 36:
#line 451 "external_y.y"
        { 
            STR2GV_MAP_ADD_ENTRY((yyvsp[-1].list), (yyvsp[0].list), (yyval.list)); 
        }
#line 2032 "external_y.c"
    break;

  case 37:
#line 456 "external_y.y"
        { 
            STR2GV_MAP_ADD_ENTRY((yyvsp[-1].list), (yyvsp[0].list), (yyval.list)); 
        }
#line 2040 "external_y.c"
    break;

  case 38:
#line 463 "external_y.y"
        {
            GVALUE_CREATE(G_TYPE_BOXED_GLIST_GV, boxed, (yyvsp[-1].list), gvptr1);
            GVALUE_CREATE(G_TYPE_BOXED_MAP_GV, boxed, (yyvsp[0].list), gvptr2);
            GList *newList = g_list_prepend(g_list_prepend(NULL, gvptr2), gvptr1);  
            GVALUE_CREATE(G_TYPE_BOXED_GLIST_GV, boxed, newList, gvnewlist);
            (yyval.gv) = gvnewlist;
            g_list_free(newList);
            g_list_free((yyvsp[-1].list));
            g_list_free((yyvsp[0].list));
        }
#line 2055 "external_y.c"
    break;

  case 39:
#line 477 "external_y.y"
        {
            GVALUE_CREATE(G_TYPE_BOXED_GLIST_GV, boxed, (yyvsp[-1].list), gvptr1);
            GVALUE_CREATE(G_TYPE_BOXED_MAP_GV, boxed, (yyvsp[0].list), gvptr2);
            
            GList *newList = g_list_prepend(g_list_prepend(NULL, gvptr2), gvptr1);  
            GVALUE_CREATE(G_TYPE_BOXED_GLIST_GV, boxed, newList, gvnewlist);
            (yyval.gv) = gvnewlist;
            g_list_free(newList);
            g_list_free((yyvsp[-1].list));
            g_list_free((yyvsp[0].list));
        }
#line 2071 "external_y.c"
    break;

  case 40:
#line 492 "external_y.y"
        {
            GVALUE_CREATE(G_TYPE_GSTRING, boxed, (yyvsp[-3].str), gvstr1); 
            GVALUE_CREATE(G_TYPE_GSTRING, boxed, (yyvsp[-5].str), gvstr2); 
            (yyvsp[-1].list) = g_list_reverse((yyvsp[-1].list));
            (yyval.list) = g_list_prepend(g_list_prepend((yyvsp[-1].list), gvstr1), gvstr2); 
            g_string_free((yyvsp[-3].str), TRUE);
            g_string_free((yyvsp[-5].str), TRUE);
        }
#line 2084 "external_y.c"
    break;

  case 41:
#line 504 "external_y.y"
        { 
            GVALUE_CREATE(G_TYPE_FLOAT, float, (yyvsp[0].floatnum), gvfloat); 
            (yyval.gv) = gvfloat; 
        }
#line 2093 "external_y.c"
    break;

  case 42:
#line 512 "external_y.y"
        { 
            GVALUE_CREATE(G_TYPE_GSTRING, boxed, (yyvsp[0].str), gvstr); 
            g_string_free ((yyvsp[0].str), TRUE); 
            (yyval.gv) = gvstr; 
        }
#line 2103 "external_y.c"
    break;

  case 43:
#line 521 "external_y.y"
        { 
            GVALUE_CREATE(G_TYPE_INT, int, (yyvsp[0].intnum), gvint); 
            (yyval.gv) = gvint; 
        }
#line 2112 "external_y.c"
    break;

  case 44:
#line 529 "external_y.y"
        { 
            GVALUE_CREATE(G_TYPE_INT, int, (yyvsp[0].bool), gvint); 
            (yyval.gv) = gvint; 
        }
#line 2121 "external_y.c"
    break;

  case 45:
#line 537 "external_y.y"
        { 
            GVALUE_CREATE(G_TYPE_BOXED_GLIST_GV, boxed, (yyvsp[0].list), gvptr);
            g_list_free((yyvsp[0].list));
            (yyval.gv) = gvptr;
        }
#line 2131 "external_y.c"
    break;

  case 50:
#line 552 "external_y.y"
        { 
            (yyval.list) = g_list_reverse((yyvsp[-1].list));
        }
#line 2139 "external_y.c"
    break;

  case 53:
#line 563 "external_y.y"
        { 
            (yyval.list) = NULL; 
        }
#line 2147 "external_y.c"
    break;

  case 54:
#line 568 "external_y.y"
        { 
            (yyval.list) = g_list_prepend(NULL, (yyvsp[0].gv));
        }
#line 2155 "external_y.c"
    break;

  case 55:
#line 573 "external_y.y"
        { 
            (yyval.list) = g_list_prepend((yyvsp[-2].list), (yyvsp[0].gv)); 
        }
#line 2163 "external_y.c"
    break;


#line 2167 "external_y.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (scanner, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *, YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (scanner, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, scanner);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, scanner);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (scanner, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, scanner);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[+*yyssp], yyvsp, scanner);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 577 "external_y.y"


#ifdef EXTERNAL_TEST

/* 
 * Test code can be built by configuring GNUbg with --without-gtk option and doing the following:
 *
 * ./ylwrap external_l.l lex.yy.c external_l.c -- flex 
 * ./ylwrap external_y.y y.tab.c external_y.c y.tab.h test1_y.h -- bison 
 * gcc -Ilib -I. -Wall `pkg-config  gobject-2.0 --cflags --libs` external_l.c external_y.c  glib-ext.c -DEXTERNAL_TEST -o exttest
 *
 */
 
#define BUFFERSIZE 1024

int fJacoby = TRUE;

int main()
{
    char buffer[BUFFERSIZE];
    scancontext scanctx;

    memset(&scanctx, 0, sizeof(scanctx));
    g_type_init ();
    ExtInitParse((void **)&scanctx);

    while(fgets(buffer, BUFFERSIZE, stdin) != NULL) {
        ExtStartParse(scanctx.scanner, buffer);
        if(scanctx.ct == COMMAND_EXIT)
            return 0;
        
        if (scanctx.bi.gsName)
            g_string_free(scanctx.bi.gsName, TRUE);
        if (scanctx.bi.gsOpp)
            g_string_free(scanctx.bi.gsOpp, TRUE);

        scanctx.bi.gsName = NULL;
        scanctx.bi.gsOpp = NULL;
    }

    ExtDestroyParse(scanctx.scanner);
    return 0;
}

#endif
