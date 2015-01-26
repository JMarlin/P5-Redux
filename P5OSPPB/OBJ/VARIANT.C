#include "variant.h"
#include "../memory/memory.h"
#include "../ASCII_IO/ascii_i.h"
#include "lists.h"


variant* makeVariant(char* name) {

    variant* retvar = (variant*)kmalloc(sizeof(variant));

    retvar->name = name;
    retvar->mode = VAR_MODE_UNDEF;
    return retvar;    
}

//Probably would be better as a macro
void collectVariant(variant* thisVariant) {
        
    kfree((void*)thisVariant);
}

double str2dbl(char* dblStr) {

    int i = 0;
    double w = 0;
    double f = 0;
    double sign = 1;    
    double p = 1;

    if(dblStr[i] == 0)
        return 0;

    //Get sign
    if(dblStr[i] == '-') {
        sign = -1;
        i++;
    }
    
    //Get whole part
    if(!(dblStr[i] && (dblStr[i] >= '0' && dblStr[i] <= '9'))) return;

    while(1) {
        w += dblStr[i] - '0';
        i++;
        if(!dblStr[i]) return w*sign;
        if(!(dblStr[i] >= '0' && dblStr[i] <= '9')) break;
        w = w * 10;
    }

    if(dblStr[i] != '.')
        return 0.0;

    i++;

    //Get fractional part
    if(!(dblStr[i] && (dblStr[i] >= '0' && dblStr[i] <= '9'))) return;

    while(1) {
        w += dblStr[i] - '0';
        i++;
        if(!dblStr[i]) return w+p;
        if(!(dblStr[i] >= '0' && dblStr[i] <= '9')) return 0.0;
        p = p / 10.0;
    }           
}


int pow(int x, int y) {

    int o = 1;

    for(; y > 0; y--)
        o *= x;

    return o;
}


char* dbl2str(double value) {

    double w, f;
    int wi, fi;
    int l = 0,
        i = 0,
        j = 0,
        k = 0;
    char* retBuffer;

    wi = (int)value; 
    f = ((value - wi)*pow(10, STR_FRAC_LEN));
    fi = (int)f;    

    while(wi % pow(10, i)) i++;
    retBuffer = (char*)kmalloc(STR_FRAC_LEN + i + 1);

    for(j = i; j >= 0; j--) {
        retBuffer[k] = (wi % pow(10, j)) - '0';
        k++; 
    }

    retBuffer[k++] = '.';
    for(j = STR_FRAC_LEN; j >= 0; j--) {
        retBuffer[k] = (fi % pow(10, j)) - '0';
        k++; 
    }

    retBuffer[k] = 0;
    return retBuffer;        
} 


//Updates the internal values of thisVariant based
//on the value of its active type member
void variantInternalCast(variant* thisVariant) {

    char* tmpStr; 

    switch(thisVariant->mode) {
        
        case VAR_MODE_STRING: 
            thisVariant->dblVal = str2dbl(thisVariant->strVal);
            //thisVariant->objPtrVal = (baseObject*)0;
            break;

        case VAR_MODE_NUMBER:
            tmpStr = dbl2str(thisVariant->dblVal);
            variantAssignString(thisVariant, dbl2str(thisVariant->dblVal));
            kfree(tmpStr);
            //thisVariant->objPtrVal = (baseObject*)0;
            break;        

        case VAR_MODE_OBJ:
            thisVariant->strVal = ""; //thisVariant->objPtrVal->do(thisVariant->objPtrVal, "toStr");
            //Doesn't matter, because doing anything between a number and an object should be illegal, but whatever.
            thisVariant->dblVal = 0;     
            break;                

        case VAR_MODE_UNDEF:
        default:
            break;
    }
}


void variantAssignNum(variant* thisVariant, double value) {
  
    thisVariant->dblVal = value;                
}


char* cloneStr(char* origStr) {

    int i = 0,
        j = 0;
    char* retBuf;

    while(origStr[i]) i++;   
    retBuf = (char*)kmalloc(i);
    
    for(j = 0; j < i; j++)
        retBuf[j] = origStr[i];        
}


void variantAssignString(variant* thisVariant, char* value) {

    if(value) kfree((void*)thisVariant->strVal);
    thisVariant->strVal = cloneStr(value);
}


varList* newVarList(void) {       

    return (varList*)newList();        
}

variant* getVar(varList* inList, char* searchName) {
        
    variant* curVar;
    listItem* curItem;

    listRewind(inList);

    while(curItem = listNext(inList)) {

        if(curItem->data) {
            curVar = (variant*)curItem;
    
            if(strcmp(curVar->name, searchName)) {
                listRewind(inList);
                return curVar;
            }            
        }
    }            

    listRewind(inList);
    return (variant*)0;
}


void delVar(varList* inList, char* searchName) {
        
    variant* curVar;

    curVar = getVar(inList, searchName);

    if(curVar) {
        listRemoveObj(inList, (void*)curVar);
        collectVariant(curVar);
    }
}


void allocVar(varList* inList, char* newName) {
        
    listAdd(inList, (void*)makeVariant(newName));     
}         
