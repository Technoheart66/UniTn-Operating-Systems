// signal_groups.c

// Section 0: copyright & licensing
/*
  Author: Francesco Dall'Agata
*/

// Section 1: index
// 0) copyright & licensing
// 1) index
// 2) assignment
// 3) include
// 4) define
// 5) typedef
// 6) global variables
// 6) external declaration
// 7) function declaration
// 8) main
// 9) function definition

// Section 2: assignment
/*
Nel prossimo esempio:
1. Processo ‘ancestor’ crea un figlio
  a. Il figlio cambia il proprio gruppo e genera 3 figli (4 proc. nel Gruppo1)
  b. I 4 processi aspettano fino all’arrivo di un segnale
2. Processo ‘ancestor’ crea un secondo figlio
  a. Il figlio cambia il proprio gruppo e genera 3 figli (4 proc. nel Gruppo2)
  b. I 4 processi aspettano fino all’arrivo di un segnale
3. Processo ‘ancestor’ manda due segnali diversi ai due gruppi
*/

// Section 3: include
#include <stdio.h>   // printf()
#include <stdlib.h>  // EXIT_SUCCESS
#include <unistd.h>  // getpid(), getppid(), getpgid() etc.
#include <stdbool.h> // true, false, bool
#include <signal.h>  // signals, struct sigaction sa

// Section 4: define
#define STR_LENGTH (100)

// Section 6: global variables
volatile sig_atomic_t ancestor_pid; // volatile, see: https://stackoverflow.com/questions/246127/why-is-volatile-needed-in-c
                                    // see also https://www.html.it/pag/72236/la-parola-chiave-volatile/
                                    // sig_atomic_t is a data type defined in <signal.h> that is guaranteed to be read and written atomically. This means that operations on variables of this type are not interrupted by signals, making them safe to use in signal handlers
                                    // see: https://www.gnu.org/software/libc/manual/html_node/Atomic-Types.html
                                    // see also: https://stackoverflow.com/questions/24931456/how-does-sig-atomic-t-actually-work

// Section 7: function declaration
// print process status in relation to process hierarchy, PID, PPID and GPID
void print_process(volatile pid_t *ancestor);

// print process info and received signal
void handler(int signo, siginfo_t *info, void *context);

// Section 4: main
int main(int argc, char *argv[])
{
  ancestor_pid = getpid();
  int first_child = 0;
  int second_child = 0;

  struct sigaction sa;
  sa.sa_sigaction = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGUSR1, &sa, NULL);
  sigaction(SIGUSR2, &sa, NULL);

  print_process(&ancestor_pid);
  first_child = fork();
  if (first_child == 0)
  {
    // child leader 1
    first_child = getpid();
    // change group and become group leader 1
    setpgid(0, 0);
    print_process(&ancestor_pid);
    for (int i = 0; i < 3 && getpid() == first_child; i++)
    {
      int new_child = fork();
      if (new_child == 0)
      {
        print_process(&ancestor_pid);
      }
    }
  }
  else
  {
    // ancestor
    second_child = fork();
    if (second_child == 0)
    {
      // child leader 2
      second_child = getpid();
      // change group and become group leader 2
      setpgid(0, 0);
      print_process(&ancestor_pid);
      for (int i = 0; i < 3 && getpid() == second_child; i++)
      {
        int new_child = fork();
        if (new_child == 0)
        {
          print_process(&ancestor_pid);
        }
      }
    }
    else
    {
      // ancestor
      if (getpid() == ancestor_pid) // to be sure it is the ancestor
      {
        kill(-first_child, SIGUSR1);  // Send SIGUSR1 to group 1
        kill(-second_child, SIGUSR2); // Send SIGUSR2 to group 2
      }
    }
  }
  printf("banana");

  do
  {
    // wait indefinitely to prevent processes to terminate and be reassigned
  } while (true);
  return EXIT_SUCCESS;
}

void print_process(volatile pid_t *ancestor)
{
  char status[STR_LENGTH] = "Status: ?";
  if (getpid() == *ancestor)
  {
    snprintf(status, STR_LENGTH, "Status: Ancestor\nPID: %d, PPID: %d, GID: %d\n", getpid(), getppid(), getpgid(0));
  }
  else if (getpid() == getpgid(0))
  {
    snprintf(status, STR_LENGTH, "Status: Leader\nPID: %d, PPID: %d, GID: %d\n", getpid(), getppid(), getpgid(0));
  }
  else
  {
    snprintf(status, STR_LENGTH, "Status: Child\nPID: %d, PPID: %d, GID: %d\n", getpid(), getppid(), getpgid(0));
  }
  printf("%s", status);
}

void handler(int signo, siginfo_t *info, void *context)
{
  print_process(&ancestor_pid);
  printf("Received signal: %d\n", signo);
}