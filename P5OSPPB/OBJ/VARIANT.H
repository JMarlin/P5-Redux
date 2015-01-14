#ifndef VARIANT_H
#define VARIANT_H

#include "lists.h"
//#include "baseobject.h"

#define VAR_MODE_UNDEF  0
#define VAR_MODE_STRING 1
#define VAR_MODE_NUMBER 2
#define VAR_MODE_OBJ    3

typedef struct variant {

       char* name;
       char* strVal;
       double dblVal;
       void* objPtrVal; //baseObject* objPtrVal;
       unsigned char mode;         

} variant;

typedef list varList; 

variant* makeVariant(char* name);
void collectVariant(variant* thisVariant);
void variantInternalCast(variant* thisVariant);
void variantAssignNum(variant* thisVariant, double value);
void variantAssignString(variant* thisVariant, char* value);
varList* newVarList(void);
variant* getVar(varList* inList, char* searchName);
void delVar(varList* inList, char* searchName);
void allocVar(varList* inList, char* newName);

#endif
