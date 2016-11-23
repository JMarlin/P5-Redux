#include "../include/memory.h"
#include "object.h"

String* String_new(char* source_buf) {

    int len, i;
    String* string = (String*)malloc(sizeof(String));

    if(!string)
        return string;

    Object_init((Object*)string, String_delete_function);
    
    if(!source_buf) {

        string->buf = source_buf;
        return string;
    }

    for(len = 1; source_buf[len-1]; len++);

    if(!(string->buf = (char*)malloc(len))) {

        Object_delete((Object*)string);
        return (String*)0;
    }

    for(i = 0; i < len; i++)
        string->buf[i] = source_buf[i];

    return string;
}

int String_compare(String* string_a, String* string_b) {

    int i;

    for(i = 0; string_a->buf[i]; i++)
        if(string_a->buf[i] != string_b->buf[i])
            return 0;

    return string_a->buf[i] == string_b->buf[i];
}

void String_delete_function(Object* string_object) {

    String* string;

    if(!string_object)
        return;

    string = (String*)string_object;

    if(string->buf)
        free(string->buf);

    Object_default_delete_function(string_object);
}

void Object_default_delete_function(Object* object) {

    free((void*)object);
}

void Object_init(Object* object, DeleteFunction delete_function) {

    if(delete_function)
        object->delete_function = delete_function;
    else
        object->delete_function = Object_default_delete_function;
}

void Object_delete(Object* object) {

    if(!object)
        return;

    if(object->delete_function)
        object->delete_function(object);
    else
        Object_default_delete_function(object);
}
