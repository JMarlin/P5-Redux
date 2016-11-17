#include "associativearray.h"
#include <stdlib.h>

AssociativeArray* AssociativeArray_new() {

    AssociativeArray* associative_array =
        (AssociativeArray*)malloc(sizeof(AssociativeArray));

    if(!associative_array)
        return associative_array;

    Object_init((Object*)associative_array, AssociativeArray_delete_function);
    associative_array->keys = List_new();
    associative_array->values = List_new();

    if(!(associative_array->keys && associative_array->values)) {

        Object_delete((Object*)associative_array);
        return (AssociativeArray*)0;
    }

    return associative_array;
}

void AssociativeArray_delete_function(Object* associative_array_object) {

    AssociativeArray* associative_array;

    if(!associative_array_object)
        return;

    associative_array = (AssociativeArray*)associative_array_object;

    Object_delete((Object*)associative_array->keys);
    Object_delete((Object*)associative_array->values);
    Object_default_delete_function(associative_array_object);
}

Object* AssociativeArray_get(AssociativeArray* associative_array, String* key) {

    int i;

    for(i = 0; i < associative_array->keys->count; i++)
        if(String_compare((String*)List_get_at(associative_array->keys, i), key))
            break;

    if(i == associative_array->keys->count)
        return (Object*)0;

    return List_get_at(associative_array->values, i);
}

int AssociativeArray_add(AssociativeArray* associative_array, String* key, Object* value) {

    if(!List_add(associative_array->keys, (Object*)key))
        return 0;
    
    return List_add(associative_array->values, value);
}
