#ifndef __MONLINKEDLIST_H__
#define __MONLINKEDLIST_H__

#include <stdio.h>
#include <stdlib.h>

typedef struct monitoringChildren MonChild;
struct monitoringChildren {
    pid_t monPID;
    pid_t childPID;
    char name[1024];
    char secs[1024];
    MonChild *next;
};

int monLL_len(MonChild *node_head); //monLL length
void monLL_push(MonChild **node_head, pid_t monPID, pid_t childPID, char name[1024], char secs[1024]); //pushes a value d onto the monLL
int monLL_pop(MonChild **node_head); //removes the head from the monLL & returns its value
void monLL_print(MonChild **node_head); //prints all the monLL data
void monLL_clear(MonChild **node_head); //clears the monLL of all elements
void monLL_snoc(MonChild **node_head, pid_t monPID, pid_t childPID, char name[1024], char secs[1024]); //appends a node
int monLL_elem(MonChild **node_head, pid_t monPID); //checks for an element
MonChild *monLL_get(MonChild **node_head, pid_t monPID); 
MonChild *monLL_remove(MonChild *node_head, pid_t monPID); //checks for an element and removes it
void monLL_insert(MonChild **node_head, pid_t childPID, pid_t monPID); 
#endif