// prova_esame_1_07062025.c

// Section 0: author & introduction
// Author: Francesco Dall'Agata UniTN 221451
// Introduction: prova d'esame caricata su moodle "Prova esame 1",
//               tentativo del 07/06/2025, makefile del 06/06/2025

// Section 1: index
// 0) author & introduction
// 1) index
// 2) include
// 3) define
// 4) typedef
// 5) global variables
// 6) function declaration
// 7) main
// 8) function definition

// Section 2: include
#include <stdlib.h>  // EXIT_SUCCESS, EXIT_FAILURE etc.
#include <stdio.h>   // printf(), fprintf(), fopen() etc.
#include <string.h>  // strcmp(), strlen() etc.
#include <signal.h>  // kill(), sigaction(), sigprocmask() etc.
#include <sys/msg.h> // QUEUE -> msgget()
#include <fcntl.h>   // FILE DESCRIPTORS -> fcntl()

// Section 3: define
#define STR_INPUT_BUFFER 256

int main(int argc, char *argv[])
{
  unsigned short int exit_value = EXIT_SUCCESS;

  // check if there is 1 and only 1 parameter
  if (argc != 2) // two since program + param = 2
  {
    fprintf(stderr, "%s%s", "ERROR: wrong parameters\n", "USAGE: ./a.out <∕path∕to∕file.txt>\n");
    exit_value = 11; // custom value, 11 = wrong parameters
    return exit_value;
  }

  // open the file or return error
  FILE *input_file;
  input_file = fopen(argv[1], "r"); // [0] program, [1] param, [2] param etc.
  if (input_file == NULL)
  {
    fprintf(stderr, "%s%s", "ERROR: couldn't open input file\n", "USAGE: ./a.out <path/to/file.txt\n");
    exit_value = 12; // custom value, 12 = wronf filepath
    return exit_value;
  }

  // create queue for later use

  unsigned short int queue_id = msgget();

  // read the file until it is empty
  char buffer_from_file[STR_INPUT_BUFFER];
  unsigned short int counter = 0;
  while (!feof(input_file))
  {
    counter++;
    // getline(); // not part of C standard, it is a POSIX extension, it can automatically allocate memory
    fgets(buffer_from_file, sizeof(buffer_from_file), input_file); // read one line
    char command[STR_INPUT_BUFFER];                                // store the command
    char option_one[STR_INPUT_BUFFER / 2];                         // store the first command argument
    char option_two[STR_INPUT_BUFFER / 2];                         // store the second command argument
    int matches = sscanf(buffer_from_file, "%s %s %s", command, option_one, option_two);
    switch (matches)
    {
    case 0:
      fprintf(stderr, "%d) ERROR: couldn't parse any command\n", counter);
      break;

    case 1:
      fprintf(stderr, "%d) ERROR: couldn't parse command arguments '%s <arg1> <arg2>'\n", counter, command);
      break;

    case 2:
      fprintf(stderr, "%d) ERROR: couldn't parse last command argument '%s %s <arg2>'\n", counter, command, option_one);
      break;

    case 3:                             // correct parsing, all 3 variables are set
      if (strcmp("kill", command) == 0) // if the command is kill
      {
        // kill <signo> <pid>
        printf("%d) kill %s %s\n", counter, option_one, option_two);
        kill(option_one, option_two);
      }
      else if (strcmp("queue", command) == 0) // if the command is queue
      {
        // queue <category> <word>
        printf("%d) queue %s %s\n", counter, option_one, option_two);
      }
      else if (strcmp("fifo", command) == 0) // if the command is fifo
      {
        // fifo <name> <word>
        printf("%d) fifo %s %s\n", counter, option_one, option_two);
      }
      else
      {
        fprintf(stderr, "%d) ERROR: unspecified command or not implemented '%s %s %s'\n", counter, command, option_one, option_two);
      }

      break;

    default: // less than 0 or more than 3 yields a generic error
      fprintf(stderr, "%d) ERROR: parsing error '%s'\n", counter, buffer_from_file);
      break;
    }
  }

  return exit_value;
}
