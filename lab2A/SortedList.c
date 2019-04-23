#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <string.h>
#include "SortedList.h"


void SortedList_insert (SortedList_t *list, SortedListElement_t *element){
    SortedList_t *head = list;
    SortedList_t *curr = head->next;
    
    while (curr != list){
        if (strcmp(element->key, curr->key) < 0) break;
        if (opt_yield & INSERT_YIELD) sched_yield();
        curr = curr->next;
    }
    element->prev = curr->prev;
    element->next = curr;
    curr->prev->next = element;
    curr->prev = element;
}

int SortedList_delete(SortedListElement_t *element){
    if (element->prev->next != element || element->next->prev != element) return -11;
    if (opt_yield & DELETE_YIELD)sched_yield();
    element->prev->next = element->next;
    element->next-> prev = element->prev;
    return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
    SortedList_t *curr = list->next;

    while (curr != list){
        if (strcmp(key, curr->key) == 0) return curr;
        if (opt_yield & LOOKUP_YIELD) sched_yield();
        curr = curr->next;
    }
    return NULL;
}

int SortedList_length(SortedList_t *list){
    int length = 0;
    SortedList_t *curr = list->next;
    while (curr != list){
        if (curr->prev->next != curr || curr->next->prev != curr) return -1;
        else {
            length++;
            if (opt_yield & DELETE_YIELD)sched_yield();
            curr = curr->next;
        }
    }
    return length;
}