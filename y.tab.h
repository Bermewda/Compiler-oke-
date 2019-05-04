/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
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
    INTEGER = 258,
    STRING = 259,
    CHARACTER = 260,
    VARIABLE = 261,
    WHILE = 262,
    IF = 263,
    PUTI = 264,
    PUTH = 265,
    PUTC = 266,
    PUTS = 267,
    PUTI_ = 268,
    PUTH_ = 269,
    PUTC_ = 270,
    PUTS_ = 271,
    ARRAY = 272,
    ARRAY_DECLARE = 273,
    PARAM_ARRAY_DECLARE = 274,
    STRING_ARRAY_DECLARE = 275,
    IFX = 276,
    ELSE = 277,
    AND = 278,
    OR = 279,
    GE = 280,
    LE = 281,
    EQ = 282,
    NE = 283,
    UMINUS = 284
  };
#endif
/* Tokens.  */
#define INTEGER 258
#define STRING 259
#define CHARACTER 260
#define VARIABLE 261
#define WHILE 262
#define IF 263
#define PUTI 264
#define PUTH 265
#define PUTC 266
#define PUTS 267
#define PUTI_ 268
#define PUTH_ 269
#define PUTC_ 270
#define PUTS_ 271
#define ARRAY 272
#define ARRAY_DECLARE 273
#define PARAM_ARRAY_DECLARE 274
#define STRING_ARRAY_DECLARE 275
#define IFX 276
#define ELSE 277
#define AND 278
#define OR 279
#define GE 280
#define LE 281
#define EQ 282
#define NE 283
#define UMINUS 284

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 72 "mainParser.y" /* yacc.c:1909  */

    int iValue;                 /* integer value */
    char *sValue;		/* address of the string */
    char *vName;                /* symbol table index */
    nodeType *nPtr;             /* node pointer */

#line 119 "y.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
