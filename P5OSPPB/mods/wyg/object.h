#ifndef OBJECT_H
#define OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

struct Object_struct;

typedef void (*DeleteFunction)(struct Object_struct* object); 

typedef struct Object_struct {
    DeleteFunction delete_function;
} Object;

typedef struct String_struct {
    Object object;
    char* buf;
} String;

String* String_new(char* source_buf);
int String_compare(String* string_a, String* string_b);
void String_delete_function(Object* string_object);
void Object_init(Object* object, DeleteFunction delete_function);
void Object_default_delete_function(Object* object);
void Object_delete(Object* object);

#ifdef __cplusplus
}
#endif

#endif //OBJECT_H