// Signals_foundamentals_1.c

// Librerie essenziali
#include <stdio.h>  // printf
#include <signal.h> // signals
#include <stdlib.h> // atoi, rand, srand, malloc, free, exit, EXIT_SUCCESS, EXIT_FAILURE, size_t etc.
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h> // system calls for the time and random number generator

void handler(int signo)
{
  printf("signal %d received\n", signo);
  sleep(2);
  printf("Signal done\n");
}

int main(void)
{
  printf("This process has PID [%d]", getpid());
  struct sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;          // Initialise flags
  sigemptyset(&sa.sa_mask); // using 'sigemptyset' let's use an empty mask to block no signal
  sigaction(SIGUSR1, &sa, NULL);
  while (1)
    ;
}
