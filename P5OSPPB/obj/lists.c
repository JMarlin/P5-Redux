#include "lists.h"
#include "../memory/memory.h"


//Allocate memory for a new list structure and return it
list* newList() {
        
    list* returnList;
     
    returnList = (list*)kmalloc(sizeof(list));
    returnList->count = 0;
    returnList->rootItem = (listItem*)0;
    returnList->itrPtr = (listItem*)0;
    return returnList;
}


listItem* getTailItem(listItem* rootItem) {
        
    listItem* nextItem = rootItem;
    listItem* prevItem = rootItem;    

    while(nextItem) {
        prevItem = nextItem;
        nextItem = nextItem->next;
    }

    return prevItem;
}


void listRewind(list* listObj) {

    listObj->itrPtr = listObj->rootItem;
}


listItem* listNext(list* listObj) {

    listItem* retItem = listObj->itrPtr;
    
    if(retItem->next)
        listObj->itrPtr = retItem->next;
    
    return retItem;        
}


void listAdd(list* thisList, void* newObj) {
    
    listItem* newItem;
    listItem* tailItem = getTailItem(thisList->rootItem);

    newItem = (listItem*)kmalloc(sizeof(listItem));
    newItem->next = (listItem*)0;
    newItem->data = newObj;
    
    if(tailItem)
        tailItem->next = newObj;
    else
        thisList->rootItem = newItem;   

    thisList->count++;
}


void listRemoveObj(list* thisList, void* targetItem) {
        
    listItem* nextItem = thisList->rootItem;
    listItem* prevItem = (listItem*)0;

    if(thisList->count == 0)
        return;

    while(1) {

        if(nextItem->data == targetItem) {
         
            if(prevItem == (listItem*)0)
                thisList->rootItem = nextItem->next;
            else
                prevItem = nextItem->next;
   
            kfree(nextItem);
            thisList->count--;                                               
            return;
        }
        
        if(nextItem->next) {
            prevItem = nextItem;
            nextItem = nextItem->next;
        }else{
            return;
        }
    }
}

void listRemove(list* thisList, int itemIndex) {
    
    listItem* nextItem = thisList->rootItem;
    listItem* prevItem = (listItem*)0;
    int itemCounter = 0;

    if(thisList->count <= itemIndex)
            return;

    while(1) {            

        if(itemCounter == itemIndex) {
            
            if(prevItem == (listItem*)0)
                thisList->rootItem = nextItem->next;
            else
                prevItem = nextItem->next;
            
            kfree(nextItem);
            thisList->count--;                                               
            return;
        }
        
        if(nextItem->next) {
            prevItem = nextItem;
            nextItem = nextItem->next;
            itemCounter++;
        }else{
            return;
        }
    }
}

void* getListItem(list* thisList, int itemIndex) {
        
    listItem* nextItem = thisList->rootItem;
    listItem* prevItem = (listItem*)0;
    int itemCounter = 0;

    if(thisList->count <= itemIndex)
        return (void*)0;

    while(1) {            

        if(itemCounter == itemIndex)                                                
            return nextItem->data;
        
        if(nextItem->next) {
            prevItem = nextItem;
            nextItem = nextItem->next;
            itemCounter++;
        }else{
            return (void*)0;
        }
    }        
}


void listDelete(list* thisList) {
    
    while(thisList->count)
        listRemove(thisList, 0);

    kfree(thisList);
}
