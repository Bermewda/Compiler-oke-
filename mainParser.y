%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "main.h"

#define TABLE_SIZE 40000

/* prototypes */
nodeType *opr(int oper, int nops, ...);
nodeType *id(char *var_name, int isGlobal);
nodeType *con(int value);
nodeType *charCon(char *value);
nodeType *strCon(char *value);
nodeType* addOperand(nodeType* p1,nodeType* p2);
nodeType * OneDArray(nodeType * name, nodeType * index);
void freeNode(nodeType *p);
int ex(nodeType *p, int, int, int);
void eop();
int yylex(void);

void yyerror(char *s);

// variable related
char* sym[TABLE_SIZE]; //symbol table
int var_count = 0; //variable count
int loc_var_count = 0;//count for local variable


int inSYM(char *var_name);
void insertSYM(char * var_name, int isGlobal);
int getSYMIdx(char *var_name, int isGlobal);
void emptySYM(int isGlobal);
void printsp(int coun);
void prepass(nodeType *p, int infunc);
void printStackTop(int type);
void insertArraySYM(char * array_name, int array_size, int is_global);
int checkType (int index);
void insertFUNC(char * func_name);
int getFUNCIdx(char * func_name);
void emptyFUNC();
void insertArg(int argnum, int idx);
void setType (int index, int t);
int checkExprType (nodeType* p);

//helper functions
void printSYM();
// function related
char* func[TABLE_SIZE]; //function table
int func_count = 0; //function count
int argTable[TABLE_SIZE];

void insertFUNC(char *var_name);
int getFUNCIdx(char *var_name);
void emptyFUNC();

void insertArg(int argnum, int idx);
// variable type related
int vType[TABLE_SIZE]; //type table

// expression type checking
int checkExprType (nodeType* p);

int argc = 0; // global variable for arguments count

int in_func = 0; // global variable for checking if variables are declared inside functions


void prepass(nodeType *p, int infunc);
%}

%union {
    int iValue;                 /* integer value */
    char *sValue;		/* address of the string */
    char *vName;                /* symbol table index */
    nodeType *nPtr;             /* node pointer */
};

%token <iValue> INTEGER
%token <sValue> STRING CHARACTER
%token <vName> VARIABLE
%token WHILE IF 
%token PUTI PUTH PUTC PUTS PUTI_ PUTH_ PUTC_ PUTS_ 
%token ARRAY ARRAY_DECLARE PARAM_ARRAY_DECLARE
%token STRING_ARRAY_DECLARE
%nonassoc IFX
%nonassoc ELSE

%left AND OR

%left GE LE EQ NE '>' '<'
%left '+' '-'
%left '*' '/' '%'
%nonassoc UMINUS

%type <nPtr> stmt expr stmt_list vari tree

%%

program:
        tree                {  prepass($1, 0); printsp(var_count); printSYM(); ex($1, 998, 998, 0); eop(); exit(0); }
        ;

tree:
	  tree stmt		{ $$ = opr(MAIN, 2, $1, $2); in_func = 0; }
    | /* NULL */		{ $$ = NULL; }
    ;

stmt:
          ';'                                 { $$ = opr(';', 2, NULL, NULL); }
        | vari '=' expr ';'                   { $$ = opr('=', 2, $1, $3); }
        | WHILE '(' expr ')' stmt             { $$ = opr(WHILE, 2, $3, $5); }
        | IF '(' expr ')' stmt %prec IFX      { $$ = opr(IF, 2, $3, $5); }
        | IF '(' expr ')' stmt ELSE stmt      { $$ = opr(IF, 3, $3, $5, $7); }
        | '{' stmt_list '}'                   { $$ = $2; }
        | PUTI '(' expr ')' ';'               { $$ = opr(PUTI, 1, $3);}
        | PUTI_ '(' expr ')' ';'              { $$ = opr(PUTI_, 1, $3);}
        | PUTH '(' expr ')' ';'               { $$ = opr(PUTH, 1, $3);}
        | PUTH_ '(' expr ')' ';'              { $$ = opr(PUTH_, 1, $3);}
        | PUTC '(' expr ')' ';'               { $$ = opr(PUTC, 1, $3);}
        | PUTC_ '(' expr ')' ';'              { $$ = opr(PUTC_, 1, $3);}
        | PUTS '(' expr ')' ';'               { $$ = opr(PUTS, 1, $3);}
        | PUTS_ '(' expr ')' ';'              { $$ = opr(PUTS_, 1, $3);}
        | expr ';'                            { $$ = $1; }
        | ARRAY vari '[' INTEGER ']' ';'      { $$ = opr(ARRAY_DECLARE, 2, $2, con($4));}
        | vari '[' expr ']' '=' expr ';'      { $$ = opr('=', 3, $1, $3, $6);}
        | ARRAY vari '[' INTEGER ']' '=' STRING ';'      { $$ = opr(STRING_ARRAY_DECLARE, 3, $2, con($4), strCon($7)); }
        ;

vari:
        VARIABLE { $$ = id($1, 0); }
        | '@' VARIABLE { $$ = id($2, 1); }
        ;

stmt_list:
          stmt                  { $$ = $1; }
        | stmt_list stmt        { $$ = opr(';', 2, $1, $2); }
        ;

expr:
        INTEGER               { $$ = con($1); }
        | CHARACTER		{ $$ = charCon($1); }
        | STRING		{ $$ = strCon($1); }
        | '-' expr %prec UMINUS { $$ = opr(UMINUS, 1, $2); }
        | expr '+' expr         { $$ = opr('+', 2, $1, $3); }
        | expr '-' expr         { $$ = opr('-', 2, $1, $3); }
        | expr '*' expr         { $$ = opr('*', 2, $1, $3); }
        | expr '%' expr         { $$ = opr('%', 2, $1, $3); }
        | expr '/' expr         { $$ = opr('/', 2, $1, $3); }
        | expr GE expr          { $$ = opr(GE, 2, $1, $3); }
        | expr LE expr          { $$ = opr(LE, 2, $1, $3); }
        | expr NE expr          { $$ = opr(NE, 2, $1, $3); }
        | expr EQ expr          { $$ = opr(EQ, 2, $1, $3); }
        | '(' expr ')'          { $$ = $2; }
        | vari                  { $$ = $1; }
        | vari '[' expr ']'     { $$ = OneDArray($1, $3);}
        ;

%%

#define SIZEOF_NODETYPE ((char *)&p->con - (char *)p)

nodeType *con(int value) {
    nodeType *p;
    size_t nodeSize;

    /* allocate node */
    nodeSize = SIZEOF_NODETYPE + sizeof(conNodeType);
    if ((p = malloc(nodeSize)) == NULL)
        yyerror("out of memory");

    /* copy information */
    p->type = typeCon;
    p->con.value = value;

    return p;
}

nodeType *charCon(char *value) {
    nodeType *p;
    size_t nodeSize;

    /* allocate node */
    nodeSize = SIZEOF_NODETYPE + sizeof(conNodeType);
    if ((p = malloc(nodeSize)) == NULL)
        yyerror("out of memory");
    /* copy information */
    p->type = typeCharCon;
    p->charCon.value = value;

    return p;
}

nodeType *strCon(char *value) {
    nodeType *p;
    size_t nodeSize;

    /* allocate node */
    nodeSize = SIZEOF_NODETYPE + sizeof(conNodeType);
    if ((p = malloc(nodeSize)) == NULL)
        yyerror("out of memory");

    /* copy information */
    p->strCon.value = (char *)malloc(sizeof(char) * 200);
    int i=0;
    while(value[i]!='\0'){
        p->strCon.value[i] = value[i];
        i++;
    }
    p->type = typeStrCon;
    //p->strCon.value = value;

    return p;
}

nodeType *id(char *name, int isGlobal) {
    nodeType *p;
    size_t nodeSize;

    /* allocate node */
    nodeSize = SIZEOF_NODETYPE + sizeof(idNodeType);
    if ((p = malloc(nodeSize)) == NULL)
        yyerror("out of memory");

    p->id.isGlobal = isGlobal;
    p->id.var_name = name;
    p->type = typeId;

    return p;
}

nodeType *opr(int oper, int nops, ...) {
    va_list ap;
    nodeType *p;
    size_t nodeSize;
    int i;

    /* allocate node */
    nodeSize = SIZEOF_NODETYPE + sizeof(oprNodeType) +
        (nops - 1) * sizeof(nodeType*);
    if ((p = malloc(nodeSize)) == NULL)
        yyerror("out of memory");

    /* copy information */
    p->type = typeOpr;
    p->opr.oper = oper;
    p->opr.nops = nops;
    va_start(ap, nops);
    for (i = 0; i < nops; i++)
        p->opr.op[i] = va_arg(ap, nodeType*);
    va_end(ap);
    return p;
}

/* OneD Array construction function*/
nodeType * OneDArray(nodeType * name, nodeType * index)
{
  nodeType *p;
  size_t nodeSize;
  /* allocate node*/
  nodeSize = SIZEOF_NODETYPE + sizeof(OneDArrayNodeType);
  if((p = malloc(nodeSize)) == NULL)
  {
    yyerror("out of memory");
  }
  /* copy information */
  p->type = typeOneDArray;
  p->onedarray.name = name;
  p->onedarray.index = index;

  return p;
}

void freeNode(nodeType *p) {
    int i;

    if (!p) return;
    if (p->type == typeOpr) {
        for (i = 0; i < p->opr.nops; i++)
            freeNode(p->opr.op[i]);
    }
    free (p);
}

void yyerror(char *s) {
    fprintf(stdout, "%s\n", s);
}


int main(int argc, char **argv) {
extern FILE* yyin;
    yyin = fopen(argv[1], "r");
    yyparse();
    return 0;
}
