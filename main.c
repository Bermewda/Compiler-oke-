#include <stdio.h>
#include "main.h"
#include "y.tab.h"
#include <stdlib.h>
#include <string.h>

// for debugging
// #define DEBUG
// #define RTDEBUG

#define TABLE_SIZE 40000
extern int var_count, func_count, loc_var_count;
extern int vType[TABLE_SIZE];
extern char* func[TABLE_SIZE];
extern int argTable[TABLE_SIZE];
int funcIdx = -1;
int thisArgc = -1;
int hasreturn = 0;

static int lbl;

//structs for array
typedef struct
{
  char* name;
  int size;
} oneDArray;

oneDArray* onedarray_list[50];

int onedarray_count = 0;

// function declaration
//sym related
int inSYM(char *var_name);
void insertSYM(char * var_name, int isGlobal);
int getSYMIdx(char *var_name, int isGlobal);
void emptySYM(int isGlobal);
void insert1DArraySYM(char * array_name, int array_size, int is_global);

void printsp(int coun);

void prepass(nodeType *p, int infunc);

void printStackTop(int type);

// array list related
void insert1DArrayList(char* name, int size);
oneDArray* getOneDArray(char* name);

//function related
void insertArg(int argnum, int idx);
void setType (int index, int t);

int checkExprType (nodeType* p);
int checkType (int index);

// helper functions
void printSYM();
void printIN();

/*
 function to print the symbol table
*/
void printSYM()
{
#ifdef DEBUG
  printf("[DEBUG]\t\tGlobal SYM begin:\n");
  int i;
  for(i = 0; i < var_count; i++)
  {
    printf("[DEBUG]\t\tidx: %d\tvar_name:\t%s\n", i, sym[i]);
  }
  printf("[DEBUG]\t\tGlobal SYM end\n");
  printf("[DEBUG]\t\tLocal SYM begin:\n");
  for(i = 0; i < loc_var_count; i++)
  {
    printf("[DEBUG]\t\tidx: %d\tvar_name:\t%s\n", i, sym[i + TABLE_SIZE/2]);
  }
  printf("[DEBUG]\t\tLocal SYM end\n");
#endif
}

/*
 function to print the computed offset
*/
void printIN(){
  #ifdef RTDEBUG
  printf("\tpush\t\"in: \"\n");
  printf("\tputs_\n");
  printf("\tpush\tin\n");
  printf("\tputi\n");
  #endif
}

/*
function to let nas know the number of global variables
*/
void printsp(int coun){
  printf("\tpush\tsp\n");
  printf("\tpush\t%d\n", coun);
  printf("\tadd\n");
  printf("\tpop\tsp\n");
}

/*
function for prepassing the whole tree
  p: node
  infunc:
    0: outside functions
    1: inside functions
*/
void prepass(nodeType *p, int infunc){

  if (!p) return;
  switch(p->type) {    
    case typeId:
    if (infunc == 0){
      p->id.isGlobal=1;
    }
    insertSYM(p->id.var_name, p->id.isGlobal);
    break;
    case typeOneDArray:
    {
      prepass(p->onedarray.index, infunc);
      if(infunc == 0)
      {
        p->onedarray.name->id.isGlobal = 1;
      }
      break;
    }
    case typeOpr:
    switch(p->opr.oper) {
      case MAIN:
      prepass(p->opr.op[0], infunc);
      prepass(p->opr.op[1], infunc);
      break;
      case WHILE:
      prepass(p->opr.op[0], infunc);
      prepass(p->opr.op[1], infunc);
      break;
      case IF:
      if (p->opr.nops > 2) {
        /* if else then */
        prepass(p->opr.op[0], infunc);
        prepass(p->opr.op[1], infunc);
        prepass(p->opr.op[2], infunc);
      } else {
        /* if else */
        prepass(p->opr.op[0], infunc);
        prepass(p->opr.op[1], infunc);
      }
      break;
      case '=':
      if(p->opr.op[0]->type == typeId) {
        /* insert into symbol table */
        if (infunc == 0){
          p->opr.op[0]->id.isGlobal=1;
        }
        insertSYM(p->opr.op[0]->id.var_name, p->opr.op[0]->id.isGlobal);
      }else if(p->opr.op[0]->type == typeOpr && p->opr.op[0]->opr.oper == '@'){
        prepass(p->opr.op[0], infunc);
      }
      prepass(p->opr.op[1], infunc);
      break;
      case PARAM_ARRAY_DECLARE:
      {
        char * array_name = p->opr.op[0]->id.var_name;
        // int is_global = p->opr.op[0]->id.isGlobal;
        int array_fst_size = p->opr.op[1]->con.value;

        // if outside function then the array is global, the reverse is not true
        if (infunc == 0){
          p->opr.op[0]->id.isGlobal = 1;
        }

        if(p->opr.nops == 2)//1D Array
        {
          insert1DArrayList(array_name, array_fst_size);
        }
        break;
      }
      case STRING_ARRAY_DECLARE:
      {
        char * array_name = p->opr.op[0]->id.var_name;
        int array_fst_size = p->opr.op[1]->con.value;
        char * array_str_content = p->opr.op[2]->strCon.value;
        // if outside function then the array is global, the reverse is not true
        if (infunc == 0){
          p->opr.op[0]->id.isGlobal = 1;
        }
        insert1DArraySYM(array_name, array_fst_size, p->opr.op[0]->id.isGlobal);
        insert1DArrayList(array_name, array_fst_size);
        break;
      }
      case ARRAY_DECLARE:
      {
        char * array_name = p->opr.op[0]->id.var_name;
        // int is_global = p->opr.op[0]->id.isGlobal;
        int array_fst_size = p->opr.op[1]->con.value;

        // if outside function then the array is global, the reverse is not true
        if (infunc == 0 || p->opr.op[0]->id.isGlobal){
          p->opr.op[0]->id.isGlobal = 1;
        }

        if(p->opr.nops == 2)//1D Array
        {
          insert1DArraySYM(array_name, array_fst_size, p->opr.op[0]->id.isGlobal);
          insert1DArrayList(array_name, array_fst_size);
        }
        int index = getSYMIdx(array_name, p->opr.op[0]->id.isGlobal);
        setType(index, 4); // set the type to be 4
        break;
      }
      case PUTI:
      case PUTI_:
      case PUTH:
      case PUTH_:
      case PUTC:
      case PUTC_:
      case PUTS:
      case PUTS_:
      prepass(p->opr.op[0], infunc);
      break;
      case ';':
      prepass(p->opr.op[0],infunc);
      prepass(p->opr.op[1],infunc);
      break;
      default:
      prepass(p->opr.op[0],infunc);
      prepass(p->opr.op[1],infunc);
      break;
    }
    default:
    break;
  }
}

/*
function to deal with printing for different type
*/
void printStackTop(int type){

  if(type == 1){
    printf("\tputi\n");
  }else if(type == 2){
    printf("\tputs\n");
  }else if(type == 3){
    printf("\tputc\n");
  }else if(type == 4){
    printf("\tputh\n");
  }else{
    // unknown type
    yyerror("unknown type for printing");
  }
}

/*
function for checking existance of a variable in the symbol table
var_name: variable name
*/
int inSYM(char * var_name)
{
  int i;
  for(i = 0; i < var_count; i++)
  {
    if(strcmp(sym[i], var_name) == 0)
    {
      return 1; // in sym
    }
  }
  return 0; // not in sym
}

/*
function for adding variables to symbol table
var_name: variable name
*/
void insertSYM(char * var_name, int isGlobal)
{
  if(var_count >= TABLE_SIZE)
  {
    printf("\tNumber of variables exceeds the limit!\n");
    return;
  }
  if(getSYMIdx(var_name, isGlobal) < 0)//not in sym
  {
    if(isGlobal == 1){ //global variable
      sym[var_count] = (char *) malloc (strlen(var_name));
      strcpy(sym[var_count], var_name);
      var_count ++;
    } else {
      sym[loc_var_count+TABLE_SIZE/2] = (char *) malloc (strlen(var_name));
      strcpy(sym[loc_var_count+TABLE_SIZE/2], var_name);
      loc_var_count ++;
    }
  }
}

/* function for inserting an array name into the SYM
array_name: Name of the array
array_size: Size of the array
is_global: if the array is declared globally
*/
void insert1DArraySYM(char * array_name, int array_size, int is_global)
{
  // check for resource availability
  if(var_count > TABLE_SIZE)
  {
    printf("\t Size of the array: %s exceeds the memory limit!\n", array_name);
    return;
  }

  if(getSYMIdx(array_name, is_global) < 0)//not in sym
  {
    if(is_global == 1)
    {
      // global
      int i;
      for(i = 0; i < array_size; i++)
      {

        sym[var_count] = (char *) malloc (strlen(array_name));
        strcpy(sym[var_count], array_name);
        var_count ++;
      }
    }
    else
    {
      // local
      int i;
      for(i = 0; i < array_size; i++)
      {
        sym[loc_var_count+TABLE_SIZE/2] = (char *) malloc (strlen(array_name));
        strcpy(sym[loc_var_count+TABLE_SIZE/2], array_name);
        loc_var_count ++;
      }

    }
  }
}

/*
Insert an 1D array into the list
name: Array name
size: Array size
*/
void insert1DArrayList(char* name, int size)
{
  if(onedarray_count < 50)
  {
    oneDArray* array;
    int nodeSize;
    nodeSize = sizeof(array);
    if((array = malloc(nodeSize)) == NULL)
    {
      yyerror("Error: not enough memory for allocating 1D array list\n");
      return;
    }

    array->name = name;
    array->size = size;

    onedarray_list[onedarray_count] = array;
    onedarray_count++;
  }
  else
  {
    printf("Number of arrays exceeds limit: %d at most\n", 50);
  }
}

/*
get the settings of an 1D array
*/
oneDArray* getOneDArray(char* name)
{
  int i;
  for(i = 0; i < onedarray_count; i++)
  {
    if(strcmp(onedarray_list[i]->name, name) == 0)
    {
      return onedarray_list[i];
    }
  }
  return NULL;
}

/*
function for getting index of a variable in sym
*/
int getSYMIdx(char * var_name, int isGlobal)
{
  int i;
  if (isGlobal == 1){ // for global variable

    for(i = 0; i < var_count; i++)
    {
      if(strcmp(sym[i], var_name) == 0)
      {
        return i;
      }
    }
  }else{ // for local variable
    for(i = TABLE_SIZE/2; i < TABLE_SIZE/2 + loc_var_count; i++)
    {
      if(strcmp(sym[i], var_name) == 0)
      {
        return i;
      }
    }
  }

  return -1; // not found
}

/*
function for clearing the symbol table
*/
void emptySYM(int isGlobal)
{
  int i;
  if(isGlobal){
    for(i = 0; i < var_count; i++)
    {
      free(sym[i]);
    }
    var_count = 0;
  }
  else{
    for(i = TABLE_SIZE/2; i < TABLE_SIZE/2 + loc_var_count; i++)
    {
      free(sym[i]);
    }
    loc_var_count = 0;
  }
}

/*
function to get type for stored variable
*/
int checkType (int index)
{
  return vType[index];
}

void insertArg(int argnum, int idx)
{
  argTable[idx] = argnum;
}

/*
function to set type for some variable
*/
void setType (int index, int t)
{
  vType[index] = t;
  return;
}

/*
function for checking type for an expression
*/
int checkExprType (nodeType* p){
  switch(p->type) {
    case typeId:{
      char *str = p->id.var_name;
      int index = getSYMIdx(str, p->id.isGlobal);
      int type = checkType(index);
      return type;
    }
    case typeCon:
      return 1;
    case typeStrCon:
      return 2;
    case typeCharCon:
      return 3;
    case typeOpr:
    switch(p->opr.oper) {
      case UMINUS:
        return 1;
      default:
        return 1;
    }
  }
}

int ex(nodeType *p, int blbl, int clbl, int infunc) {
  int lblx, lbly, lblz;

  if (!p) return 0;
  switch(p->type) {
    case typeCon:
    {
      printf("\tpush\t%d\n", p->con.value);
      break;
    }
    case typeCharCon:
    {
      printf("\tpush\t%s\n", p->charCon.value);
      break;
    }
    case typeStrCon:
    {
      printf("\tpush\t\"%s\"\n", p->strCon.value);
      break;
    }
    case typeId:
    {
      char *str = p->id.var_name;
      int index = getSYMIdx(str, p->id.isGlobal);
      if (index==-1)
      {
        printf("\tError: Variable \"%s\" uninitialized\n", str);
        exit(0);
      }
      int thisT = checkType(index);
      if (p->id.isGlobal == 1)
      {
        if (thisT == 4){ // this is an array paramter
          printf("\tpush\t%d\n", index);
        }else{
          printf("\tpush\tsb[%d]\n", index);
        }
      }
      else { // for local
        index = index-TABLE_SIZE/2;
        int trueIdx = -1;
        if (index >= argTable[funcIdx]) // for local variable
        {
          trueIdx = index - argTable[funcIdx];
          if (thisT == 4){ // this is an array paramter
          //printf("idx: %d, argc: %d\n", trueIdx, argTable[funcIdx]);
            printf("\tpush\t%d\n", trueIdx);  // pass the reference
            printf("\tpush\t%d\n", 3+argTable[funcIdx]);
            printf("\tpush\tfp[-3]\n"); // compute the absolute position
            printf("\tadd\n");
            printf("\tadd\n");
          }else{
            printf("\tpush\tfp[%d]\n", trueIdx);
          }
        }
        else // for parameter
        {
          trueIdx = -3 - argTable[funcIdx] + index; //3 is accounted for the saved caller's pc, fp, sp
          printf("\tpush\tfp[%d]\n", trueIdx);
        }
      }
      break;
    }
    case typeOneDArray:
    {
      char * array_name = p->onedarray.name->id.var_name;
      int index = getSYMIdx(array_name, p->onedarray.name->id.isGlobal);
      if (index==-1)
      {
        printf("\t[Debug]: 1D Array name %s, is global: %d\n", array_name, p->onedarray.name->id.isGlobal);
        printf("\tError: 1D Array \"%s\" Undeclared\n", array_name);
        exit(0);
      }
      //TODO check array index outofbound

      // evaluate the expression to get index
      ex(p->onedarray.index, -1, -1, infunc);

      // get array index offset
      // check if the array is global
      if (p->onedarray.name->id.isGlobal == 1)
      {
        // global
        printf("\tpush\t%d\n", index);
      }
      else
      { // for local
        int locIdx = index-TABLE_SIZE/2;
        //printf("name: %s, locIdx: %d, argc: %d\n", array_name, locIdx, argTable[funcIdx]);
        if (locIdx >= argTable[funcIdx]) // for local
          printf("\tpush\t%d\n",  locIdx - argTable[funcIdx]);
        else // for parameter
          printf("\tpush\tfp[%d]\n",  -3 - argTable[funcIdx] + locIdx);
      }

      // add up offset and the index
      printf("\tadd\n");

      // pop the index to the in register
      printf("\tpop\tin\n");

      if(p->onedarray.name->id.isGlobal == 1)
      {
        // global
        printf("\tpush\tsb[in]\n");
      }
      else
      {
        // local
        int locIdx = index-TABLE_SIZE/2;
        if (locIdx >= argTable[funcIdx]) // for local
          printf("\tpush\tfp[in]\n");
        else // for parameter
          printf("\tpush\tsb[in]\n");
      }
      break;
    }
    case typeOpr:
    switch(p->opr.oper) {
      
      case WHILE:
      {
        lblx = lbl++;
        lbly = lbl++;
        printf("L%03d:\n", lblx);
        ex(p->opr.op[0], blbl, clbl, infunc);
        printf("\tj0\tL%03d\n", lbly);
        ex(p->opr.op[1], lbly, lblx, infunc);
        printf("\tjmp\tL%03d\n", lblx);
        printf("L%03d:\n", lbly);
        break;
      }
      case IF:
      {
        ex(p->opr.op[0], blbl, clbl, infunc);
        if (p->opr.nops > 2) {
          /* if else */
          printf("\tj0\tL%03d\n", lblx = lbl++);
          ex(p->opr.op[1], blbl, clbl, infunc);
          printf("\tjmp\tL%03d\n", lbly = lbl++);
          printf("L%03d:\n", lblx);
          ex(p->opr.op[2], blbl, clbl, infunc);
          printf("L%03d:\n", lbly);
        } else {
          /* if */
          printf("\tj0\tL%03d\n", lblx = lbl++);
          ex(p->opr.op[1], blbl, clbl, infunc);
          printf("L%03d:\n", lblx);
        }
        break;
      }
      case STRING_ARRAY_DECLARE:
      {
        char * array_name = p->opr.op[0]->id.var_name;
        int is_global = p->opr.op[0]->id.isGlobal;
        int index = getSYMIdx(array_name, is_global);

        if(index == -1)
        {
          printf("\tError: Array %s used before declared!\n", array_name);
        }

        char * array_str_content = p->opr.op[2]->strCon.value;
        int array_str_len = strlen(array_str_content);

        int i;
        for(i = 0; i < array_str_len; i++)
        {
          printf("\tpush\t\'%c\'\n", array_str_content[i]);
          printf("\tpush\t%d\n", i);

          //TODO check for array index outofbound

          // push the index offset on the stack
          if(is_global)
          {
            printf("\tpush\t%d\n", index);
          }
          else
          {
            int locIdx = index - TABLE_SIZE/2;
            if (locIdx >= argTable[funcIdx]){ // for local array
              printf("\tpush\t%d\n", locIdx - argTable[funcIdx]);
            } else { // for array parameter
              printf("\tpush\tfp[%d]\n", -3 - argTable[funcIdx] + locIdx);
            }
          }

          // add up the index and the offset and store it in "in" register
          printf("\tadd\n");
          printf("\tpop\tin\n");

          if(is_global)
          {
            printf("\tpop\tsb[in]\n");
          }
          else
          {
            int locIdx = index - TABLE_SIZE/2;
            if (locIdx>=argTable[funcIdx]){ // for local array
              printf("\tpop\tfp[in]\n");
            } else { // for array parameter
              printIN();
              printf("\tpop\tsb[in]\n");
            }
          }//end if
        }//end for
      }
      case ARRAY_DECLARE:
      {
        break;
      }
      case '=':
      {
        if(p->opr.nops == 2)// Variable assignment:  vari = expr
        {
          ex(p->opr.op[1], blbl, clbl, infunc);

          if(p->opr.op[0]->type == typeId) {
            /* examine type */
            int index = getSYMIdx(p->opr.op[0]->id.var_name, p->opr.op[0]->id.isGlobal);
            int thisT = checkExprType(p->opr.op[1]);
            setType(index, thisT);
            if (index == -1)
            insertSYM(p->opr.op[0]->id.var_name, p->opr.op[0]->id.isGlobal);
            if (p->opr.op[0]->id.isGlobal)// for global
            {
              printf("\tpop\tsb[%d]\n", index);
            }
            else{ // for local
              index = index-TABLE_SIZE/2;
              if (index >= argTable[funcIdx]) // for parameter
              printf("\tpop\tfp[%d]\n",  index - argTable[funcIdx]);
              else // for local variable
              printf("\tpop\tfp[%d]\n",  -3 - argTable[funcIdx] + index);
            }
          } else if (p->opr.op[0]->type == typeOpr && p->opr.op[0]->opr.oper == '@') { /* @ for global variable */
            nodeType* tmp = p->opr.op[0]->opr.op[0];
            int index = getSYMIdx(tmp->id.var_name, tmp->id.isGlobal);
            int thisT = checkExprType(p->opr.op[1]);
            setType(index, thisT);

            if (tmp->id.isGlobal)// for global
            printf("\tpop\tsb[%d]\n", index);
          }

        }
        else if(p->opr.nops == 3) // Array element assignment: vari [exper] = expr
        {
          char * array_name = p->opr.op[0]->id.var_name;
          int is_global = p->opr.op[0]->id.isGlobal;
          int index = getSYMIdx(array_name, is_global);

          if(index == -1)
          {
            printf("\tError: Array %s used before declared!\n", array_name);
          }

          // evaluate the expression to get the value to be assigned
          ex(p->opr.op[2], blbl, clbl, infunc);

          // evaluate the index
          ex(p->opr.op[1], blbl, clbl, infunc);

          //TODO check for array index outofbound

          // push the index offset on the stack
          if(is_global)
          {
            printf("\tpush\t%d\n", index);
          }
          else
          {
            int locIdx = index - TABLE_SIZE/2;
            if (locIdx >= argTable[funcIdx]){ // for local array
              printf("\tpush\t%d\n", locIdx - argTable[funcIdx]);
            } else { // for array parameter
              printf("\tpush\tfp[%d]\n", -3 - argTable[funcIdx] + locIdx);
            }
          }

          // add up the index and the offset and store it in "in" register
          printf("\tadd\n");
          printf("\tpop\tin\n");

          if(is_global)
          {
            printf("\tpop\tsb[in]\n");
          }
          else
          {
            int locIdx = index - TABLE_SIZE/2;
            if (locIdx>=argTable[funcIdx]){ // for local array
              printf("\tpop\tfp[in]\n");
            } else { // for array parameter
              printIN();
              printf("\tpop\tsb[in]\n");
            }
          }
        }
      case UMINUS:
      {
        ex(p->opr.op[0], blbl, clbl, infunc);
        printf("\tneg\n");
        break;
      }
      case PUTI:
      {
        int typeP = checkExprType(p->opr.op[0]);
        ex(p->opr.op[0],blbl, clbl, infunc);
        printf("\tputi\n");
        break;
      }
      case PUTI_:
      {
        int typeP = checkExprType(p->opr.op[0]);
        ex(p->opr.op[0],blbl, clbl, infunc);
        printf("\tputi_\n");

        break;
      }
      case PUTH:
      {
        int typeP = checkExprType(p->opr.op[0]);
        ex(p->opr.op[0], blbl, clbl, infunc);
        printf("\tputh\n");
        break;
      }
      case PUTH_:
      {
        int typeP = checkExprType(p->opr.op[0]);
        ex(p->opr.op[0], blbl, clbl, infunc);
        printf("\tputh_\n");

        break;
      }
      case PUTC:
      {
        int typeP = checkExprType(p->opr.op[0]);
        ex(p->opr.op[0],blbl,clbl, infunc);
        printf("\tputc\n");
        break;
      }
      case PUTC_:
      {
        int typeP = checkExprType(p->opr.op[0]);
        ex(p->opr.op[0],blbl,clbl, infunc);
        printf("\tputc_\n");

        break;
      }
      case PUTS:
      {
        int typeP = checkExprType(p->opr.op[0]);
        ex(p->opr.op[0],blbl,clbl, infunc);
        printf("\tputs\n");
        break;
      }
      case PUTS_:
      {
        int typeP = checkExprType(p->opr.op[0]);
        ex(p->opr.op[0],blbl,clbl, infunc);
        printf("\tputs_\n");
        break;
      }
      
      default:
      ex(p->opr.op[0], blbl, clbl, infunc);
      ex(p->opr.op[1], blbl, clbl, infunc);
      switch(p->opr.oper) {
        case '+':   printf("\tadd\n"); break;
        case '-':   printf("\tsub\n"); break;
        case '*':   printf("\tmul\n"); break;
        case '/':   printf("\tdiv\n"); break;
        case '%':   printf("\tmod\n"); break;
        case GE:    printf("\tcompGT\n"); break;
        case LE:    printf("\tcompLT\n"); break;
        case NE:    printf("\tcompNE\n"); break;
        case EQ:    printf("\tcompEQ\n"); break;
      }
    }
  }

  return 0;
}
}


void eop() {
  // add end of program label
  printf("\tjmp\tL999\n");
  printf("L998:\n");
  printf("\tpush\t999999\n");
  printf("\tputi\n");
  printf("L999:\n");
}

