#include "inttypes.h"
#include "../include/memory.h"
#include "list.h"


//================| ListNode Class Implementation |================//

//Basic list constructor
List* List_new() {
    
    //Malloc and/or fail null
    List* list;
    if(!(list = (List*)malloc(sizeof(List))))
        return list;

    //Fill in initial property values
    //(All we know for now is that we start out with no items) 
    Object_init((Object*)list, List_delete);
    list->count = 0;
    list->root_node = (ListNode*)0;

    return list;
}

//Insert a payload at the end of the list
//Zero is fail, one is success
int List_add(List* list, Object* payload) {

    //Try to make a new node, exit early on fail 
    ListNode* new_node;
    if(!(new_node = ListNode_new(payload))) 
        return 0;

    //If there aren't any items in the list yet, assign the
    //new item to the root node
    if(!list->root_node) {
 
        list->root_node = new_node;        
    } else {

        //Otherwise, we'll find the last node and add our new node after it
        ListNode* current_node = list->root_node;

        //Fast forward to the end of the list 
        while(current_node->next)
            current_node = current_node->next;

        //Make the last node and first node point to each other
        current_node->next = new_node;
        new_node->prev = current_node; 
    }

    //Update the number of items in the list and return success
    list->count++;

    return 1;
}

//Get the payload of the list item at the given index
//Indices are zero-based
Object* List_get_at(List* list, unsigned int index) {

    unsigned int current_index;
    ListNode* current_node;

    //If there's nothing in the list or we're requesting beyond the end of
    //the list, return nothing
    if(list->count == 0 || index >= list->count) 
        return (Object*)0;

    //Iterate through the items in the list until we hit our index
    current_node = list->root_node;

    //Iteration, making sure we don't hang on malformed lists
    for(current_index = 0; (current_index < index) && current_node; current_index++)
        current_node = current_node->next;

    //Return the payload, guarding against malformed lists
    return current_node ? current_node->payload : (Object*)0;
}

//Remove the item at the specified index from the list and return the item that
//was removed
//Indices are zero-based
Object* List_remove_at(List* list, unsigned int index) {

    //This operates very similarly to List_get_at

    Object* payload; 
    ListNode* current_node;
    unsigned int current_index;

    //Bounds check
    if(list->count == 0 || index >= list->count) 
        return (Object*)0;

    //Iterate through the items
    current_node = list->root_node;

    for(current_index = 0; (current_index < index) && current_node; current_index++)
        current_node = current_node->next;

    //This is where we differ from List_get_at by stashing the payload,
    //re-pointing the current node's neighbors to each other and 
    //freeing the removed node 

    //Return early if we got a null node somehow
    if(!current_node)
        return (Object*)0;

    //Stash the payload so we don't lose it when we delete the node     
    payload =  current_node->payload;
 
    //Re-point neighbors to each other 
    if(current_node->prev)
        current_node->prev->next = current_node->next;

    if(current_node->next)
        current_node->next->prev = current_node->prev;

    //If the item was the root item, we need to make
    //the node following it the new root
    if(index == 0)
        list->root_node = current_node->next;

    //Now that we've clipped the node out of the list, we must free its memory
    Object_delete((Object*)current_node); 

    //Make sure the count of items is up-to-date
    list->count--; 

    //Finally, return the payload
    return payload;
}

//Delete a list as well as any items which it still contains
//As such, shouldn't be used if there are any objects in the 
//list referenced elsewhere or else you'll have dangling pointer
//issues
void List_delete(Object* list_object) {

    List* list = (List*)list_object;

    if(!list_object)
        return;

    //Remove each item from the list and pass it to its deleter
    while(list->count)
        Object_delete(List_remove_at(list, 0));

    //And delete the list itself
    free(list);
}
