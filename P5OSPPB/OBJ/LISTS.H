#ifndef LISTS_H
#define LISTS_H

typedef struct listItem {
        struct listItem* next;
        void* data; 
}

typedef struct list {
        int count;
        listItem* rootItem;
        listItem* itrPtr;
} list;

list* newList();
void listRewind(list* listObj);
void listNext(list* listObj);
void listAdd(list* thisList, void* newItem);
void listRemoveObj(list* thisList, void* targetItem);
void listRemove(list* thisList, int itemIndex);
void* getListItem(list* thisList, int itemIndex);
void listDelete(list* thisList);

#endif
