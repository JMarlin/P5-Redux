#include "../include/memory.h"
#include "list.h"

List* List_new(void) {
    
    List* ret_list = (List*)malloc(sizeof(List));
    
    if(!ret_list)
        return ret_list;
        
    ret_list->root_item = (ListItem*)0;
    ret_list->count = 0;
	List_rewind(ret_list);
   	
    return ret_list;
}

//Deletes all elements of the list, and deletes the contained values
//using the passed delete function
void List_delete(List* list, deleter del_func) {
	
	ListItem* current_item = list->root_item;
	ListItem* prev_item;
	
	if(current_item) {
		
		//Fast forward to the end of the list
		while(current_item->next)
		    current_item = current_item->next;
			
	    //Delete in reverse order
		while(current_item) {
		
		    //Temporarily store the previous element so that we don't lose it
		    prev_item = current_item->prev;
			
			//Use the supplied deleter to delete the lite item's value 
			del_func(current_item->value);
			
			//Finally, get rid of the current ListItem and move back down the list
			free((void*)current_item);
			current_item = prev_item;    	
		}
	}
	
	//Now that we've deleted all of the content, we can free the root object
	free((void*)list);
}

void* List_pop(List* list, void* value) {
	
	ListItem* cur_item = list->root_item;
	void* ret_val;
    
    if(!value || list->count == 0) 
        return (void*)0;
        
    while(cur_item && (cur_item->value != value))
        cur_item = cur_item->next;
        
    if(!cur_item)
        return (void*)0;
    
    if(cur_item == list->current_item) {
        
        if(cur_item->prev) {
            
            list->current_item = cur_item->prev; 
        } else if(cur_item->next) {
            
            list->current_item = cur_item->next;
        } else {
            
            list->current_item = (ListItem*)0;
            list->root_item = (ListItem*)0;
        }
    }
    

    
    if(cur_item->prev)
        cur_item->prev->next = cur_item->next;
        
    if(cur_item->next)
        cur_item->next->prev = cur_item->prev;
    
    list->count--;
	ret_val = cur_item->value;
	free((void*)cur_item);
	return ret_val;
}

void List_remove(List* list, void* value, deleter del_func) {
    
    void* popval = List_pop(list, value);
	
	if(popval)
    	del_func(value);        
}

void List_rewind(List* list) {
    
    list->current_item = list->root_item;
}

int List_add(List* list, void* value) {
    
    ListItem* current_item;
    ListItem* new_item = (ListItem*)malloc(sizeof(ListItem));
    
    if(!new_item)
        return 0;    
    
    new_item->value = value;
    new_item->next = (ListItem*)0;
        
    if(!list->root_item) {
        
        list->root_item = new_item;
        list->current_item = new_item;
    } else {
        
        current_item = list->root_item;
        
        while(current_item->next)
            current_item = current_item->next;
            
        new_item->prev = current_item;
        current_item->next = new_item; 
    }
    
	list->count++;
	
    return 1;
}

void* List_get_next(List* list) {
    
    void* ret_val;
    
    if(!list->current_item) 
        return (void*)0;
    
    ret_val = list->current_item->value;
    
    if(list->current_item)
        list->current_item = list->current_item->next;
        
    return ret_val;
}

void List_seek_to(List* list, int index) {
	
	list->current_item = List_get_at(list, index);
}

void* List_get_at(List* list, int index) {
    
    ListItem* cur_item = list->root_item;
    
    if(index < 0) 
	    index = 0;
    
    if(index >= list->count)
        index = list->count - 1;  
    
    while(index) {
		
        cur_item = cur_item->next;
	    index--;
	}
    
    return cur_item ? cur_item->value : cur_item;
}

//Finds the first instance of the pointer value in the list, -1 if not found
int List_get_index(List* list, void* value) {
	
	void* cmp_value;
	int i = 0;
	
	List_for_each(list, cmp_value, void*) {
		
		if(cmp_value == value)
		    break;
		
		i++;
	}
	
	if(i == list->count)
	    return -1;
    else
	    return i;
}

int List_has_next(List* list) {
    
    return !!(list->current_item);
}

void List_print(List* list, printer print_func) {
    
    void* value;
    
    List_for_each(list, value, void*) {
            
        print_func(value);
    }
}
