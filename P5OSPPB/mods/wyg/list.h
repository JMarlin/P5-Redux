#ifndef LIST_H
#define LIST_H

#ifndef NULL
#define NULL (void*)0
#endif //NULL

typedef struct ListItem {
    struct ListItem* prev;
    struct ListItem* next;
    void* value;
} ListItem;

typedef struct List {
    ListItem* root_item;
    ListItem* current_item;
	unsigned int count;
} List; 

typedef void (*deleter)(void* value);
typedef void (*printer)(void* value);

List* List_new(void);
void List_delete(List* list, deleter del_func);
void List_rewind(List* list);
void List_seek_to(List* list, int index);
void* List_get_next(List* list);
int List_add(List* list, void* value);
int List_has_next(List* list);
int List_get_index(List* list, void* value);
void* List_get_at(List* list, int index);
void* List_pop(List* list, void* value);
void List_remove(List* list, void* value, deleter del_func);
void List_print(List* list, printer print_func);

//Iterates through the values stored in list, placing each into variable value which is of type type 
#define List_for_each(list, value, type) for(List_rewind(list); ((value) = (type)List_get_next(list)); )
#define List_for_each_skip(list, value, type, skip) for(List_seek_to((list), (skip)); ((value) = (type)List_get_next(list)); )

#endif //LIST_H