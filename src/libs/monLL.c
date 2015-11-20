#include "linkedList.h"

typedef struct monitoringChildren MonChild;
struct monitoringChildren {
    pid_t monPID;
    pid_t childPID;
    char name[1024]; 
    char secs[1024];
    MonChild *next;
};

int monLL_len(MonChild *node_head)
{
    MonChild *curr = node_head;
    int len = 0;
     
    while(curr){
        ++len;
        curr = curr -> next;
    }
    return len;
}
 
void monLL_push(MonChild **node_head, pid_t monPID, pid_t childPID, char name[1024], char secs[1024])
{
    MonChild *node_new = malloc(sizeof(MonChild));
     
    node_new -> monPID = monPID;
    node_new -> childPID = childPID;
    sprintf(node_new->name, "%s", name);
    sprintf(node_new->secs, "%s", secs);
    // node_new -> name = name;
    // node_new -> secs = secs;

    node_new -> next = *node_head;
    *node_head = node_new;
}
 
int monLL_pop(MonChild **node_head)
{
    MonChild *node_togo = *node_head;
    int d;
     
    if(node_head)
    {
        d = node_togo -> monPID;
        *node_head = node_togo -> next;
        free(node_togo);
    }
    return d;
}
 
void monLL_print(MonChild **node_head)
{
    MonChild *node_curr = *node_head;
     
    if(!node_curr)
        puts("the monLL is empty");
    else
    {
        while(node_curr)
        {
            printf("%d ", node_curr -> monPID); //set for integers, modifiable
            node_curr = node_curr -> next;
        }
        putchar('\n');
    }
}
 
void monLL_clear(MonChild **node_head)
{
    while(*node_head)
        monLL_pop(node_head);
}
 
void monLL_snoc(MonChild **node_head, pid_t monPID, pid_t childPID, char name[1024], char secs[1024])
{
    MonChild *node_curr = *node_head;
     
    if(!node_curr)
        monLL_push(node_head, monPID, childPID, name, secs);
    else
    {
        //find the last node
        while(node_curr -> next)
            node_curr = node_curr -> next;
        //build the MonChild after it
        monLL_push(&(node_curr -> next), monPID, childPID, name, secs);
    }
}
 
int monLL_elem(MonChild **node_head, pid_t monPID)
{
    MonChild *node_curr = *node_head;
     
    while(node_curr)
    {
        if(node_curr -> monPID == monPID) //set for numbers, modifiable
            return 1;
        else
            node_curr = node_curr -> next;
    }
    return 0;
}

MonChild *monLL_remove(MonChild *node_head, pid_t monPID){ //checks for an element and removes it

   /* See if we are at end of list. */
  if (node_head == NULL)
    return NULL;

  /*
   * Check to see if current node is one
   * to be deleted.
   */
  if (node_head->monPID == monPID) {
    MonChild *tempNextP;

    /* Save the next pointer in the node. */
    tempNextP = node_head->next;

    /* Deallocate the node. */
    free(node_head);

    /*
     * Return the NEW pointer to where we
     * were called from.  I.e., the pointer
     * the previous call will use to "skip
     * over" the removed node.
     */
    return tempNextP;
  }

  /*
   * Check the rest of the list, fixing the next
   * pointer in case the next node is the one
   * removed.
   */
  node_head->next = monLL_remove(node_head->next, monPID);


  /*
   * Return the pointer to where we were called
   * from.  Since we did not remove this node it
   * will be the same.
   */
  return node_head;

}

void monLL_insert(MonChild **node_head, pid_t childPID, pid_t monPID){
    MonChild *node_curr = *node_head;
     
    while(node_curr)
    {
        if(node_curr -> childPID == childPID){ //set for numbers, modifiable
            node_curr ->monPID = monPID;
            return;
        }else{
            node_curr = node_curr -> next;
        }
    }
    return;
}

MonChild *monLL_get(MonChild **node_head, pid_t monPID){
    
    MonChild *node_curr = *node_head;
     
    while(node_curr)
    {
        if(node_curr -> monPID == monPID) //set for numbers, modifiable
            return node_curr;
        else
            node_curr = node_curr -> next;
    }
    return NULL;
}