// process_groups.c

// Section 0: index
// 0) index
// 1) include

// Section 1: include
#include <stdlib.h> // NULL, atoi(), rand(), srand(), malloc(), free(), exit(), EXIT_SUCCESS, EXIT_FAILURE, size_t etc.
#include <stdio.h>  // printf()
#include <unistd.h> // getpid(), getppid()

int main(int argc, char *argv[])
{
  int child = fork();
  // get the process PID
  // pid_t pid = getpid();

  // get the process GID (Group ID), do not confuse with getgid()! (gets the usergroup ID)
  // pid_t gid = getpgid(0);
  switch (child)
  {
  case 0: // child, 0
    printf("I am the child\tPID: %d, PPID: %d, GID: %d\n", getpid(), getppid(), getpgid(0));
    printf("I will now become the group leader\n");
    setpgid(0, 0); // become group leader, same as setpgid(0, getpid())
    printf("I am the child\tPID: %d, PPID: %d, GID: %d\n", getpid(), getppid(), getpgid(0));
    break;

  default: // parent, PID of the child
    printf("I am the parent\tPID: %d, PPID: %d, GID: %d\n", getpid(), getppid(), getpgid(0));
    break;
  }
  return EXIT_SUCCESS;
}