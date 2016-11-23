#ifndef ASSOCIATIVEARRAY_H
#define ASSOCIATIVEARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "list.h"

typedef struct AssociativeArray_struct {
    Object object;
    List* keys;
    List* values;
} AssociativeArray;

AssociativeArray* AssociativeArray_new();
void AssociativeArray_delete_function(Object* associative_array_object);
Object* AssociativeArray_get(AssociativeArray* associative_array, String* key);
int AssociativeArray_add(AssociativeArray* associative_array, String* key, Object* value);

#ifdef __cplusplus
}
#endif

#endif //ASSOCIATIVEARRAY_H