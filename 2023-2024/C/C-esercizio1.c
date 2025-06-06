// lengthOf.c

/*
1. Scrivere un’applicazione che data una stringa come argomento ne stampa a video la
lunghezza, ad esempio:
./lengthof "Questa frase ha 28 caratteri"
deve restituire a video il numero 28 senza usare strlen
*/

//#include <cstdio> doesn't work with gcc but it does with g++
#include <stdio.h>

int main(int argc, char *argv[])
{
  if (argc == 2)
  {
    // int result = sizeof(tmp); this is wrong becuase it just returns the variable size, which should be 8 bytes
    char *tmp = argv[1]; // copy pointer to first argument, be careful not to see argv as a string
    int result = 0;
    while (*tmp != 0)
    {
      tmp++;    // increase the pointer to the next location or else the while will never end
      result++; // since the condition is satisfied we increase the number of characters
    }
    printf("Without using strlen()\n"
           "This phrase passed as an argument has exactly %d characters\n",
           result);
  }
  else
  {
    printf("ERROR: wrong number of arguments\n"
           "USAGE: ./a.out \"Just one Argument\" or justOneArgument\n");
  }
  return 0;
}