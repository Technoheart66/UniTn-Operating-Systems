// C-foundamentals_0.c

// Macro and directives

// #include <cstdio> does not work using gcc but it does using g++
#include "stdio.h"
#include <string.h>

/*
https://stackoverflow.com/questions/37538/how-do-i-determine-the-size-of-my-array-in-c
Another advantage is that you can now easily parameterize the array name in a macro and get

#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))
we could have done sizeof(a)/sizeof(int) but if the variable isn't of type int we would have a nasty bug
0 means the first element, and by referring to 'a' we automatically change the type

int a[17];
size_t n = NELEMS(a);
*/

int main(int argc, char *argv[])
{
  // In Visual Studio Code, to format code: SHIFT + ALT + F

  // to compile use gcc filepath.c
  // also the C++ compiler works using g++

  // printf and fprintf
  /*

  int printf(const char *format, ...)
    sends data in the stdout stream using specified format
  int fprintf(FILE *stream, const char *format, ...)
    sends data in the specified stream using specified format

  These are standard functions of the C language, defined in stdio.h (also cstdio, see difference between directives header and library https://stackoverflow.com/questions/10460250/cstdio-stdio-h-namespace)
  There is no print(), if there is it means it's a custom function
  #include <lib> copia il contenuto del file lib (cercando nelle cartelle delle librerie) nel file corrente
  #include "lib" come sopra ma cerca prima nella cartella corrente
  e ad altre direttive come #define etc.
  */

  printf("This is an integer: %d\n", 4);
  int pippo = 10;
  printf("This is the integer variable pippo: %d\n", pippo);
  printf("This is a string: %s\n", "ciao");

  char *pluto = "pluto"; // read-only string
  // pluto[2] = 'a'; Segmentation fault! it is read-only
  printf("This is the string variable pluto: %s\n\tit is a read-only string and is defined char * pluto = \"pluto\";\n", pluto);
  char string[] = "string[]=\"ciao\";"; // writable string in the stack
  printf("This is the string variable string[]: %s\n", string);
  char stringDue[] = {'c', 'i', 'a', 'o'};
  string[2] = 'a';
  printf("This is the string variable string[] after new assignment: %s\n\tthe new assignment is string[2] = \'a\'\n", string);

  // arguments

  if (argc > 1)
  {
    printf("Hey you gave me some arguments!\n\tIn total they are: %d, one more because the program itself is counted!\n\tThese are: ", argc);
    for (int i = 0; i < argc; i++)
    {
      printf("(#%d %s) ", i, argv[i]);
      if (i == argc - 1)
      {
        printf("\n");
      }
    }
  }
  else
  {
    printf("You can also try to pass me some arguments\n");
  }

  // string.h

  // char * strncat(char *dest, const char *src,size_t n)
  char *paperino = "paperino";
  char copia[30];                                                                     // lunghezza 50 byte, 1 byte a carattere da definizione
  printf("This is the string variable copia[30]: %s\n\tIt should be empty\n", copia); // empty
  strncat(copia, paperino, 3);
  printf("After strncat() of length 3, this is the string variable copia[30]: %s\n", copia);
  strcat(copia, paperino);
  printf("After strcat(), this is the string variable copia[30]: %s\n", copia);
  strcpy(copia, paperino);
  printf("After strcpy(), this is the string variable copia[30]: %s\n", copia);
  // char * strchr(const char *str, int c)
  printf("Now we try to look for the character '.' in the string, using strchr()\n");
  char *ret = strchr(paperino, '.');
  printf("The string after |%c| is |%s|\n\tIt should be null since there is no '.'\n", '.', ret);
  printf("Now we try to look for the character 'n' in the string, using strchr()\n");
  ret = strchr(paperino, 'e');
  printf("The string after |%c| is |%s|\n\tIt should be 'erino'\n", 'n', ret);
  // int strcmp(const char *str1, const char *str2)
  char *compare = "paperino";
  int cmpRes = strcmp(paperino, compare);
  printf("Let's compare the string paperino and the string compare, the result is %d\n\tIt should be 0 if they are the same\n", cmpRes);
  // and many more

  // malloc and free
  // void * malloc(size_t size)
  // void free(void * pnt)

  return 0;
}