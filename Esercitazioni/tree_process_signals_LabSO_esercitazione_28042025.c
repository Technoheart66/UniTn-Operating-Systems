// Esercitazione Laboratorio Sistemi Operativi 28/04/2025

// Svolto il 02/05/2025

/*
Creare un’applicazione in C che gestisca un albero di processi tramite dei segnali.
In particolare, il programma, una volta lanciato, deve accettare i seguenti comandi:
  ● child n--> crea nuovi figli al livello n
  ● kill n --> termina i figli al livello n
  ● print --> stampa in output l’albero dei processi
  ● quit --> termina il processo dopo aver terminato tutti i figli

L’intera comunicazione deve avvenire esclusivamente tramite segnali inviati dal processo principale.
L’output del comando ‘p’ non deve essere ordinato ma deve essere ben chiaro il livello di ogni-
  processo (per esempio usando la tabulazione).
*/

// Include section
#include <stdio.h>    // printf, sscanf
#include <stdlib.h>   // atoi, rand, srand, malloc, free, exit, EXIT_SUCCESS, EXIT_FAILURE, size_t etc.
#include <unistd.h>   // getpid, getppid
#include <stdbool.h>  // Boolean datatypes
#include <string.h>   // strcmp
#include <sys/wait.h> // wait

// Define section
#define NULL_CHAR '\0'
#define ASCII_0 48
#define ASCII_9 57

// I need a way to store each child information, so let's do a linked list
typedef struct node
{
  int level;
  pid_t pid;
  pid_t ppid;
  struct node *next;
} node_t;

// Function declaration section
int take_input(char *input_line, int *size, FILE *stream, pid_t *origin, node_t *head);
bool is_number(char *c);
int create_child_at_level(int *input_level, pid_t *origin, node_t *head);
void handler_origin(int signo, siginfo_t *info, void *empty);
void print_tree(node_t *head, pid_t *origin); // unordered
void quit(node_t *head, pid_t *origin);

int main()
{
  // scope: main
  /*
    Origin signal mask
    Here I want to make the original process wait for a signal sent by the child before printing again
    Let's create a custom data structure of type sigset_t to edit the signal mask, remember that this-
      is the only way to edit the signal mask as it is not directly accessible.
  */
  sigset_t sigset_origin;
  int signal_origin;

  sigemptyset(&sigset_origin);                  // let's ensure the new signal mask is empty before we edit it
  sigaddset(&sigset_origin, SIGUSR1);           // adding SIGUSR1, the child will send it when he is done
  sigprocmask(SIG_BLOCK, &sigset_origin, NULL); // apply the new set, this will change the current mask

  /*
    Origin signal handling
    Here I want to handle the signal SIGUSR1
    Notice how we set the struct mask to the new mask defined above!
  */
  struct sigaction sa_origin;
  sa_origin.sa_sigaction = handler_origin;
  sa_origin.sa_flags = SA_SIGINFO;      // use sa_sigaction
  sa_origin.sa_mask = sigset_origin;    // use the custom mask we defined before
  sigaction(SIGUSR1, &sa_origin, NULL); // set our custom handler for SIGUSR1

  pid_t origin = getpid(); // the original process PID, will be shared beteen forked processes

  printf("- - - S T A R T - - -\n");
  printf("PID: [%d]\n", origin); // It works also without <unistd.h> but it gives a warning
  printf("PPID: [%d]\n", getppid());

  // counter to print how many commands have been sent
  int operation_counter = 1;
  bool exit = false;

  char input_line[256]; // max input line dimension
  int input_line_dim = sizeof(input_line);

  node_t *head = NULL;
  head = (node_t *)malloc(sizeof(node_t));
  head->level = 0;
  head->next = NULL;
  head->pid = origin;

  do
  {
    if (origin == getpid())
    {
      sleep(1);
      printf("\nCurrent operation: #%d\n", operation_counter);
      printf("Waiting for user input, choose between:\n - child n\n - kill n\n - print\n - quit\n> ");
      exit = take_input(input_line, &input_line_dim, stdin, &origin, head);
      operation_counter++;
      if (sigwait(&sigset_origin, &signal_origin) == 0)
      {
        printf("-> sigwait works\n");
      }
      else
      {
        perror("-> sigwait failed\n");
      }
    }
  } while (exit);
  return EXIT_SUCCESS;
}

// Function definition section
int take_input(char *input_line, int *size, FILE *stream, pid_t *origin, node_t *head)
{
  int result = false;
  /*
  Remember that
    - sscanf() reads formatted input from a string.
    - scanf() reads formatted input from stdin.
  */
  /*
  fgets() returns pointer to the string buffer if successful, NULL or if EOF or an error occurred
  TRUE if point
  FALSE if NULL or EOF
  */
  if (fgets(input_line, *size, stream))
  {
    char input_copy[256]; // actual converted value in a new variable, safer because it is a backup
    char command_text[8]; // store the text command value here
    int command_num;      // store the integer command value here

    snprintf(input_copy, sizeof(input_copy), "%s", input_line);
    /*
    sscanf() function returns the number of fields that were successfully converted and assigned
    n if it successfully converted and assigned to n variable up to that point
    EOF it it couldn't assign any input to a variable
    */
    // Remember: no need to explicitly put &command_text[0] since in the variable command_text is stored the pointer to the first element
    int matches = sscanf(input_line, "%s %d", command_text, &command_num);
    result = true; // continue unless specified otherwise
    switch (matches)
    {
    case 1:
      if (strcmp(command_text, "print") == 0)
      {
        print_tree(head, origin);
      }
      else if (strcmp(command_text, "quit") == 0)
      {
        quit(head, origin); // inside here every node should be freed
        result = false;
      }
      else
      {
        printf("Unknown command\n");
        kill(*origin, SIGUSR1);
      }
      break;
    case 2:
      if (strcmp(command_text, "child") == 0)
      {
        create_child_at_level(&command_num, origin, head); // signals are sent inside this function
      }
      else if (strcmp(command_text, "kill") == 0)
      {
        // kill
      }
      else
      {
        printf("Unknown command\n"); // two parameters but unknown commands or bad format
        kill(*origin, SIGUSR1);
      }
      break;
    default:
      printf("ERROR: error while parsing user input: %s\n", input_line);
      kill(*origin, SIGUSR1);
      break;
    }
  }
  else
  {
    printf("ERROR: user input error: %s\n", input_line);
    kill(*origin, SIGUSR1);
  }
  return result;
}

int create_child_at_level(int *input_level, pid_t *origin, node_t *head)
{
  /*
    For getpid() and getppid() it's important to remember that if we save them in a variable and then we call fork()
    they will always be the same, so we need to call getpid() again to know the child pid.
    Moreover if the parent process ends before the child then the child will be re-parented under another process
  */

  /*
    The childs need to do nothing and wait for the termination signal, only the parent is responsible of stdin/stdout.
    So let's make the childs wait for a signal, and the only signal they will receive is when they terminate.
    Let's create a custom data structure of type sigset_t to edit the signal mask, remember that this-
      is the only way to edit the signal mask as it is not directly accessible.
  */
  int result = 0;         // final result of this function
  sigset_t sigset_figlio; // signal mask set
  int signal_figlio;      // store the signal number received using sigwait()

  sigemptyset(&sigset_figlio);        // let's ensure the new signal mask is empty before we edit it
  sigaddset(&sigset_figlio, SIGTERM); // adding SIGTERM, the default software termination signal

  pid_t figlio = -1; // fork() returns -1 in case of error
  if ((*input_level) > 0)
  {
    printf("Creating child, levels to descend: {%d}\n", *input_level);
    figlio = fork();

    node_t *current = head;
    // new node
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    if (new_node == NULL)
    {
      fprintf(stderr, "Failed to allocate memory for new node\n");
      result = 1;
    }
    else
    {
      new_node->next = NULL;
      new_node->level = -1;

      // !!! IMPORTANT !!!
      // travel to the last node of the desired level
      while (current->next != NULL && current->level <= *input_level && current->next->level <= *input_level)
      {
        current = current->next;
      }

      // initialize the new node, corner-case: only origin
      if (current->level < *input_level)
      {
        new_node->level = current->level + 1;
      }
      else
      {
        new_node->level = current->level;
      }

      switch (figlio)
      {
      case -1: // fork() returns -1 in case of error
        printf("Error on fork() while creating a child\n");
        break;

      case 0: // fork() returns 0 to the child
      {
        printf("\nSono un figlio:\n\tinput_level{%d}, new_node->level{%d}, PID{%d}, PPID{%d}\n", *input_level, new_node->level, getpid(), getppid());
        if (new_node->level < *input_level)
        {
          printf("Sto facendo un altro figlio, devo arrivare al livello giusto\n");
          create_child_at_level(input_level, origin, head);
        }
        else
        {
          printf("Sono arrivato al livello giusto!\n");
        }
        kill(*origin, SIGUSR1); // before waiting indefinetly let's tell the parent that we are done

        do
        {
          // do nothing for now
        } while (sigwait(&sigset_figlio, &signal_figlio) == 0); // sigwait() returns 0 on success
        break;
      }

      default: // fork() returns the PID of the new child to the parent
        // edit the node
        new_node->pid = figlio;
        new_node->ppid = getpid();
        printf("Sono un genitore:\n\tinput_level{%d}, current->level{%d}, PID{%d}, PPID{%d}, Figlio{%d}\n", *input_level, current->level, getpid(), getppid(), figlio);
        break;
      }
    }
  }
  else
  {
    printf("Cannot create a child at input_level{%d}\nThere can only be the origin at level 0 and no childs before it\nUSAGE: child n, with n>0\n", *input_level);
    kill(*origin, SIGUSR1); // tell the parent that we are done
  }

  return result;
}

bool is_number(char *c)
{
  bool result = false;
  if (*c >= ASCII_0 && *c <= ASCII_9) // ASCII codes, check if it's a number
  {
    result = true;
  }
  return result;
}

void handler_origin(int signo, siginfo_t *info, void *empty)
{
  switch (signo)
  {
  case 10:
    printf("\nSIGSUR1 received!\n");
    break;

  default:
    printf("\nsignal unknown: {%d}", signo);
    break;
  }
}

void print_tree(node_t *head, pid_t *origin)
{
  if (*origin == getpid())
  {
    printf("printing...\n");
    node_t *current = head;
    while (current != NULL)
    {
      for (int i = 0; i < current->level; i++)
      {
        if (i == 0)
        {
          printf("|");
        }
        printf("--");
      }
      printf("[PID: %d, ", current->pid);
      if (getpid() == *origin)
      {
        printf("PPID: 0, ");
      }
      else
      {
        printf("PPID: %d, ", getppid());
      }
      printf("Depth: %d]\n", current->level);
      current = current->next;
    }
    kill(*origin, SIGUSR1); // tell the parent that we are done
  }
}

void quit(node_t *head, pid_t *origin)
{
  if (*origin == getpid())
  {
    printf("quitting...\n");
    kill(*origin, SIGUSR1); // tell the parent that we are done
  }
}