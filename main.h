typedef enum { typeCon, typeStrCon, typeCharCon, typeId, typeOpr, typeFunc, typeOneDArray, typeTwoDArray, typeThreeDArray} nodeEnum;

/* constants */
typedef struct {
    int value;                  /* value of constant */
} conNodeType;

/* character constants */
typedef struct {
    char *value;                 /* value of string constant */
} charConNodeType;

/* string constants */
typedef struct {
    char *value;                /* value of string constant */
} strConNodeType;

/* identifiers */
typedef struct {
    char *var_name;             /* subscript to sym array */
    int isGlobal;
} idNodeType;

/* operators */
typedef struct {
    int oper;                   /* operator */
    int nops;                   /* number of operands */
    struct nodeTypeTag *op[1];  /* operands (expandable) */
} oprNodeType;


/* 1D Array */
typedef struct {
  struct nodeTypeTag * name;           /* name of the array, it is a nodetype because the "global" info needs to be stored */
  struct nodeTypeTag * index;              /* index of a an array element */
} OneDArrayNodeType;


typedef struct nodeTypeTag {
    nodeEnum type;              /* type of node */

    /* union must be last entry in nodeType */
    /* because operNodeType may dynamically increase */
    union {
      conNodeType con;        /* constants */
      charConNodeType charCon; /* character constants */
      strConNodeType strCon;	/* string constants */
      idNodeType id;          /* identifiers */
      oprNodeType opr;        /* operators */
      OneDArrayNodeType onedarray; /* 1D Array */
    };
} nodeType;

#define TABLE_SIZE 40000
extern char* sym[TABLE_SIZE];

#define MAIN 997
#define CALL 996
