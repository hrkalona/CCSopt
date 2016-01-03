/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
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

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse         syntax_parser_parse
#define yylex           syntax_parser_lex
#define yyerror         syntax_parser_error
#define yylval          syntax_parser_lval
#define yychar          syntax_parser_char
#define yydebug         syntax_parser_debug
#define yynerrs         syntax_parser_nerrs


/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 3 "syntax_parser.y"

/******************************************************************************
    Copyright (c) 1996-2005 Synopsys, Inc.    ALL RIGHTS RESERVED

  The contents of this file are subject to the restrictions and limitations
  set forth in the SYNOPSYS Open Source License Version 1.0  (the "License"); 
  you may not use this file except in compliance with such restrictions 
  and limitations. You may obtain instructions on how to receive a copy of 
  the License at

  http://www.synopsys.com/partners/tapin/tapinprogram.html. 

  Software distributed by Original Contributor under the License is 
  distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either 
  expressed or implied. See the License for the specific language governing 
  rights and limitations under the License.

******************************************************************************/
#include <stdio.h>
#include "syntax.h"
#ifdef DMALLOC
#include <dmalloc.h>
#endif
#include "mymalloc.h"
	
static libsynt_group_info *gs[100];
static int gsindex = 0;

 liberty_hash_table *libsynt_groups;
 liberty_hash_table *libsynt_allgroups;
 liberty_hash_table *libsynt_attrs;

 libsynt_technology libsynt_techs[20];
 int libsynt_tech_count = 0;
 
void push_group(libsynt_head *h);
void pop_group(void);

extern int synt_lineno;

libsynt_attribute_info *make_complex(libsynt_head *h);
 
libsynt_attribute_info *make_simple(char *name, libsynt_attr_type type, void *constraint_ptr);
 

 static char *enumlist[100]; /* temps used in the process of building structs */
 static int enumcount;
 


/* Line 268 of yacc.c  */
#line 130 "syntax_parser.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     COMMA = 258,
     SEMI = 259,
     LPAR = 260,
     RPAR = 261,
     LCURLY = 262,
     RCURLY = 263,
     COLON = 264,
     LBRACK = 265,
     RBRACK = 266,
     KW_FLOAT = 267,
     KW_STRING = 268,
     KW_ENUM = 269,
     KW_UNK_ARGS = 270,
     KW_INTEGER = 271,
     KW_VIRTATTR = 272,
     KW_SHORT = 273,
     KW_OR = 274,
     KW_AND = 275,
     KW_BOOLEAN = 276,
     COLONEQ = 277,
     KW_LIST = 278,
     GREATERTHAN = 279,
     LESSTHAN = 280,
     DOTS = 281,
     NUM = 282,
     STRING = 283,
     IDENT = 284
   };
#endif
/* Tokens.  */
#define COMMA 258
#define SEMI 259
#define LPAR 260
#define RPAR 261
#define LCURLY 262
#define RCURLY 263
#define COLON 264
#define LBRACK 265
#define RBRACK 266
#define KW_FLOAT 267
#define KW_STRING 268
#define KW_ENUM 269
#define KW_UNK_ARGS 270
#define KW_INTEGER 271
#define KW_VIRTATTR 272
#define KW_SHORT 273
#define KW_OR 274
#define KW_AND 275
#define KW_BOOLEAN 276
#define COLONEQ 277
#define KW_LIST 278
#define GREATERTHAN 279
#define LESSTHAN 280
#define DOTS 281
#define NUM 282
#define STRING 283
#define IDENT 284




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 293 of yacc.c  */
#line 53 "syntax_parser.y"

	char *str;
	double num;
	void *voidptr;
	libsynt_attr_type attrtype;
	libsynt_float_constraint *float_constr;
	libsynt_int_constraint *int_constr;
	libsynt_string_enum *str_enum;
	libsynt_argument *arg;
	libsynt_attribute_info *attr;
	libsynt_name_constraints nameconstr;
	libsynt_group_info *group;
	libsynt_head *head;
	libsynt_technology *tech;



/* Line 293 of yacc.c  */
#line 242 "syntax_parser.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 254 "syntax_parser.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
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
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  8
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   152

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  30
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  18
/* YYNRULES -- Number of rules.  */
#define YYNRULES  70
/* YYNRULES -- Number of states.  */
#define YYNSTATES  133

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   284

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     6,    12,    13,    18,    20,    23,
      25,    27,    29,    34,    39,    45,    51,    56,    62,    67,
      73,    78,    84,    89,    93,    95,    99,   104,   109,   114,
     122,   125,   132,   139,   149,   152,   159,   166,   176,   181,
     186,   194,   197,   204,   211,   221,   224,   231,   238,   248,
     251,   256,   261,   263,   265,   266,   268,   272,   280,   284,
     290,   292,   294,   296,   298,   300,   304,   308,   312,   316,
     318
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      31,     0,    -1,    32,    -1,    -1,    43,     7,    33,    35,
       8,    -1,    -1,    43,     7,    34,     8,    -1,    36,    -1,
      35,    36,    -1,    37,    -1,    42,    -1,    32,    -1,    29,
       9,    17,     4,    -1,    29,     9,    13,     4,    -1,    29,
       9,    13,    38,     4,    -1,    29,     9,    12,    40,     4,
      -1,    29,     9,    12,     4,    -1,    29,     9,    14,    38,
       4,    -1,    29,     9,    18,     4,    -1,    29,     9,    18,
      41,     4,    -1,    29,     9,    16,     4,    -1,    29,     9,
      16,    41,     4,    -1,    29,     9,    21,     4,    -1,     5,
      39,     6,    -1,    47,    -1,    39,     3,    47,    -1,    39,
       3,    19,    47,    -1,     5,    24,    27,     6,    -1,     5,
      25,    27,     6,    -1,     5,    24,    27,    20,    25,    27,
       6,    -1,    22,    27,    -1,     5,    24,    27,     6,    22,
      27,    -1,     5,    25,    27,     6,    22,    27,    -1,     5,
      24,    27,    20,    25,    27,     6,    22,    27,    -1,    22,
      47,    -1,     5,    24,    27,     6,    22,    47,    -1,     5,
      25,    27,     6,    22,    47,    -1,     5,    24,    27,    20,
      25,    27,     6,    22,    47,    -1,     5,    24,    27,     6,
      -1,     5,    25,    27,     6,    -1,     5,    24,    27,    20,
      25,    27,     6,    -1,    22,    27,    -1,     5,    24,    27,
       6,    22,    27,    -1,     5,    25,    27,     6,    22,    27,
      -1,     5,    24,    27,    20,    25,    27,     6,    22,    27,
      -1,    22,    47,    -1,     5,    24,    27,     6,    22,    47,
      -1,     5,    25,    27,     6,    22,    47,    -1,     5,    24,
      27,    20,    25,    27,     6,    22,    47,    -1,    43,     4,
      -1,    29,     5,    44,     6,    -1,    18,     5,    44,     6,
      -1,    45,    -1,    46,    -1,    -1,    29,    -1,    10,    29,
      11,    -1,    29,    10,     3,    26,     3,    29,    11,    -1,
      29,     3,    29,    -1,    29,     3,    29,     3,    29,    -1,
      13,    -1,    16,    -1,    12,    -1,    15,    -1,    23,    -1,
      46,     3,    13,    -1,    46,     3,    16,    -1,    46,     3,
      12,    -1,    46,     3,    23,    -1,    28,    -1,    29,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    90,    90,    93,    93,    94,    94,   100,   101,   105,
     106,   107,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   123,   127,   128,   129,   133,   134,   135,
     137,   138,   139,   140,   142,   143,   144,   145,   149,   150,
     151,   153,   154,   155,   156,   158,   159,   160,   161,   167,
     170,   171,   175,   176,   179,   180,   181,   182,   183,   184,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   200,
     201
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "COMMA", "SEMI", "LPAR", "RPAR",
  "LCURLY", "RCURLY", "COLON", "LBRACK", "RBRACK", "KW_FLOAT", "KW_STRING",
  "KW_ENUM", "KW_UNK_ARGS", "KW_INTEGER", "KW_VIRTATTR", "KW_SHORT",
  "KW_OR", "KW_AND", "KW_BOOLEAN", "COLONEQ", "KW_LIST", "GREATERTHAN",
  "LESSTHAN", "DOTS", "NUM", "STRING", "IDENT", "$accept", "file", "group",
  "$@1", "$@2", "statements", "statement", "simple_attr", "string_enum",
  "s_or_i_list", "float_constraint", "int_constraint", "complex_attr",
  "head", "arg_or_namelist", "namelist", "arglist", "s_or_i", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    30,    31,    33,    32,    34,    32,    35,    35,    36,
      36,    36,    37,    37,    37,    37,    37,    37,    37,    37,
      37,    37,    37,    38,    39,    39,    39,    40,    40,    40,
      40,    40,    40,    40,    40,    40,    40,    40,    41,    41,
      41,    41,    41,    41,    41,    41,    41,    41,    41,    42,
      43,    43,    44,    44,    45,    45,    45,    45,    45,    45,
      46,    46,    46,    46,    46,    46,    46,    46,    46,    47,
      47
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     0,     5,     0,     4,     1,     2,     1,
       1,     1,     4,     4,     5,     5,     4,     5,     4,     5,
       4,     5,     4,     3,     1,     3,     4,     4,     4,     7,
       2,     6,     6,     9,     2,     6,     6,     9,     4,     4,
       7,     2,     6,     6,     9,     2,     6,     6,     9,     2,
       4,     4,     1,     1,     0,     1,     3,     7,     3,     5,
       1,     1,     1,     1,     1,     3,     3,     3,     3,     1,
       1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     2,     0,    54,    54,     1,     3,
       0,    62,    60,    63,    61,    64,    55,     0,    52,    53,
       0,     0,     0,     0,     0,     0,    51,     0,    50,     0,
      11,     0,     7,     9,    10,     0,     6,    56,    58,     0,
      67,    65,    66,    68,     0,     4,     8,    49,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    59,     0,    16,
       0,     0,     0,    13,     0,     0,     0,    20,     0,     0,
       0,    12,    18,     0,    22,     0,     0,     0,    30,    69,
      70,    34,    15,     0,    24,    14,    17,     0,     0,    41,
      45,    21,    19,    57,     0,     0,     0,    23,     0,     0,
      27,     0,    28,     0,    25,    38,     0,    39,     0,     0,
       0,    26,     0,     0,     0,    31,    35,     0,    32,    36,
      42,    46,     0,    43,    47,    29,    40,     0,     0,    33,
      37,    44,    48
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     3,    30,    21,    22,    31,    32,    33,    65,    83,
      62,    70,    34,    35,    17,    18,    19,    81
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -65
static const yytype_int8 yypact[] =
{
     -10,     9,    26,    49,   -65,    76,    -6,    -6,   -65,    46,
      67,   -65,   -65,   -65,   -65,   -65,    37,    91,   -65,    95,
      93,    16,    92,    90,    73,   100,   -65,    13,   -65,    32,
     -65,    -7,   -65,   -65,   -65,    54,   -65,   -65,   101,    79,
     -65,   -65,   -65,   -65,    39,   -65,   -65,   -65,    77,   104,
      -2,    86,   103,     8,   105,    11,   106,   -65,    82,   -65,
      68,    38,   108,   -65,    14,   109,   110,   -65,    70,    41,
     111,   -65,   -65,   112,   -65,   107,    94,    96,   -65,   -65,
     -65,   -65,   -65,    56,   -65,   -65,   -65,    97,    98,   -65,
     -65,   -65,   -65,   -65,    15,   113,    -1,   -65,    18,   114,
     115,   102,   116,    14,   -65,   117,   118,   119,    44,    99,
      47,   -65,    50,   120,    53,   -65,   -65,   122,   -65,   -65,
     -65,   -65,   123,   -65,   -65,   124,   126,    57,    60,   -65,
     -65,   -65,   -65
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -65,   -65,   130,   -65,   -65,   -65,   121,   -65,    65,   -65,
     -65,    78,   -65,   131,   125,   -65,   -65,   -64
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -6
static const yytype_int16 yytable[] =
{
      84,    45,    59,    60,    10,    90,    11,    12,     1,    13,
      14,     1,    67,    68,     6,    72,    68,    15,   103,     2,
      61,   100,    29,    16,   105,    40,    41,    79,    80,    42,
      69,     7,   104,    69,     1,   101,    43,     7,   106,   111,
      24,    44,    79,    80,   116,    29,   119,    25,   121,     8,
     124,    50,    51,    52,    -5,    53,    54,    55,    47,    96,
      56,     9,    97,   130,   132,    78,    79,    80,    89,    79,
      80,   115,    79,    80,   118,    79,    80,   120,    79,    80,
     123,    79,    80,     9,   129,    79,    80,   131,    79,    80,
      63,    64,    76,    77,    87,    88,    23,    26,    27,    28,
      36,    37,    38,    39,    48,    49,    57,    58,    64,    71,
      74,    75,    82,    85,    86,    91,    92,    66,    93,   102,
     107,    94,     0,    95,    98,    99,   117,   109,   125,   126,
       4,     5,    20,    73,     0,     0,     0,   108,   110,   112,
       0,   114,     0,   113,     0,     0,   127,   122,   128,     0,
       0,     0,    46
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-65))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
      64,     8,     4,     5,    10,    69,    12,    13,    18,    15,
      16,    18,     4,     5,     5,     4,     5,    23,    19,    29,
      22,     6,    29,    29,     6,    12,    13,    28,    29,    16,
      22,     5,    96,    22,    18,    20,    23,     5,    20,   103,
       3,     9,    28,    29,   108,    29,   110,    10,   112,     0,
     114,    12,    13,    14,     8,    16,    17,    18,     4,     3,
      21,     7,     6,   127,   128,    27,    28,    29,    27,    28,
      29,    27,    28,    29,    27,    28,    29,    27,    28,    29,
      27,    28,    29,     7,    27,    28,    29,    27,    28,    29,
       4,     5,    24,    25,    24,    25,    29,     6,     3,     6,
       8,    11,    29,     3,     3,    26,    29,     3,     5,     4,
       4,    29,     4,     4,     4,     4,     4,    52,    11,     6,
       6,    27,    -1,    27,    27,    27,    27,    25,     6,     6,
       0,     0,     7,    55,    -1,    -1,    -1,    22,    22,    22,
      -1,    22,    -1,    25,    -1,    -1,    22,    27,    22,    -1,
      -1,    -1,    31
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    18,    29,    31,    32,    43,     5,     5,     0,     7,
      10,    12,    13,    15,    16,    23,    29,    44,    45,    46,
      44,    33,    34,    29,     3,    10,     6,     3,     6,    29,
      32,    35,    36,    37,    42,    43,     8,    11,    29,     3,
      12,    13,    16,    23,     9,     8,    36,     4,     3,    26,
      12,    13,    14,    16,    17,    18,    21,    29,     3,     4,
       5,    22,    40,     4,     5,    38,    38,     4,     5,    22,
      41,     4,     4,    41,     4,    29,    24,    25,    27,    28,
      29,    47,     4,    39,    47,     4,     4,    24,    25,    27,
      47,     4,     4,    11,    27,    27,     3,     6,    27,    27,
       6,    20,     6,    19,    47,     6,    20,     6,    22,    25,
      22,    47,    22,    25,    22,    27,    47,    27,    27,    47,
      27,    47,    27,    27,    47,     6,     6,    22,    22,    27,
      47,    27,    47
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

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
#ifndef	YYINITDEPTH
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
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
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
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
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
	    /* Fall through.  */
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

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
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
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
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
      int yyn = yypact[*yyssp];
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
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

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
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

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
      yychar = YYLEX;
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

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

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
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

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

/* Line 1806 of yacc.c  */
#line 90 "syntax_parser.y"
    { libsynt_techs[libsynt_tech_count++].lib = (yyvsp[(1) - (1)].group); }
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 93 "syntax_parser.y"
    {push_group((yyvsp[(1) - (2)].head));}
    break;

  case 4:

/* Line 1806 of yacc.c  */
#line 93 "syntax_parser.y"
    {(yyval.group) = gs[gsindex-1];pop_group();}
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 94 "syntax_parser.y"
    {push_group((yyvsp[(1) - (2)].head));}
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 94 "syntax_parser.y"
    {si2drObjectIdT toid;  (yyval.group) = gs[gsindex-1]; liberty_hash_lookup(libsynt_groups, gs[gsindex-1]->type, &toid);
                                                if( toid.v1 != (void*)NULL )(yyval.group)->ref =(libsynt_group_info*)toid.v1;
                                                else {printf("Error: line %d: Couldn't find group %s\n",(yyvsp[(1) - (4)].head)->lineno,(yyvsp[(1) - (4)].head)->ident); } pop_group();}
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 100 "syntax_parser.y"
    {}
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 101 "syntax_parser.y"
    {}
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 105 "syntax_parser.y"
    {si2drObjectIdT toid; toid.v1 = (void*)(yyvsp[(1) - (1)].attr);liberty_hash_enter_oid(gs[gsindex-1]->attr_hash, (yyvsp[(1) - (1)].attr)->name, toid);(yyvsp[(1) - (1)].attr)->next = gs[gsindex-1]->attr_list; gs[gsindex-1]->attr_list = (yyvsp[(1) - (1)].attr); }
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 106 "syntax_parser.y"
    {si2drObjectIdT toid;toid.v1 = (void*)(yyvsp[(1) - (1)].attr);liberty_hash_enter_oid(gs[gsindex-1]->attr_hash, (yyvsp[(1) - (1)].attr)->name, toid);(yyvsp[(1) - (1)].attr)->next = gs[gsindex-1]->attr_list; gs[gsindex-1]->attr_list = (yyvsp[(1) - (1)].attr);}
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 107 "syntax_parser.y"
    {si2drObjectIdT toid;toid.v1 = (void*)(yyvsp[(1) - (1)].group);liberty_hash_enter_oid(gs[gsindex-1]->group_hash, (yyvsp[(1) - (1)].group)->type,toid);(yyvsp[(1) - (1)].group)->next = gs[gsindex-1]->group_list; gs[gsindex-1]->group_list = (yyvsp[(1) - (1)].group);}
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 110 "syntax_parser.y"
    { (yyval.attr) = make_simple((yyvsp[(1) - (4)].str),SYNTAX_ATTRTYPE_VIRTUAL,0);}
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 111 "syntax_parser.y"
    { (yyval.attr) = make_simple((yyvsp[(1) - (4)].str),SYNTAX_ATTRTYPE_STRING,0);}
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 112 "syntax_parser.y"
    { (yyval.attr) = make_simple((yyvsp[(1) - (5)].str),SYNTAX_ATTRTYPE_ENUM, (yyvsp[(4) - (5)].str_enum));}
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 113 "syntax_parser.y"
    { (yyval.attr) = make_simple((yyvsp[(1) - (5)].str),SYNTAX_ATTRTYPE_FLOAT,(yyvsp[(4) - (5)].float_constr));}
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 114 "syntax_parser.y"
    { (yyval.attr) = make_simple((yyvsp[(1) - (4)].str),SYNTAX_ATTRTYPE_FLOAT,0);}
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 115 "syntax_parser.y"
    { (yyval.attr) = make_simple((yyvsp[(1) - (5)].str),SYNTAX_ATTRTYPE_ENUM, (yyvsp[(4) - (5)].str_enum));}
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 116 "syntax_parser.y"
    { (yyval.attr) = make_simple((yyvsp[(1) - (4)].str),SYNTAX_ATTRTYPE_INT,0);}
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 117 "syntax_parser.y"
    { (yyval.attr) = make_simple((yyvsp[(1) - (5)].str),SYNTAX_ATTRTYPE_INT,(yyvsp[(4) - (5)].int_constr));}
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 118 "syntax_parser.y"
    { (yyval.attr) = make_simple((yyvsp[(1) - (4)].str),SYNTAX_ATTRTYPE_INT,0);}
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 119 "syntax_parser.y"
    { (yyval.attr) = make_simple((yyvsp[(1) - (5)].str),SYNTAX_ATTRTYPE_INT,(yyvsp[(4) - (5)].int_constr));}
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 120 "syntax_parser.y"
    { (yyval.attr) = make_simple((yyvsp[(1) - (4)].str),SYNTAX_ATTRTYPE_BOOLEAN,0);}
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 123 "syntax_parser.y"
    { int i; (yyval.str_enum) = (libsynt_string_enum*)my_calloc(sizeof(libsynt_string_enum),1); (yyval.str_enum)->array = (char**)my_calloc(sizeof(char*),enumcount+1);
                                            for(i=0;i<enumcount; i++){(yyval.str_enum)->array[i] = enumlist[i];} (yyval.str_enum)->size = enumcount;}
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 127 "syntax_parser.y"
    { enumcount=0; enumlist[enumcount++] = (yyvsp[(1) - (1)].str);}
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 128 "syntax_parser.y"
    { enumlist[enumcount++] = (yyvsp[(3) - (3)].str);}
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 129 "syntax_parser.y"
    { enumlist[enumcount++] = (yyvsp[(4) - (4)].str);}
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 133 "syntax_parser.y"
    {(yyval.float_constr) = (libsynt_float_constraint*)my_calloc(sizeof(libsynt_float_constraint),1); (yyval.float_constr)->greater_than = (yyvsp[(3) - (4)].num); (yyval.float_constr)->greater_specd =1; }
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 134 "syntax_parser.y"
    {(yyval.float_constr) = (libsynt_float_constraint*)my_calloc(sizeof(libsynt_float_constraint),1); (yyval.float_constr)->less_than = (yyvsp[(3) - (4)].num); (yyval.float_constr)->less_specd = 1; }
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 135 "syntax_parser.y"
    {(yyval.float_constr) = (libsynt_float_constraint*)my_calloc(sizeof(libsynt_float_constraint),1);
                                                 (yyval.float_constr)->less_than = (yyvsp[(6) - (7)].num); (yyval.float_constr)->less_specd = 1;(yyval.float_constr)->greater_than = (yyvsp[(3) - (7)].num); (yyval.float_constr)->greater_specd =1;}
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 137 "syntax_parser.y"
    {(yyval.float_constr) = (libsynt_float_constraint*)my_calloc(sizeof(libsynt_float_constraint),1); (yyval.float_constr)->default_value = (yyvsp[(2) - (2)].num); (yyval.float_constr)->default_specd =1;}
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 138 "syntax_parser.y"
    {(yyval.float_constr) = (libsynt_float_constraint*)my_calloc(sizeof(libsynt_float_constraint),1); (yyval.float_constr)->default_value = (yyvsp[(6) - (6)].num); (yyval.float_constr)->default_specd =1; (yyval.float_constr)->greater_than = (yyvsp[(3) - (6)].num); (yyval.float_constr)->greater_specd =1; }
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 139 "syntax_parser.y"
    {(yyval.float_constr) = (libsynt_float_constraint*)my_calloc(sizeof(libsynt_float_constraint),1); (yyval.float_constr)->default_value = (yyvsp[(6) - (6)].num); (yyval.float_constr)->default_specd =1;(yyval.float_constr)->less_than = (yyvsp[(3) - (6)].num); (yyval.float_constr)->less_specd = 1; }
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 140 "syntax_parser.y"
    {(yyval.float_constr) = (libsynt_float_constraint*)my_calloc(sizeof(libsynt_float_constraint),1); (yyval.float_constr)->default_value = (yyvsp[(9) - (9)].num);
                                                 (yyval.float_constr)->less_than = (yyvsp[(6) - (9)].num); (yyval.float_constr)->less_specd = 1;(yyval.float_constr)->greater_than = (yyvsp[(3) - (9)].num); (yyval.float_constr)->greater_specd =1; (yyval.float_constr)->default_specd =1;}
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 142 "syntax_parser.y"
    {(yyval.float_constr) = (libsynt_float_constraint*)my_calloc(sizeof(libsynt_float_constraint),1); (yyval.float_constr)->refd_default = (yyvsp[(2) - (2)].str); (yyval.float_constr)->ref_default_specd =1;}
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 143 "syntax_parser.y"
    {(yyval.float_constr) = (libsynt_float_constraint*)my_calloc(sizeof(libsynt_float_constraint),1); (yyval.float_constr)->refd_default = (yyvsp[(6) - (6)].str);  (yyval.float_constr)->ref_default_specd =1;(yyval.float_constr)->greater_than = (yyvsp[(3) - (6)].num); (yyval.float_constr)->greater_specd =1; }
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 144 "syntax_parser.y"
    {(yyval.float_constr) = (libsynt_float_constraint*)my_calloc(sizeof(libsynt_float_constraint),1); (yyval.float_constr)->refd_default = (yyvsp[(6) - (6)].str); (yyval.float_constr)->ref_default_specd =1;(yyval.float_constr)->less_than = (yyvsp[(3) - (6)].num); (yyval.float_constr)->less_specd = 1; }
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 145 "syntax_parser.y"
    {(yyval.float_constr) = (libsynt_float_constraint*)my_calloc(sizeof(libsynt_float_constraint),1); (yyval.float_constr)->refd_default = (yyvsp[(9) - (9)].str);
                                                 (yyval.float_constr)->less_than = (yyvsp[(6) - (9)].num); (yyval.float_constr)->less_specd = 1;(yyval.float_constr)->greater_than = (yyvsp[(3) - (9)].num); (yyval.float_constr)->greater_specd =1; (yyval.float_constr)->ref_default_specd =1;}
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 149 "syntax_parser.y"
    {(yyval.int_constr) = (libsynt_int_constraint*)my_calloc(sizeof(libsynt_int_constraint),1); (yyval.int_constr)->greater_than = (yyvsp[(3) - (4)].num); (yyval.int_constr)->greater_specd =1; }
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 150 "syntax_parser.y"
    {(yyval.int_constr) = (libsynt_int_constraint*)my_calloc(sizeof(libsynt_int_constraint),1); (yyval.int_constr)->less_than = (yyvsp[(3) - (4)].num); (yyval.int_constr)->less_specd = 1; }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 151 "syntax_parser.y"
    {(yyval.int_constr) = (libsynt_int_constraint*)my_calloc(sizeof(libsynt_int_constraint),1);
                                                                (yyval.int_constr)->less_than = (yyvsp[(6) - (7)].num); (yyval.int_constr)->less_specd = 1;(yyval.int_constr)->greater_than = (yyvsp[(3) - (7)].num); (yyval.int_constr)->greater_specd =1;}
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 153 "syntax_parser.y"
    {(yyval.int_constr) = (libsynt_int_constraint*)my_calloc(sizeof(libsynt_int_constraint),1); (yyval.int_constr)->default_value = (yyvsp[(2) - (2)].num); (yyval.int_constr)->default_specd =1;}
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 154 "syntax_parser.y"
    {(yyval.int_constr) = (libsynt_int_constraint*)my_calloc(sizeof(libsynt_int_constraint),1); (yyval.int_constr)->default_value = (yyvsp[(6) - (6)].num); (yyval.int_constr)->default_specd =1; (yyval.int_constr)->greater_than = (yyvsp[(3) - (6)].num); (yyval.int_constr)->greater_specd =1; }
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 155 "syntax_parser.y"
    {(yyval.int_constr) = (libsynt_int_constraint*)my_calloc(sizeof(libsynt_int_constraint),1); (yyval.int_constr)->default_value = (yyvsp[(6) - (6)].num); (yyval.int_constr)->default_specd =1;(yyval.int_constr)->less_than = (yyvsp[(3) - (6)].num); (yyval.int_constr)->less_specd = 1; }
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 156 "syntax_parser.y"
    {(yyval.int_constr) = (libsynt_int_constraint*)my_calloc(sizeof(libsynt_int_constraint),1); (yyval.int_constr)->default_value = (yyvsp[(9) - (9)].num);
                                                                            (yyval.int_constr)->less_than = (yyvsp[(6) - (9)].num); (yyval.int_constr)->less_specd = 1;(yyval.int_constr)->greater_than = (yyvsp[(3) - (9)].num); (yyval.int_constr)->greater_specd =1; (yyval.int_constr)->default_specd =1;}
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 158 "syntax_parser.y"
    {(yyval.int_constr) = (libsynt_int_constraint*)my_calloc(sizeof(libsynt_int_constraint),1); (yyval.int_constr)->refd_default = (yyvsp[(2) - (2)].str); (yyval.int_constr)->ref_default_specd =1;}
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 159 "syntax_parser.y"
    {(yyval.int_constr) = (libsynt_int_constraint*)my_calloc(sizeof(libsynt_int_constraint),1); (yyval.int_constr)->refd_default = (yyvsp[(6) - (6)].str);  (yyval.int_constr)->ref_default_specd =1;(yyval.int_constr)->greater_than = (yyvsp[(3) - (6)].num); (yyval.int_constr)->greater_specd =1; }
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 160 "syntax_parser.y"
    {(yyval.int_constr) = (libsynt_int_constraint*)my_calloc(sizeof(libsynt_int_constraint),1); (yyval.int_constr)->refd_default = (yyvsp[(6) - (6)].str); (yyval.int_constr)->ref_default_specd =1;(yyval.int_constr)->less_than = (yyvsp[(3) - (6)].num); (yyval.int_constr)->less_specd = 1; }
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 161 "syntax_parser.y"
    {(yyval.int_constr) = (libsynt_int_constraint*)my_calloc(sizeof(libsynt_int_constraint),1); (yyval.int_constr)->refd_default = (yyvsp[(9) - (9)].str);
                                                                      (yyval.int_constr)->less_than = (yyvsp[(6) - (9)].num); (yyval.int_constr)->less_specd = 1;(yyval.int_constr)->greater_than = (yyvsp[(3) - (9)].num); (yyval.int_constr)->greater_specd =1; (yyval.int_constr)->ref_default_specd =1;}
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 167 "syntax_parser.y"
    { (yyval.attr)= make_complex((yyvsp[(1) - (2)].head));}
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 170 "syntax_parser.y"
    { (yyval.head) = (yyvsp[(3) - (4)].head); (yyval.head)->ident = (yyvsp[(1) - (4)].str); (yyval.head)->lineno = synt_lineno;}
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 171 "syntax_parser.y"
    { (yyval.head) = (yyvsp[(3) - (4)].head); (yyval.head)->ident = "short"; (yyval.head)->lineno = synt_lineno;}
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 175 "syntax_parser.y"
    {(yyval.head) = my_calloc(sizeof(libsynt_head),1); (yyval.head)->namecons = (yyvsp[(1) - (1)].nameconstr);}
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 176 "syntax_parser.y"
    {(yyval.head) = my_calloc(sizeof(libsynt_head),1); (yyval.head)->arglist = (yyvsp[(1) - (1)].arg);}
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 179 "syntax_parser.y"
    {(yyval.nameconstr) = SYNTAX_GNAME_NONE;}
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 180 "syntax_parser.y"
    {(yyval.nameconstr) = SYNTAX_GNAME_ONE;}
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 181 "syntax_parser.y"
    {(yyval.nameconstr) = SYNTAX_GNAME_NONE_OR_ONE;}
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 182 "syntax_parser.y"
    {(yyval.nameconstr) = SYNTAX_GNAME_ONE_OR_MORE;}
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 183 "syntax_parser.y"
    {(yyval.nameconstr) = SYNTAX_GNAME_TWO;}
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 184 "syntax_parser.y"
    {(yyval.nameconstr) = SYNTAX_GNAME_THREE;}
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 187 "syntax_parser.y"
    { (yyval.arg)=my_calloc(sizeof(libsynt_argument),1); (yyval.arg)->type = SYNTAX_ATTRTYPE_STRING;}
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 188 "syntax_parser.y"
    {(yyval.arg)=my_calloc(sizeof(libsynt_argument),1); (yyval.arg)->type = SYNTAX_ATTRTYPE_INT;}
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 189 "syntax_parser.y"
    {(yyval.arg)=my_calloc(sizeof(libsynt_argument),1); (yyval.arg)->type = SYNTAX_ATTRTYPE_FLOAT;}
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 190 "syntax_parser.y"
    {(yyval.arg)=my_calloc(sizeof(libsynt_argument),1); (yyval.arg)->type = SYNTAX_ATTRTYPE_COMPLEX_UNKNOWN;}
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 191 "syntax_parser.y"
    {(yyval.arg)=my_calloc(sizeof(libsynt_argument),1); (yyval.arg)->type = SYNTAX_ATTRTYPE_COMPLEX_LIST;}
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 192 "syntax_parser.y"
    {libsynt_argument *x= my_calloc(sizeof(libsynt_argument),1),*y; x->type = SYNTAX_ATTRTYPE_STRING; for(y=(yyval.arg);y;y=y->next){if( !y->next){y->next=x;break;}}(yyval.arg)=(yyvsp[(1) - (3)].arg);}
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 193 "syntax_parser.y"
    {libsynt_argument *x= my_calloc(sizeof(libsynt_argument),1),*y; x->type = SYNTAX_ATTRTYPE_INT;for(y=(yyval.arg);y;y=y->next){if( !y->next){y->next=x;break;}}(yyval.arg)=(yyvsp[(1) - (3)].arg);}
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 194 "syntax_parser.y"
    {libsynt_argument *x= my_calloc(sizeof(libsynt_argument),1),*y; x->type = SYNTAX_ATTRTYPE_FLOAT;for(y=(yyval.arg);y;y=y->next){if( !y->next){y->next=x;break;}}(yyval.arg)=(yyvsp[(1) - (3)].arg);}
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 195 "syntax_parser.y"
    {libsynt_argument *x= my_calloc(sizeof(libsynt_argument),1),*y; x->type = SYNTAX_ATTRTYPE_COMPLEX_LIST;for(y=(yyval.arg);y;y=y->next){if( !y->next){y->next=x;break;}}(yyval.arg)=(yyvsp[(1) - (3)].arg);}
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 200 "syntax_parser.y"
    {(yyval.str) = (yyvsp[(1) - (1)].str);}
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 201 "syntax_parser.y"
    {(yyval.str)=(yyvsp[(1) - (1)].str);}
    break;



/* Line 1806 of yacc.c  */
#line 2096 "syntax_parser.c"
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

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
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
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
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
        yyerror (yymsgp);
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
		      yytoken, &yylval);
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

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
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
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

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
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


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

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
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
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 2067 of yacc.c  */
#line 204 "syntax_parser.y"


void push_group(libsynt_head *h )
{
	si2drObjectIdT toid,noid;
	
	gs[gsindex] = (libsynt_group_info*)my_calloc(sizeof(libsynt_group_info),1);

	toid.v1 = (void*)gs[gsindex];

	gs[gsindex]->lineno = h->lineno;

	gs[gsindex]->type = h->ident;

	gs[gsindex]->name_constraints = h->namecons;

	gs[gsindex]->attr_hash = liberty_hash_create_hash_table(41, 1, 0);

	gs[gsindex]->group_hash = liberty_hash_create_hash_table(41, 1, 0);

	liberty_hash_lookup(libsynt_groups, h->ident, &noid);
	if( noid.v1 == (void*)NULL )
		liberty_hash_enter_oid(libsynt_groups, h->ident, toid);
 
	liberty_hash_lookup(libsynt_allgroups, h->ident, &noid);
	if( noid.v1 == (void*)NULL )
		liberty_hash_enter_oid(libsynt_allgroups, h->ident, toid);
 
	gsindex++;
}

void pop_group(void)
{
	gsindex--;
}

libsynt_attribute_info *make_complex(libsynt_head *h)
{
	si2drObjectIdT toid,noid;
	libsynt_attribute_info *x = my_calloc(sizeof(libsynt_attribute_info),1);
	x->name = h->ident;
	toid.v1 = (void*)x;
	
	if( h->arglist->type == SYNTAX_ATTRTYPE_COMPLEX_UNKNOWN )
		x->type = SYNTAX_ATTRTYPE_COMPLEX_UNKNOWN;
	else if( h->arglist->type == SYNTAX_ATTRTYPE_COMPLEX_LIST )
		x->type = SYNTAX_ATTRTYPE_COMPLEX_LIST;
	else
		x->type = SYNTAX_ATTRTYPE_COMPLEX;
	x->u.args = h->arglist;
	liberty_hash_lookup(libsynt_attrs, h->ident, &noid);
	if( noid.v1 == (void*)NULL )
		liberty_hash_enter_oid(libsynt_attrs, h->ident, toid);
	return x;
}

libsynt_attribute_info *make_simple(char *name, libsynt_attr_type type, void *constraint_ptr)
{
	si2drObjectIdT toid,noid;
	libsynt_attribute_info *x = my_calloc(sizeof(libsynt_attribute_info),1);
	x->name = name;
	x->type = type;
	toid.v1 = (void*)x;
	switch( type )
	{
	case SYNTAX_ATTRTYPE_STRING:
		break;
	case SYNTAX_ATTRTYPE_VIRTUAL:
		break;
	case SYNTAX_ATTRTYPE_ENUM:
		x->u.stringenum = (libsynt_string_enum*)constraint_ptr;
		break;
	case SYNTAX_ATTRTYPE_FLOAT:
		x->u.floatcon = (libsynt_float_constraint*)constraint_ptr;
		break;
	case SYNTAX_ATTRTYPE_INT:
		x->u.intcon = (libsynt_int_constraint*)constraint_ptr;
		break;
	case SYNTAX_ATTRTYPE_BOOLEAN:
		break;
	default:
		break;
	}
	liberty_hash_lookup(libsynt_attrs, x->name, &noid);
	if( noid.v1 == (void*)NULL )
		liberty_hash_enter_oid(libsynt_attrs, x->name, toid);
	return x;
}

int syntax_parser_error(char *);

int yyerror(char *s)
{
	fprintf(stderr,"%s line number %d\n", s, synt_lineno);
	return 0;
}

