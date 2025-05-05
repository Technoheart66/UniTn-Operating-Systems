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
void create_child_at_level(char *level);

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
    char command_num[4];  // store the integer command value here
    /*
    sscanf() function returns the number of fields that were successfully converted and assigned
    TRUE if it successfully converted and assigned to a variable
    FALSE otherwise
    */
    int matches = sscanf(input_line, "%s %d", command_text, command_num);
    if (sscanf(input_line, "%s %d", command_text, command_num) == 1)
    {
      printf("Your input: {%s}\n", input);
      int i = 0; // for characters/letters
      int j = 0; // for numbers
      while (input[i] != NULL_CHAR)
      {
        // printf("carattere: %c\n", input[i]);
        if (is_number(&input[i]))
        {
          printf("This is a number!! %c\n", input[i]);
          command_num[j] = input[i];
          j++;
          i++;
        }
        else
        {
          if (input[i] != ' ')
          {
            command_text[i] = input[i];
            // printf("Saving: {%c}\n", command_text[i]);
          }
          else
          {
            printf("a space\n");
          }
        }
        // Before exiting remember to increment the index by one
        i++;
      }
      // Since the while interrupts if we reached EOF then we need to check afterwards and insert it manually
      if (input[i] == NULL_CHAR)
      {
        command_text[i] = '\0';
        command_num[j] = '\0';
      }
      // Print the saved command for debugging purposes
      int stampa = 0;
      printf("This is the command: {");
      while (command_text[stampa] != NULL_CHAR)
      {
        printf("%c", command_text[stampa]);
        stampa++;
      }
      printf("}\n");
      stampa = 0;
      printf("This is the number: {");
      while (command_num[stampa] != NULL_CHAR)
      {
        printf("stampa numero\n");
        printf("%c", command_num[stampa]);
        stampa++;
      }
      printf("}\n");
    }
    else
    {
      printf("Wrong input!\nYour input: {%s}\n", input_line);
    }

    // Let's parse the input and choose what to do now
    if (strcmp(command_text, "child") == 0) // if the strings are exactly equal, accountinf for the null char as well
    {
      printf("This is the child command\n");
      create_child_at_level(command_num);
    }
  }
}

void create_child_at_level(char *level)
{
  int n = atoi(level);
  printf("Inside function! Level: {%d}\n", n);
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