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
#include <stdio.h>   // printf, sscanf
#include <stdlib.h>  // atoi, rand, srand, malloc, free, exit, EXIT_SUCCESS, EXIT_FAILURE, size_t etc.
#include <unistd.h>  // getpid, getppid
#include <stdbool.h> // Boolean datatypes
#include <string.h>  // strcmp

// Define section
#define NULL_CHAR '\0'
#define ASCII_0 48
#define ASCII_9 57

// Function declaration section
void take_input(char *input_line, int *size, FILE *stream);
bool is_number(char *c);
void create_child_at_level(int *level);

int main()
{
  printf("- - - S T A R T - - -\n");
  printf("PID: [%d]\n", getpid()); // It works also without <unistd.h> but it gives a warning
  printf("PPID: [%d]\n", getppid());

  // Taking user input
  printf("Waiting for user input, choose between:\n - child n\n - kill n\n - print\n - quit\n> ");
  char input_line[256]; // max input line dimension
  int input_line_dim = sizeof(input_line);
  take_input(input_line, &input_line_dim, stdin);

  return EXIT_SUCCESS;
}

// Function definition section
void take_input(char *input_line, int *size, FILE *stream)
{
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

    switch (matches)
    {
    case 1:
      if (strcmp(command_text, "print") == 0)
      {
        printf("Printing process tree...\n");
        // print_tree();
      }
      else if (strcmp(command_text, "quit") == 0)
      {
        printf("Quitting...\n");
        // quit_program();
      }
      else
      {
        printf("Unknown command: %s\n", command_text);
      }
      break;
    case 2:
      if (strcmp(command_text, "child") == 0)
      {
        printf("Creating %d child processes\n", command_num);
        // Let's parse the input and choose what to do now
        create_child_at_level(&command_num);
        // create_child(number);
      }
      else if (strcmp(command_text, "kill") == 0)
      {
        printf("Killing processes at level %d\n", command_num);
        // kill_level(number);
      }
      else
      {
        printf("Unknown command with number: %d\n", command_num);
      }
      break;
    default:
      printf("There was an error parsing user input\n");
      break;
    }
  }
  else
  {
    printf("Wrong input!\nYour input: {%s}\n", input_line);
  }
}

void create_child_at_level(int *level)
{
  printf("Inside function! Level: {%d}\n", *level);
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