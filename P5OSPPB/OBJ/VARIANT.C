#include "variant.h"
#include "../memory/memory.h"

variant* makeVariant() {

        variant* retvar = (variant*)kmalloc(sizeof(variant));
        retvar->mode = VAR_MODE_UNDEF;
        return retvar;
        
}

//Probably would be better as a macro
void collectVariant(variant* thisVariant) {
        
        kfree((void*)thisVariant);

}

//Updates the internal values of thisVariant based
//on the value of its active type member
void variantInternalCast(variant* thisVariant) {

        switch(thisVariant->mode) {
                
                case VAR_MODE_STRING: 
                        thisVariant->dblVal = str2dbl(thisVariant->strVal);
                        thisVariant->objPtrVal = (baseObject*)0;
                        break;

                case VAR_MODE_NUMBER:
                        thisVariant->strVal = dbl2str(thisVariant->dblVal);
                        thisVariant->objPtrVal = (baseObject*)0;
                        break;        

                case VAR_MODE_OBJ:
                        thisVariant->strVal = thisVariant->objPtrVal->toStr();
                        thisVariant->numVal = 0; //Doesn't matter, because doing anything between a number and an object should be illegal, but whatever.   
                        break;                

                case VAR_MODE_UNDEF:
                default:
                        break;

        }

}

void variantAssignNum(variant* thisVariant, double value) {
        
        

}
