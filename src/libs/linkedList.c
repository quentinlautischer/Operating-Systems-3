#include "linkedList.h"

int stack_len(BoredChild *node_head)
{
    BoredChild *curr = node_head;
    int len = 0;
     
    while(curr){
        ++len;
        curr = curr -> next;
    }
    return len;
}
 
void stack_push(BoredChild **node_head, int d)
{
    BoredChild *node_new = malloc(sizeof(BoredChild));
     
    node_new -> pid = d;
    node_new -> next = *node_head;
    *node_head = node_new;
}
 
int stack_pop(BoredChild **node_head)
{
    BoredChild *node_togo = *node_head;
    int d;
     
    if(node_head)
    {
        d = node_togo -> pid;
        *node_head = node_togo -> next;
        free(node_togo);
    }
    return d;
}
 
void stack_print(BoredChild **node_head)
{
    BoredChild *node_curr = *node_head;
     
    if(!node_curr)
        puts("the stack is empty");
    else
    {
        while(node_curr)
        {
            printf("%d ", node_curr -> pid); //set for integers, modifiable
            node_curr = node_curr -> next;
        }
        putchar('\n');
    }
}
 
void stack_clear(BoredChild **node_head)
{
    while(*node_head)
        stack_pop(node_head);
}
 
void stack_snoc(BoredChild **node_head, int d)
{
    BoredChild *node_curr = *node_head;
     
    if(!node_curr)
        stack_push(node_head, d);
    else
    {
        //find the last node
        while(node_curr -> next)
            node_curr = node_curr -> next;
        //build the BoredChild after it
        stack_push(&(node_curr -> next), d);
    }
}
 
int stack_elem(BoredChild **node_head, int d)
{
    BoredChild *node_curr = *node_head;
     
    while(node_curr)
    {
        if(node_curr -> pid == d) //set for numbers, modifiable
            return 1;
        else
            node_curr = node_curr -> next;
    }
    return 0;
}