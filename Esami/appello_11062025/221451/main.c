// esame.c

// Section 0: author & introduction
// Author: Francesco Dall'Agata 221451
// Introduction: esame di Sistemi Operativi Laboratorio del 11/06/2025

// Section 1:index
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>

// Section 3: define
#define STR_DEFAULT 64

// Section 4: typedef
typedef struct struct_queue_msg
{
    long mtype;
    char mtext[51];
} QueueMsg;

// Section 5: global variables
volatile sig_atomic_t flag_exit = false;
__sig_atomic_t nave_pid = 0;    // PID nave
__sig_atomic_t vedetta_pid = 0; // PID vedetta
int timonierePID = 0;           // to pass timonierePID inside handlers in case
volatile sig_atomic_t flag_SIGUSR2 = false;
volatile sig_atomic_t flag_SIGRTMAX = false;
volatile sig_atomic_t flag_SIGRTMIN = false;
volatile sig_atomic_t flag_CAPITANO;

// Section 6: function declaration
void handler_vedetta(int signo, siginfo_t *info, void *empty);
void handler_nave(int signo, siginfo_t *info, void *empty);
void handler_timoniere(int signo, siginfo_t *info, void *empty);
void handler_capitano(int signo, siginfo_t *info, void *empty);

// Section 7: main

int main(int argc, char *argv[])
{
    unsigned short int exit_value = EXIT_SUCCESS;

    // request #1, check parameters
    if (argc != 3)
    {
        fprintf(stderr, "ERROR: invalid arguments\nUSAGE: ./a.out <timonierePID> <n>\n");
        exit_value = 90; // codice custom as specified = 90
        exit(exit_value);
    }

    timonierePID = atoi(argv[1]);
    int n = atoi(argv[2]);

    // request #1, check if <timonierePID> exists
    if (kill(timonierePID, 0) != 0) // on failure kill returns -1, on success 0
    {
        fprintf(stderr, "ERROR: passed argument <timonierePID> = {%d}, the specified PID doesn't exist\n", timonierePID);
        exit_value = 91; // codice custom, invalid <timonierePID>
        exit(exit_value);
    }

    // request #1, check if <n> is a integer
    if (n <= 0) // n must be greater than 0
    {
        fprintf(stderr, "ERROR: passed argument <n> = {%d}, n must be an integer and greater than 0\n", n);
        exit_value = 92; // codice custom, invalid <n>
        exit(exit_value);
    }

    // printf("RIMUOVERE timonierePID = %d\n", timonierePID);
    // printf("RIMUOVERE n: %d\n", n);

    // set nave (origin) PID
    nave_pid = getpid();
    printf("NAVE %d, GRUPPO %d\n", getpid(), getpgid(0));

    // request #2, generate process vedetta and send SIGUSR1 to <timonierePID>
    vedetta_pid = fork();
    if (vedetta_pid == 0 && getpid() != nave_pid) // if fork() returned 0 it means we are the child process AND if the PID is different than nave then it means the process is vedetta
    {
        // printf("RIMUOVERE Sono la vedetta, mando SIGUSR1 a timonierePID {%d}\n", nave_pid);
        kill(timonierePID, SIGUSR1);
        printf("VEDETTA %d, GRUPPO %d\n", getpid(), getpgid(0));
    }

    // request #3, nave generates <n> processes called arpionieri. Each arpioniere creates a new file named with its PID
    pid_t childs_arpionieri[n];            // nave will store PID of generated arpionieri
    FILE *registrazione_arpioniere = NULL; // each arpionere will store here the file stream
    pid_t arpioniere_capitano = fork();

    // request #4, there will be a capitano arpioniere
    if (arpioniere_capitano == 0 && getppid() == nave_pid)
    {
        setpgid(0, 0); // become group leader
        printf("CAPITANO %d, GRUPPO %d\n", getpid(), getpgid(0));
    }

    for (int i = 0; (i < n) && (getpid() == nave_pid); i++)
    {
        pid_t arpioniere_pid = fork();

        if (getpid() == nave_pid) // if this is nave
        {
            childs_arpionieri[i] = arpioniere_pid; // store in array
        }

        if (arpioniere_pid == 0 && getpid() != nave_pid) // if my PID is not nave and fork() returned 0 to me then I am an arpioniere
        {
            char filename[STR_DEFAULT] = {0};
            snprintf(filename, sizeof(filename), "/tmp/registrazione/%d", getpid());
            // printf("RIMUOVERE i=%d {%d} creating file %s\n", i, getpid(), filename);
            registrazione_arpioniere = fopen(filename, "w"); // creates file if it doesn't exist. Open for wiriting and reading
            setpgid(0, arpioniere_capitano);                 // join the group of arpioniere capitano
            printf("ARPIONIERE %d, GRUPPO %d\n", getpid(), getpgid(0));
        }
    }

    // Inizia la caccia!
    // request #6
    // int queue_segnala_balena_id = msgget(IPC_PRIVATE, IPC_CREAT); // create hidden queue to segnalare balena

    if (arpioniere_capitano == 0 && getppid() == nave_pid)
    {
        printf("dentro cap\n");
        struct sigaction sa_capitano;
        sigemptyset(&sa_capitano.sa_mask);        // empty the mask
        sigaddset(&sa_capitano.sa_mask, SIGUSR2); // while handling the signal block more signals SIGUSR2 and later it will unblock
        sa_capitano.sa_flags = SA_SIGINFO;
        sa_capitano.sa_sigaction = handler_capitano;
        sigaction(SIGUSR2, &sa_capitano, NULL);

        while (!flag_exit)
        {
            if (flag_CAPITANO)
            {
                flag_CAPITANO = false; // reset flag
                killpg(arpioniere_capitano, SIGUSR2);
            }
        }
    }

    if (vedetta_pid == 0 && getppid() == nave_pid) // if this is the process vedetta
    {
        printf("dentro vedetta\n");
        // request #5
        struct sigaction sa_vedetta;
        sigemptyset(&sa_vedetta.sa_mask);        // empty the mask
        sigaddset(&sa_vedetta.sa_mask, SIGUSR2); // while handling the signal block more signals SIGUSR2 and later it will unblock
        sa_vedetta.sa_flags = SA_SIGINFO;
        sa_vedetta.sa_sigaction = handler_vedetta;
        sigaction(SIGUSR2, &sa_vedetta, NULL);

        while (!flag_exit)
        {
            if (flag_SIGUSR2)
            {
                flag_SIGUSR2 = false; // reset flag
                printf("Balena!\n");  // request #5
                key_t queue_key = ftok("/tmp/mare", timonierePID);
                int queue_id = msgget(queue_key, 0); // la coda è già presente in valutazione

                QueueMsg messaggio_ricevuto;
                msgrcv(queue_id, &messaggio_ricevuto, sizeof(messaggio_ricevuto), 0, 0); // read first message

                // request #7 send a signal to capitano arpioniere every time wee see a whale
                union sigval segnale_avvistamento;
                char tipo_balena[STR_DEFAULT];
                snprintf(tipo_balena, sizeof(tipo_balena), "%s", messaggio_ricevuto.mtext);
                segnale_avvistamento.sival_int = messaggio_ricevuto.mtype;
                sigqueue(arpioniere_capitano, SIGUSR1, segnale_avvistamento);

                // request #8 moby dick
                if (strcmp(messaggio_ricevuto.mtext, "mobydick") == 0)
                {
                    char comando[STR_DEFAULT] = {0};
                    snprintf(comando, sizeof(comando), "./ahab.out %d", timonierePID);
                    system(comando); // call captain Ahab using shell
                }
            }
        }
    }

    // Moby dick
    // request #9
    // receive a signal SIGRTMAX

    if (getpid() == nave_pid)
    {
        // install handler
        struct sigaction sa_nave;
        sigemptyset(&sa_nave.sa_mask);
        sigaddset(&sa_nave.sa_mask, SIGRTMAX);
        sa_nave.sa_flags = SA_SIGINFO;
        sa_nave.sa_sigaction = handler_nave;
        sigaction(SIGRTMAX, &sa_nave, NULL);

        while (!flag_exit)
        {
            if (flag_SIGRTMAX)
            {
                flag_SIGRTMAX = false; // reset flag
                int arpionieri_uccisi = 0;
                for (int i = 0; i < n; i++)
                {
                    if (kill(childs_arpionieri[i], 0) == -1) // if the PID is not existent meaning the child has been killed
                    {
                        arpionieri_uccisi++;
                    }
                }
                union sigval segnale;
                segnale.sival_int = arpionieri_uccisi;
                sigqueue(timonierePID, SIGRTMIN, segnale);
            }
        }
    }

    if (getpid() == timonierePID) // if I am timoniere I'll wait for a signal
    {
        // install handler
        struct sigaction sa_timoniere;
        sigemptyset(&sa_timoniere.sa_mask);
        sigaddset(&sa_timoniere.sa_mask, SIGRTMAX);
        sa_timoniere.sa_flags = SA_SIGINFO;
        sa_timoniere.sa_sigaction = handler_timoniere;
        sigaction(SIGUSR2, &sa_timoniere, NULL);

        while (!flag_exit)
        {
            if (flag_SIGRTMIN)
            {
                flag_SIGRTMIN = true; // reset flag
            }
        }
    }

    while (!flag_exit)
    {
    }

    return exit_value;
}

// Section 8: function definition
void handler_vedetta(int signo, siginfo_t *info, void *empty)
{
    flag_SIGUSR2 = true;
}

void handler_nave(int signo, siginfo_t *info, void *empty) // request #9
{
    flag_SIGRTMAX = true;
}

void handler_timoniere(int signo, siginfo_t *info, void *empty) // request #9
{
    flag_SIGRTMIN = true;
    printf("Sono il timoniere, sono stati uccisi %d arpionieri\n", info->si_value.sival_int);
}

void handler_capitano(int signo, siginfo_t *info, void *empty)
{
    flag_CAPITANO = true;
}
