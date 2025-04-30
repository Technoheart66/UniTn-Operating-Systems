// sigaction4.c
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void handler(int signo, siginfo_t *info, void *empty)
{
  // print id of process issuing the signal
  printf("Signal received from %d\n", info->si_pid);
}
int main()
{
  printf("This process has PID: %d\n", getpid());
  struct sigaction sa;
  sa.sa_sigaction = handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO;    // Use sa_sigaction
  sa.sa_flags |= SA_RESETHAND; // Restore the signal action to the default upon entry to the signal handler. This flag is meaningful only when establishing a signal handler
  sigaction(SIGUSR1, &sa, NULL);
  while (1)
    ;
}
