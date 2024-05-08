#include <stdio.h>
#include <stdlib.h>
#include "linkedlist.h"

typedef struct Node{
    void *content;
    Node *next;
}Node;

typedef struct Linklist{
    Node *head;
    Node *tail;
}Linklist;


Linklist *newLinklist(){
    Linklist *list = (Linklist *)malloc(sizeof(Linklist));
    list -> head = NULL;
    list -> tail = NULL;
    return list;
}

Node *newNode(void* content){
    Node *node = (Node *)malloc(sizeof(Node));
    node -> content = content;
    node -> next = NULL;
    return node;
}

void insert_Linklist(Linklist *list, void* content){
    if(list -> head == NULL){
        //List is empty
        Node *node = newNode(content);
        list -> head = node;
        list -> tail = node;
    }
    else{
        //List is non-empty, insert to tail
        Node *node = newNode(content);
        list -> tail -> next = node;
        list -> tail = node;
    }
}

/*Returns content field of linklist head, removes head*/
void *pop_Linklist(Linklist *list){
    Node *node;
    void *content;
    if(list -> head == NULL) return NULL;
    node = list -> head;
    list -> head = node -> next;
    if(list -> head == NULL) list -> tail = NULL;
    content = node -> content;
    free(node);
    return content;
}

/*Returns content field of head node*/
void *check_head(Linklist *list){
    if(list -> head == NULL) return NULL;
    return(list -> head -> content);
}

/*Frees linked list*/
void free_list(Linklist *list){
    Node *curr = NULL;
    Node *next = NULL;

    next = list -> head;
    while(next != NULL){
        curr = next;
        next = next -> next;
        free(curr);
    }
    free(list);
}

int is_empty(Linklist *list){
    if(list -> head == NULL) return 1;
    else return 0;
}