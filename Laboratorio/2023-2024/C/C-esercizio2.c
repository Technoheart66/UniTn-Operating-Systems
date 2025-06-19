// checkArgs.c

/*
2. Scrivere un’applicazione che definisce una lista di argomenti validi e legge quelli passati
alla chiamata verificandoli e memorizzando le opzioni corrette, restituendo un errore in
caso di un’opzione non valida.
*/

#include "stdio.h"
#include "string.h"
#define NELEMS(x) (sizeof(x) / sizeof((x)[0]))
typedef char bool;
#define TRUE 1
#define FALSE 0

bool isValid(char *argument, const char *valid[], int size)
{
  bool result = FALSE;
  for (int i = 0; i < size; i++)
  {
    if (strcmp(argument, valid[i]) == 0)
    {
      result = TRUE;
    }
  }
  return result;
}

int main(int argc, char *argv[])
{
  int returnCode = 0; // declaring and defining the return code as successful
  const char *valid[] = {"mela", "pera", "banana"};
  int sizeValid = (int)NELEMS(valid);
  printf("NEW: the size of the array of valid arguments is '%d'\n"
         "\tWe derived that using a macro (sizeof(variable) / sizeof(variable[0]))\n",
         sizeValid);
  if (argc > 1)
  {
    for (int i = 1; i < argc; i++)
    {
      if (isValid(argv[i], valid, sizeValid))
      {
        printf("SUCCESS: checked argument '%s'\n", argv[i]);
      }
      else
      {
        printf("FAIL: checked argument '%s'\n", argv[i]);
      }
    }
  }
  else
  {
    printf("No arguments have been passed\n"
           "USAGE: ./a.out arg1 arg2 arg3 etc.\n"
           "\tPass me some fruit names in italian\n");
  }
  return 0;
}