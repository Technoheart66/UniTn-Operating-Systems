// gnu_source.c

#include <stdio.h>
#include <signal.h>
//#define _GNU_SOURCE // or _POSIX_C_SOURCE 200809L

int main()
{
#ifdef _GNU_SOURCE
  printf("_GNU_SOURCE is defined\n");
#else
  printf("_GNU_SOURCE is NOT defined\n");
#endif

#ifdef _POSIX_C_SOURCE
  printf("_POSIX_C_SOURCE = %ld\n", _POSIX_C_SOURCE);
#else
  printf("_POSIX_C_SOURCE is NOT defined\n");
#endif

  return 0;
}