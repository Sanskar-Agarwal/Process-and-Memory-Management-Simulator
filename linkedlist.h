#ifndef LINKLIST_H
#define LISTLIST_H

//Represents each elements of linkedlist
typedef struct Node Node;

//Represents linkedlist
typedef struct Linklist Linklist;

//Creates new linkedlist and return pointer to it
Linklist *newLinklist();

//Creates new node with the provided content and returns pointer to it
Node *newNode(void *content);

//Inserts new node at the end of linkedlist 
void insert_Linklist(Linklist *list, void* content);

//Removes first node from list, returns its content and frees memory
void *pop_Linklist(Linklist *list);

//Returns content of first node in linkedlist without removing it
void *check_head(Linklist *list);

//frees memory occupied by entire linkedlist
void free_list(Linklist *list);

//Checks if list is empty
int is_empty(Linklist *list);

#endif