// manipulateString.c

/*
3. Realizzare funzioni per stringhe
char *stringrev(char * str) inverte ordine caratteri
int stringpos(char * str, char chr) cerca chr in str e restituisce la posizione
*/

#include "stdio.h"
#include "string.h"
#define MAXLENGTH 50 // max length of string

char *stringRev(char *str)
{
  int length = strlen(str);
  char tmp[MAXLENGTH + 1];
  int strEnd = length;
  for (int i = 0; i < length; i++)
  {
    tmp[i] = str[--strEnd];
  }
  tmp[length] = 0;
  str = tmp;
  return str;
}

int main(int argc, char *argv[])
{
  if (argc == 1)
  {
    char *pluto = "pluto";
    char *reversed = stringRev(pluto);
    printf("The original string was '%s', the reversed string is '%s'\n", pluto, reversed);
  }
  else
  {
    printf("ERROR: wrong number of arguments\n"
           "USAGE: ./a.out\n");
  }

  return 0;
}