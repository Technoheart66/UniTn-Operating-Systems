// Fork foundamentals

#include <stdio.h>  //printf
#include <time.h>
#include <unistd.h> //fork, pipes and I/O

int main(int arcg, char *argv[])
{
  // Let's try to generate a child process using fork()
  int res = fork();
  printf("This is after the fork, I am %d\n", res);
  int pid = getpid();
  int ppid = getppid();
  if (!res)
  {
    printf("res=%d: I'm the parent, my PID is '%d' and my PPID is '%d'\n", res, pid, ppid);
  }
  else
  {
    sleep(1);
    printf("res=%d: I'm the child, I slept 1 second, my PID is '%d' and my PPID is '%d'\n", res, pid, ppid);
  }

  return 0;
}