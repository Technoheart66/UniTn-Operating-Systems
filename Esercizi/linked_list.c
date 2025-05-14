// linked_list.c

// source: https://www.learn-c.org/en/Linked_lists

#include <stdlib.h>  // NULL, atoi(), rand(), srand(), malloc(), free(), exit(), EXIT_SUCCESS, EXIT_FAILURE, size_t etc.
#include <stdbool.h> // bool datatype, TRUE, FALSE
#include <stdio.h>   // printf

typedef struct node
{
  int val;
  struct node *next;
} node_t;

// function declaration section
void print_list(node_t *head);
void push(node_t *head, int new_val);
int pop(node_t **head);
int remove_last(node_t *head);
int remove_by_index(node_t **head, int n);
// TODO int remove_by_value; which receives a double pointer to the head and removes the first item in the list which has the value val


int main()
{
  // node_t * head = NULL;
  // head = (node_t *) malloc(sizeof(node_t));

  unsigned short int result = 0;

  node_t *head = NULL;
  head = (node_t *)malloc(sizeof(node_t));
  // malloc should return the pointer to the allocated memory space
  // if malloca fails it returns NULL
  if (head == NULL)
  {
    result = -1;
  }

  head->val = 19;
  head->next = NULL;

  print_list(head);

  push(head, 20);
  print_list(head);

  int popped = pop(&head);
  print_list(head);

  push(head, 19);
  push(head, 18);
  print_list(head);
  int last = remove_last(head);
  print_list(head);

  return result;
}

// iterating over a list (and print)
void print_list(node_t *head)
{
  node_t *current = head;

  printf("Printing list\n");
  while (current != NULL)
  {
    printf("%d\n", current->val);
    current = current->next;
  }
}

// adding an item at the end of the list
void push(node_t *head, int new_val)
{
  node_t *current = head;
  while (current->next != NULL)
  {
    current = current->next;
  }

  current->next = (node_t *)malloc(sizeof(node_t));
  current->next->val = new_val;
  current->next->next = NULL;
}

// remove the very first item of the list (and return it's val)
int pop(node_t **head)
{
  int result = -1;
  node_t *next_node = NULL;

  if (*head == NULL)
  {
    result = -1;
  }
  else
  {
    next_node = (*head)->next;
    result = (*head)->val;
    free(*head);
    *head = next_node;
  }

  return result;
}

int remove_last(node_t *head)
{
  int retval = 0;
  /* if there is only one item in the list, remove it */
  if (head->next == NULL)
  {
    retval = head->val;
    free(head);
  }
  else
  {
    /* get to the second to last node in the list */
    node_t *current = head;
    while (current->next->next != NULL)
    {
      current = current->next;
    }

    /* now current points to the second to last item of the list, so let's remove current->next */
    retval = current->next->val;
    free(current->next);
    current->next = NULL;
  }

  return retval;
}

int remove_by_index(node_t **head, int n)
{
  int i = 0;
  int retval = -1;
  node_t *current = *head;
  node_t *temp_node = NULL;

  if (n == 0)
  {
    return pop(head);
  }

  for (i = 0; i < n - 1; i++)
  {
    if (current->next == NULL)
    {
      return -1;
    }
    current = current->next;
  }

  if (current->next == NULL)
  {
    return -1;
  }

  temp_node = current->next;
  retval = temp_node->val;
  current->next = temp_node->next;
  free(temp_node);

  return retval;
}