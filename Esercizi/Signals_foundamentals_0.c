// Signals_foundamentals_0.c

// Useful C tips on signal handling

/*
Source: https://faculty.cs.niu.edu/~hutchins/csci480/signals.htm

# Signal      Default     Comment                              POSIX
  Name        Action

 1 SIGHUP     Terminate   Hang up controlling terminal or      Yes
                          process
 2 SIGINT     Terminate   Interrupt from keyboard, Control-C   Yes
 3 SIGQUIT    Dump        Quit from keyboard, Control-\        Yes
 4 SIGILL     Dump        Illegal instruction                  Yes
 5 SIGTRAP    Dump        Breakpoint for debugging             No
 6 SIGABRT    Dump        Abnormal termination                 Yes
 6 SIGIOT     Dump        Equivalent to SIGABRT                No
 7 SIGBUS     Dump        Bus error                            No
 8 SIGFPE     Dump        Floating-point exception             Yes
 9 SIGKILL    Terminate   Forced-process termination           Yes
10 SIGUSR1    Terminate   Available to processes               Yes
11 SIGSEGV    Dump        Invalid memory reference             Yes
12 SIGUSR2    Terminate   Available to processes               Yes
13 SIGPIPE    Terminate   Write to pipe with no readers        Yes
14 SIGALRM    Terminate   Real-timer clock                     Yes
15 SIGTERM    Terminate   Process termination                  Yes
16 SIGSTKFLT  Terminate   Coprocessor stack error              No
17 SIGCHLD    Ignore      Child process stopped or terminated  Yes
                          or got a signal if traced
18 SIGCONT    Continue    Resume execution, if stopped         Yes
19 SIGSTOP    Stop        Stop process execution, Ctrl-Z       Yes
20 SIGTSTP    Stop        Stop process issued from tty         Yes
21 SIGTTIN    Stop        Background process requires input    Yes
22 SIGTTOU    Stop        Background process requires output   Yes
23 SIGURG     Ignore      Urgent condition on socket           No
24 SIGXCPU    Dump        CPU time limit exceeded              No
25 SIGXFSZ    Dump        File size limit exceeded             No
26 SIGVTALRM  Terminate   Virtual timer clock                  No
27 SIGPROF    Terminate   Profile timer clock                  No
28 SIGWINCH   Ignore      Window resizing                      No
29 SIGIO      Terminate   I/O now possible                     No
29 SIGPOLL    Terminate   Equivalent to SIGIO                  No
30 SIGPWR     Terminate   Power supply failure                 No
31 SIGSYS     Dump        Bad system call                      No
31 SIGUNUSED  Dump        Equivalent to SIGSYS                 No
*/

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