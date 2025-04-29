// Signals_foundamentals_0.c

#include <stdio.h>  //printf
#include <signal.h> //signals
#include <stdlib.h> //atoi, rand, srand, malloc, free, exit, EXIT_SUCCESS, EXIT_FAILURE, size_t etc.

// A custom handler must be a function of type void that has an int as an argument to accept a signal
void custom_handler(int sig)
{
  if (sig == SIGINT)
  {
    printf("CTRL+C\n");
  }
  else if (sig == SIGTSTP) // slight difference between SIGSTP and SIGSTOP https://stackoverflow.com/questions/11886812/what-is-the-difference-between-sigstop-and-sigtstp
  {
    printf("CTRL+Z\n"); // since SIGSTP, see link above
    exit(2);            // let's interrupt the process since it's CTRL+Z
  }
}

int main(int argc, char *argv[])
{
  // DEPRECATED, we need to use sigaction()

  // sighandler_t signal(int signum, sighandler_t handler);

  // Imposta un nuovo signal handler handler per il segnale signum.
  // Restituisce il signal handler precedente. Quello nuovo può essere:
  // ● SIG_DFL: handler di default
  // ● SIG_IGN: ignora il segnale
  // ● typedef void (*sighandler_t)(int): custom handler.

  signal(SIGCHLD, SIG_DFL); // Use default handler

  // signal(SIGINT, SIG_IGN);  // Ignore signal
  // Instead of the above line which ignores CTRL+C let's print it
  signal(SIGINT, custom_handler);
  // Instead of immediately interrupting the process with CTRL+Z let's print it and do some stuff
  signal(SIGTSTP, custom_handler);
  while (1)
    ;

  return 0;
}