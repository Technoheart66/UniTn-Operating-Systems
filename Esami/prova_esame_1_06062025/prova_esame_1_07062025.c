// prova_esame_1_07062025.c

// Section 0: author & introduction
// Author: Francesco Dall'Agata UniTN 221451
// Introduction: prova d'esame caricata su moodle "Prova esame 1",
//               tentativo del 07/06/2025, makefile del 06/06/2025

// Section 1: index
// 0) author & introduction
// 1) index
// 2) include
// 3) define
// 4) typedef
// 5) global variables
// 6) function declaration
// 7) main
// 8) function definition

// Section 2: include
#include <stdlib.h>  // EXIT_SUCCESS, EXIT_FAILURE etc.
#include <stdio.h>   // printf(), fprintf(), fopen() etc.
#include <string.h>  // strcmp(), strlen() etc.
#include <signal.h>  // kill(), sigaction(), sigprocmask() etc.
#include <sys/msg.h> // QUEUE -> msgget()
#include <fcntl.h>   // FILE DESCRIPTORS -> fcntl()
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>

// Section 3: define
#define STR_INPUT_BUFFER 256
#define STR_COMMAND 64
#define STR_OPTION 64

// Section 4: typedef
typedef struct msg_buffer_struct // new type for queue messages
{
  long mtype;
  char mtext[STR_OPTION];
} MsgQueue;

typedef struct param_buffer_thread_struct // new type for passing arguments to a threaded function
{
  char command[STR_COMMAND];
  char option_one[STR_OPTION];
  char option_two[STR_OPTION];
} ParamsThread;

// Section 5: global variables
volatile sig_atomic_t flag_SIGUSR1 = 0;

// Section 6: function declaration
void handler_SIGUSR1(int signo, siginfo_t *info, void *empty);
void *thread_kill(void *param);  // threaded; send a kill signal
void *thread_queue(void *param); // threaded; send a message in the queue specified in the file
void *thread_fifo(void *param);  // threaded; send a message in the FIFO specified in the file

// Section 7: main
int main(int argc, char *argv[])
{
  unsigned short int exit_value = EXIT_SUCCESS;

  // check if there is 1 and only 1 parameter
  if (argc != 2) // two since program + param = 2
  {
    fprintf(stderr, "%s%s", "ERROR: wrong parameters\n", "USAGE: ./a.out <∕path∕to∕file.txt>\n");
    exit_value = 11; // custom value, 11 = wrong parameters
    return exit_value;
  }

  // open the file or return error
  FILE *input_file;
  input_file = fopen(argv[1], "r"); // [0] program, [1] param, [2] param etc.
  if (input_file == NULL)
  {
    fprintf(stderr, "%s%s", "ERROR: couldn't open input file\n", "USAGE: ./a.out <path/to/file.txt\n");
    exit_value = 12; // custom value, 12 = wronf filepath
    return exit_value;
  }

  // Acknowledgment: handler
  struct sigaction sigaction_SIGUSR1, old_sa;
  sigaction_SIGUSR1.sa_handler = handler_SIGUSR1;
  sigemptyset(&sigaction_SIGUSR1.sa_mask);
  sigaddset(&sigaction_SIGUSR1.sa_mask, SIGUSR1); // while inside handler block SIGUSR1 so that the handler can't be interrupted
  sigaction(SIGUSR1, &sigaction_SIGUSR1, &old_sa);

  // Acknowledgment: block signals
  sigset_t block, old_block;
  sigemptyset(&block);
  sigfillset(&block);
  sigdelset(&block, SIGUSR1);
  sigprocmask(SIG_BLOCK, &block, &old_block);

  // Acknowledgment: sigsuspend -> done after first command

  // create queue
  key_t queue_key = ftok(argv[1], 1);
  unsigned short int queue_id = msgget(queue_key, IPC_CREAT);

  // read the file until it is empty
  char buffer_from_file[STR_INPUT_BUFFER];
  unsigned short int counter = 0;
  while (!feof(input_file))
  {
    counter++;
    // getline(); // not part of C standard, it is a POSIX extension, it can automatically allocate memory
    fgets(buffer_from_file, sizeof(buffer_from_file), input_file); // read one line
    char command[STR_COMMAND];                                     // store the command
    char option_one[STR_OPTION];                                   // store the first command argument
    char option_two[STR_OPTION];                                   // store the second command argument
    int matches = sscanf(buffer_from_file, "%s %s %s", command, option_one, option_two);
    switch (matches)
    {
    case 0:
      fprintf(stderr, "%d) ERROR: couldn't parse any command\n", counter);
      break;

    case 1:
      fprintf(stderr, "%d) ERROR: couldn't parse command arguments '%s <arg1> <arg2>'\n", counter, command);
      break;

    case 2:
      fprintf(stderr, "%d) ERROR: couldn't parse last command argument '%s %s <arg2>'\n", counter, command, option_one);
      break;

    case 3:                             // correct parsing, all 3 variables are set
      if (strcmp("kill", command) == 0) // if the command is kill
      {
        // kill <signo> <pid>
        // printf("%d) kill %s %s\n", counter, option_one, option_two);
        // kill(atoi(option_one), atoi(option_two));

        // threaded version
        pthread_t thread;
        ParamsThread *parametri = malloc(sizeof(ParamsThread));
        snprintf(parametri->command, sizeof(parametri->command), "%s", command);
        snprintf(parametri->option_one, sizeof(parametri->option_one), "%s", option_one);
        snprintf(parametri->option_two, sizeof(parametri->option_two), "%s", option_two);
        pthread_create(&thread, NULL, thread_kill, (void *)parametri); // inside thread
      }
      else if (strcmp("queue", command) == 0) // if the command is queue
      {
        // queue <category> <word>
        printf("%d) queue %s %s\n", counter, option_one, option_two);
        MsgQueue messaggio_coda;
        messaggio_coda.mtype = atoi(option_one);
        snprintf(messaggio_coda.mtext, sizeof(messaggio_coda.mtext), "%s", option_two);
        msgsnd(queue_id, &messaggio_coda, sizeof(messaggio_coda.mtext), 0); // 0 = no flags, default behavior
      }
      else if (strcmp("fifo", command) == 0) // if the command is fifo
      {
        // fifo <name> <word>
        printf("%d) fifo %s %s\n", counter, option_one, option_two);
        // create FIFO if it doesn't exist
        mkfifo(option_one, IPC_CREAT);
        open(option_one, O_RDWR);
        if (write(option_one, sizeof(option_two), option_two) == -1)
        {
          fprintf(stderr, "ERROR: write error, errno = %d", errno);
        }
      }
      else
      {
        fprintf(stderr, "%d) ERROR: unspecified command or not implemented '%s %s %s'\n", counter, command, option_one, option_two);
      }
      break;

    default: // less than 0 or more than 3 yields a generic error
      fprintf(stderr, "%d) ERROR: parsing error '%s'\n", counter, buffer_from_file);
      break;
    }

    // Acknowledgment: sigsuspend -> done after first command
    sigset_t wait_SIGUSR1;
    sigemptyset(&wait_SIGUSR1);        // ensure set is empty
    sigfillset(&wait_SIGUSR1);         // ensure set is completely full
    sigdelset(&wait_SIGUSR1, SIGUSR1); // only SIGUSR1 should pass
    while (!flag_SIGUSR1)              // if flag is not set keep waiting
    {
      sigsuspend(&wait_SIGUSR1); // wait for any signal to arrive but first change the mask
    }
  }

  return exit_value;
}

void handler_SIGUSR1(int signo, siginfo_t *info, void *empty)
{
  printf("Inside handler_SIGUSR1\n");
}

void *thread_kill(void *param) // threaded; send a kill signal
{
  ParamsThread *args = (ParamsThread *)param;
  kill(atoi(args->option_one), atoi(args->option_two));
  return NULL; // end thread
}