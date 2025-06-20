// main_19062025.c

// Section 0: author & introduction
// Author: Francesco Dall'Agata
// Introduction: exam of 11/06/2025 of Sistemi Operativi (Laboratorio) by PhD M. Grisafi

// Section 1: index
// 0) index
// 1) author & introduction
// 2) include
// 3) define
// 4) typedef
// 5) global variables
// 6) function declaration
// 7) main
// 8) function definition

// Section 2: include
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h> // to access errno
#include <stdbool.h>
#include <unistd.h>
#include <sys/msg.h>   // queue
#include <fcntl.h>     // fifo
#include <sys/types.h> // fifo
#include <sys/stat.h>  // fifo
#include <string.h>
#include <sys/wait.h> //waitpid()
#include <errno.h>
#include <pthread.h>
#include <stdint.h> // compiler/architecture independent fixed-size int values
#include <sys/ipc.h>

// Section 3: define
#define STR_DEFAULT 128

// Section 4: typedef
typedef struct struct_msg
{
    long mtype;              // will contain whale distance
    char mtext[STR_DEFAULT]; // will contain whale species
} QueueMsg;

// Section 5: global variables
pid_t timonierePID = 0;                        // stores input parameter <timonierePID>
unsigned short int n = 0;                      // stores input parameter <n>
volatile sig_atomic_t flag_exit = false;       // false = keep running, true = terminate
int private_queue = 0;                         // stores IPC_PRIVATE queue ID
volatile sig_atomic_t flag_balena = false;     // flog for vedetta SIGUSR2, false = nothing, true = balena
volatile sig_atomic_t flag_equipaggio = false; // flag for nave SIGRTMAX

// Section 6: function declaration
void vedetta();               // manages the tasks of process 'vedetta'
void arpioniere(pid_t group); // manages the tasks of each process 'arpioniere'
void handler_vedetta_avvistamento(int signo, siginfo_t *info, void *empty);
void handler_nave_SIGRTMAX(int signo, siginfo_t *info, void *empty);

// Section 7: main
int main(int argc, char *argv[])
{
    // process 'nave'
    // request #1, check if parameters are present, <timonierePID> and <n>, if not exit with code 90
    unsigned short int exit_code = EXIT_SUCCESS; // assume success by default
    if (argc != 3)                               // if there are exactly two parameters in addition to the program itself
    {
        fprintf(stderr, "ERROR: missing parameters\nUSAGE: %s <timonierePID> <n>\n", argv[0]);
        exit_code = 90; // request #1, a different count of parameters shall terminate the program with code 90
        return exit_code;
    }

    // assign parameters to variables for readability
    char *end_ptr;                                // stores termination character '\0' for strtol error checking
    timonierePID = strtol(argv[1], &end_ptr, 10); // stores the input parameter <timonierePID>
    n = strtol(argv[2], &end_ptr, 10);            // sotres the input parameter <n>

    // check if parameter <timonierePID> is correct
    if (kill(timonierePID, 0) != 0) // with signal = 0  no signal is actually sent but error checking is performed
    {
        fprintf(stderr, "ERROR: invalid parameter <timonierePID> = %d, it must be an existing PID\nUSAGE: %s <timonierePID> <n>\n", timonierePID, argv[0]);
        exit_code = 91; // custom exit code
        return exit_code;
    }

    // check if parameter <n> is correct
    if (n <= 0) // n must be at least 1
    {
        fprintf(stderr, "ERROR: invalid parameter <n> = %d, it must be greater than 0\nUSAGE: %s <timonierePID> <n>\n", n, argv[0]);
        exit_code = 92; // custom exit code
        return exit_code;
    }

    // request #7, creating a private queue so that process 'vedetta' and proceses 'arpionierei' can communicate
    private_queue = msgget(IPC_PRIVATE, IPC_CREAT | 0666);

    // request #2, process 'nave' must generate a new process 'vedetta'
    pid_t nave_pid, vedetta_pid;
    nave_pid = getpid();  // store the PID of process 'nave'
    vedetta_pid = fork(); // create a new process with fork(), 'nave' will receive the PID while the new process 'vedetta' will receive 0

    if ((vedetta_pid == 0) && (getpid() != nave_pid)) // if this is the new process
    {
        vedetta();
        exit(EXIT_SUCCESS); // extra layer of protection
    }

    // request #3, process 'nave' must create <n> child processes, they are 'arpionieri'
    bool guard_exit = false; // guard to exit for statement
    pid_t arpionieri_pid[n]; // used by 'nave' to store child processes information
    char folder[STR_DEFAULT] = "./tmp/registrazione/";
    char file_registrazione[STR_DEFAULT] = {0};
    for (int i = 0; i < n && !guard_exit; i++)
    {
        arpionieri_pid[i] = fork();
        if (arpionieri_pid[i] == 0) // if it is the child process
        {
            guard_exit = true; // set guard to exit the cycle
            if (i == 0)        // if this is the first process let's make it leader
            {
                arpioniere(0); // call function for process 'arpioniere' with 0 to make it captain
            }
            else
            {
                arpioniere(arpionieri_pid[0]); // call function for process 'arpioniere' to join group of captain
            }

            return EXIT_SUCCESS; // extra layer of safeness
        }
    }

    // request #9, mobydick will begin the attack an dkill some harpooners, the attack it's over when a signal SIGRTMAX arrives
    struct sigaction sa_nave_check_arpionieri;
    sigemptyset(&sa_nave_check_arpionieri.sa_mask);
    sigaddset(&sa_nave_check_arpionieri.sa_mask, SIGRTMAX);
    sa_nave_check_arpionieri.sa_flags = SA_SIGINFO;
    sa_nave_check_arpionieri.sa_sigaction = handler_nave_SIGRTMAX;
    sigaction(SIGRTMAX, &sa_nave_check_arpionieri, NULL);

    while (!flag_exit)
    {
        pause();
        if (flag_equipaggio)
        {
            flag_equipaggio = false;
            int status = 0;
            int killed_by_mobydick = 0;
            pid_t terminating_pid = 0;
            while ((terminating_pid = waitpid(-arpionieri_pid[0], &status, WNOHANG)) != -1) /// check all childs with group of the first hapooner is it is alive with WNOHANG we go on if no child changed state
            {
                killed_by_mobydick++;
            }
            union sigval payload;
            payload.sival_int = n - killed_by_mobydick;
            sigqueue(timonierePID, SIGRTMIN, payload);
        }
    }

    return exit_code;
}

// Section 8: function definition
void vedetta() // manages the tasks of process 'vedetta'
{
    // request #2, when created process 'vedetta' must send SIGUSR1 to <timonierePID>
    kill(timonierePID, SIGUSR1);

    // request #5, upon seeing a whale (balena) via SIGUSR2, process 'vedetta' must print on stdout
    struct sigaction sa_vedetta_avvistamento;
    sigemptyset(&sa_vedetta_avvistamento.sa_mask);        // ensure the signal mask is empty
    sigaddset(&sa_vedetta_avvistamento.sa_mask, SIGUSR2); // add SIGUSR2 to the mask so that the same signal can't interrupt the current execution of the handler
    sa_vedetta_avvistamento.sa_flags = SA_SIGINFO;
    sa_vedetta_avvistamento.sa_sigaction = handler_vedetta_avvistamento;
    sigaction(SIGUSR2, &sa_vedetta_avvistamento, NULL);

    // request #6, process 'vedetta' must read on a queue the type and distance of the whale
    key_t queue_key = ftok("./tmp/mare", timonierePID);
    int queue_mare = msgget(queue_key, IPC_CREAT); // msgflg = 0 means to obtain the exiting queue id otherwise do not create it, in this case let's create since it is not already present when testing

    while (!flag_exit)
    {
        pause();
        if (flag_balena)
        {
            flag_balena = false;                                       // immediately reset flag
            QueueMsg msg_rcv;                                          // stores the message content
            msgrcv(queue_mare, &msg_rcv, sizeof(msg_rcv.mtext), 0, 0); // reads the first message of any type in the queue mare
            printf("Balena!\n");                                       // request #5, print in stdout

            // request #8, process 'vedetta' must understand if the whale is moby dick and invoke captain Ahab
            if (strcmp("mobydick", msg_rcv.mtext) == 0) // if it is exactly mobydick
            {
                // call Ahab, since we are using execl we need to create a new process
                pid_t call_ahab = fork();
                if (call_ahab == 0) // if it is the newly created process
                {
                    char convert_pid[16];
                    snprintf(convert_pid, sizeof(convert_pid), "%d", getpid());
                    execl("./ahab.out", "ahab.out", convert_pid, NULL); // substitute current process image with new image
                }
            }

            // request #7, vedetta must communicate whale type and distance to the crew
            for (int i = 0; i < n; i++) // we need to send exactly <n> messages
            {
                QueueMsg msg_snd;
                snprintf(msg_snd.mtext, sizeof(msg_snd.mtext), "%s", msg_rcv.mtext);
                msg_snd.mtype = msg_rcv.mtype;
                msgsnd(private_queue, &msg_snd, sizeof(msg_snd), 0); // send a message for each haarponer to read
            }
        }
    }

    exit(EXIT_SUCCESS);
}

void arpioniere(pid_t group) // manages the tasks of process 'arpioniere'
{
    // request #3, each harpooner must register itself by creating a file in /tmp/registrazione/<PID>
    char filename[STR_DEFAULT] = {0};
    snprintf(filename, sizeof(filename), "%s%d", "./tmp/registrazione/", getpid());
    FILE *file = fopen(filename, "w");
    fclose(file);

    // request #4, all harpooners must be part of the same squad and there is always a captain harpooner
    if (group == 0) // if group is 0 the process will become captain
    {
        setpgid(0, 0); // the process itself uses it's own PID and becomes part of the process's own PID group, thus becoming group leader
    }
    else
    {
        setpgid(0, group); // the process itself uses it's own PID and joins the specified group passed to the function
    }

    while (!flag_exit) // until we choose to exit
    {
        // request #7, each arpioniere must wait for a message sent by process 'vedetta' and write in a FIFO
        QueueMsg msg;
        msgrcv(private_queue, &msg, sizeof(msg.mtext), 0, 0); // 0 = just read a message of any type, 0 = no flag, default behaviour

        char fifo_name[STR_DEFAULT] = {0};
        snprintf(fifo_name, sizeof(fifo_name), "./tmp/%d-%ld", getpid(), msg.mtype);
        mkfifo(fifo_name, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IWOTH | S_IROTH);
        int fd = open(fifo_name, O_CREAT | O_WRONLY, 0666);
        write(fd, msg.mtext, strlen(msg.mtext));
        close(fd);
    }

    exit(EXIT_SUCCESS);
}

void handler_vedetta_avvistamento(int signo, siginfo_t *info, void *empty)
{
    flag_balena = true; // set flag to true
}

void handler_nave_SIGRTMAX(int signo, siginfo_t *info, void *empty)
{
    flag_equipaggio = true; // set fral to true
}
