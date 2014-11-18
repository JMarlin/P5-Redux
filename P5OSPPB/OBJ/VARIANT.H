#infndef VARIANT_H
#define VARIANT_H

#include "lists.h"
#include "baseobject.h"

#define VAR_MODE_UNDEF  0
#define VAR_MODE_STRING 1
#define VAR_MODE_NUMBER 2
#define VAR_MODE_OBJ    3

typedef struct variant {

       char* strVal;
       double dblVal;
       baseObject* objPtrVal;
       unsigned char mode;         

} variant;

#endif
