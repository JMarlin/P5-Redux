#ifndef LISTNODE_H
#define LISTNODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "object.h"

//================| ListNode Class Declaration |================//

//A type to encapsulate an individual item in a linked list
typedef struct ListNode_struct {
    Object object;
    Object* payload;
    struct ListNode_struct* prev;
    struct ListNode_struct* next;
} ListNode;

//Methods
ListNode* ListNode_new(Object* payload); 

#ifdef __cplusplus
}
#endif

#endif //LISTNODE_H
