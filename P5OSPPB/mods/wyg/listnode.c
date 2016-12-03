#include "inttypes.h"
#include "../include/memory.h"
#include "listnode.h"


//================| ListNode Class Implementation |================//

//Basic listnode constructor
ListNode* ListNode_new(Object* payload) {

    //Malloc and/or fail null
    ListNode* list_node;
    if(!(list_node = (ListNode*)malloc(sizeof(ListNode))))
        return list_node;

    //Assign initial properties
    Object_init((Object*)list_node, 0);
    list_node->prev = (ListNode*)0;
    list_node->next = (ListNode*)0;
    list_node->payload = payload; 

    return list_node;
}

