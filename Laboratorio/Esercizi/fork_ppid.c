// fork_ppid.c

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

#define INPUT_LENGTH 8

int take_input(char *input_line, int *size, FILE *stream);

int main(int argc, char *argv[])
{
  printf("Try to guess the fork tree after 3 consecutive fork() calls\nPress ENTER to continue...");

  char input_line[INPUT_LENGTH]; // max input line dimension
  int input_line_dim = sizeof(input_line);

  take_input(input_line, &input_line_dim, stdin);

  fork();
  fork();
  fork();
  printf("Ciao! PID: %d, PPID: %d\n", getpid(), getppid());
  do
  {
    // wait indefinetely, to terminate use CTRL+C
  } while (1);

  return EXIT_SUCCESS;
}

int take_input(char *input_line, int *size, FILE *stream)
{
  int result = false; // default: fail/error

  if (fgets(input_line, *size, stream))
  {
    char input_copy[INPUT_LENGTH]; // input backup variable, good programming: in case of failure I can still retrieve the original input
    char parsed_input[8];          // for sscanf, store the input text value here

    snprintf(input_copy, sizeof(input_copy), "%s", input_line);

    int matches = sscanf(input_copy, "%c", parsed_input);
    switch (matches)
    {
    case 0:
      printf("no input, conitnuing...\n");
    case 1:
      printf("input parsed: ");
      if (parsed_input[0] == '\n')
      {
        printf("ENTER\n");
      } else {
        printf("%s", input_line);
      }
      printf("continuing...\n");
      break;

    default:
      printf("ERROR: input not parsed correctly, matches=%d\ncontinuing...\n", matches);
      break;
    }
  }
  else
  {
    printf("ERROR: user input error: %s\n", input_line);
  }
}
