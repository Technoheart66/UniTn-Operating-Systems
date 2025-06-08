#include <stdio.h>
#include <errno.h>
#include <string.h>

int main()
{
  FILE *f = fopen("doesnotexist.txt", "r");
  if (f == NULL)
  {
    printf("Error code: %d\n", errno);              // Numeric errno value
    printf("Error message: %s\n", strerror(errno)); // Human-readable message
    perror("CIAOOOO");
  }
  return 0;
}
