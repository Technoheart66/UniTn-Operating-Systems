//sigaction3.c
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void handler(int signo)
{
  printf("signal %d received\n", signo);
  sleep(2);
  printf("Signal done\n");
}
int main()
{
  printf("Process id: %d\n", getpid());
  struct sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0; // Initialise flags
  sigemptyset(&sa.sa_mask);
  sigaddset(&sa.sa_mask, SIGUSR2); // Block SIGUSR2 in handler
  sigaction(SIGUSR1, &sa, NULL);   // we defined a custom action just for SIGUSR1
  // SIGUSR2 is free! So if we send a kill with -12 (which corresponds to SIGUSR2) the signal will be blocked ...
  // ... and the default action is perfomed since we did not specify a custom handler!
  while (1)
    ;
}