#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <stdio.h>
#include <stdlib.h>


typedef struct boredChildren BoredChild;
struct boredChildren {
	int pid;
	BoredChild *next;
};

int stack_len(BoredChild *node_head); //stack length
void stack_push(BoredChild **node_head, int d); //pushes a value d onto the stack
int stack_pop(BoredChild **node_head); //removes the head from the stack & returns its value
void stack_print(BoredChild **node_head); //prints all the stack data
void stack_clear(BoredChild **node_head); //clears the stack of all elements
void stack_snoc(BoredChild **node_head, int d); //appends a node
int stack_elem(BoredChild **node_head, int d); //checks for an element

#endif