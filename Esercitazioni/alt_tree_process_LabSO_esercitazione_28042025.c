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

// Function declaration section
int take_input(char *input_line, int *size, FILE *stream, pid_t *origin);
bool is_number(char *c);
void create_child_at_level(int *levels_to_descend, pid_t *origin);
void handler_origin(int signo, siginfo_t *info, void *empty);

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
  do
  {
    if (origin == getpid())
    {
      printf("\nCurrent operation: #%d\n", operation_counter);
      printf("Waiting for user input, choose between:\n - child n\n - kill n\n - print\n - quit\n> ");
      exit = take_input(input_line, &input_line_dim, stdin, &origin);
      operation_counter++;
      if (sigwait(&sigset_origin, &signal_origin) == 0)
      {
        printf("-> sigwait works\n");
        sleep(1);
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
int take_input(char *input_line, int *size, FILE *stream, pid_t *origin)
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
    char input[256];      // actual converted value in a new variable, safer because it is a backup
    char command_text[8]; // store the text command value here
    int command_num;      // store the integer command value here
    /*
    sscanf() function returns the number of fields that were successfully converted and assigned
    TRUE if it successfully converted and assigned to a variable
    FALSE otherwise
    */
    // Remember: no need to explicitly put &command_text[0] since in the variable command_text is stored the pointer to the first element
    int matches = sscanf(input_line, "%s %d", command_text, &command_num);
    result = true; // let's continue executing sice we succesfully passed something to parse
    switch (matches)
    {
    case 1:
      if (strcmp(command_text, "print") == 0)
      {

        printf("Printing process tree...\n");
        // result is already true let's continue
        kill(*origin, SIGUSR1); // tell the parent that we are done
        // print_tree();
      }
      else if (strcmp(command_text, "quit") == 0)
      {
        printf("Quitting...\n");
        result = false;         // set the return value to false so that the origin can understand to terminate
        kill(*origin, SIGUSR1); // tell the parent that we are done
      }
      else
      {
        printf("Unknown command: %s\n", input_line);
        kill(*origin, SIGUSR1); // tell the parent that we are done
        // result is already true let's continue
      }
      break;
    case 2:
      if (strcmp(command_text, "child") == 0)
      {
        // let's create a single child at level n
        printf("Creating a child at level: {%d}\n", command_num);
        create_child_at_level(&command_num, origin); // signals are sent inside this function
        // result is already true, let's continue
      }
      else if (strcmp(command_text, "kill") == 0)
      {
        printf("Killing processes at level %d\n", command_num);
        // kill_level(number);
        // result is already true let's continue
      }
      else
      {
        printf("Unknown command: %s\n", input_line);
        // result is already true, let's continue
        kill(*origin, SIGUSR1); // tell the parent that we are done
      }
      break;
    default:
      printf("There was an error parsing user input\n");
      kill(*origin, SIGUSR1); // tell the parent that we are done
      break;
    }
  }
  else
  {
    printf("\nWrong input!\nYour input: {%s}\nQuitting...\n", input_line);
    kill(*origin, SIGUSR1); // tell the parent that we are done
  }
  return result;
}

void create_child_at_level(int *levels_to_descend, pid_t *origin)
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
  sigset_t sigset_figlio;
  int signal_figlio;
  sigemptyset(&sigset_figlio);        // let's ensure the new signal mask is empty before we edit it
  sigaddset(&sigset_figlio, SIGTERM); // adding SIGTERM, the default software termination signal

  printf("Creating child, levels to descend: {%d}\n", *levels_to_descend);
  pid_t figlio = -1; // fork() returns -1 in case of error
  if ((*levels_to_descend) > 0)
  {
    figlio = fork();
  }
  else
  {
    printf("I cannot create a child at level 0!\n");
    kill(*origin, SIGUSR1); // tell the parent that we are done
  }

  switch (figlio)
  {
  case -1:
    /*
    fork() returns -1 in case of error
    so this is an error handling section
    */
    printf("Error on fork() while creating a child\n");
    break;

  case 0:
    /*
    to the newly created child process fork() returns 0
    so this is the child
    */
    (*levels_to_descend)--;
    printf("\nSono un figlio:\n\tlevels_to_descend{%d}, PID{%d}, PPID{%d}\n", *levels_to_descend, getpid(), getppid());
    if ((*levels_to_descend) > 0)
    {
      printf("Sono un figlio e sto facendo un altro figlio!\n");
      create_child_at_level(levels_to_descend, origin);
    }
    else
    {
      printf("Sono un figlio e non ci sono più figli da fare!\n");
    }
    kill(*origin, SIGUSR1); // before waiting indefinetly let's tell the parent that we are done

    do
    {
      // do nothing for now
    } while (sigwait(&sigset_figlio, &signal_figlio) == 0); // sigwait() returns 0 on success
    break;

  default:
    /*
    to the parent fork() returns the pid of the newly created process
    so this is the parent
     */
    printf("Sono un genitore:\n\tlevels_to_descend{%d}, PID{%d}, PPID{%d}, Figlio{%d}\n", *levels_to_descend, getpid(), getppid(), figlio);
    break;
  }
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