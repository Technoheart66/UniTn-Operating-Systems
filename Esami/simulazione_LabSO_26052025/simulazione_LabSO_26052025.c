// simulazione_LabSO_26052025

// Section 0: author
// Francesco Dall'Agata

// Section 1: index
// 0) author
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
#include <signal.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>
#include <math.h> // floor(), ceil(), nearbyint(), remember to link with gcc <name.c> -lm

// Section 3: define
#define STR_FILE_LENGTH 50
#define ELO_MIN 100
#define ELO_MAX 4000

// Section 4: typedef
typedef struct node // I need a way to store each child information, so let's do a linked list
{
    pid_t pid;
    struct node *next;
} node_t;

// Fate il flush dopo la scrittura su stdout e stderr!

// Section 5: global variables
volatile sig_atomic_t origin_PID; // volatile così il compilatore non può effettuare ottimizzazioni, sig_atomic_t così viene previsto l'accesso non sincronizzato (i.e. segnali)

// Section 6: function declaration
int check_preconditions(int *argc, char *argv[], int *numeroGiocatori, FILE *stream_arbitro); // controlli pre esecuzione
int calculate_elo(int *min, int *max);                                                        // returns a pseudo-random number between min and max included
void list_insert_child(node_t *head, pid_t *pid_child);
void list_print(node_t *head);

// Section 7: main
int main(int argc, char *argv[])
{
    int result = EXIT_SUCCESS; // valore di tirono della funzione main()
    origin_PID = getpid();     // memorizziamo il processo origine

    FILE *stream_arbitro = NULL; // primo parametro passato al programma: <arbitro>
    int numeroGiocatori = 0;     // secondo parametro passato al programma: <numeroGiocatori>

    node_t *lista = NULL; // I need to store child PIDs
    lista = (node_t *)malloc(sizeof(node_t));
    if (lista == NULL)
    {
        printf("ERROR: failed to allocate memory for the list\n");
        return EXIT_FAILURE;
    }
    lista->pid = 0;
    lista->next = NULL;

    result = check_preconditions(&argc, argv, &numeroGiocatori, stream_arbitro);
    if (result == EXIT_SUCCESS)
    {
        char file_giocatore[STR_FILE_LENGTH]; // buffer per generare la stringa del file dei giocatori

        for (int i = 0; i < numeroGiocatori && getpid() == origin_PID; i++)
        {
            int child = fork(); // creiamo un nuovo proceso
            switch (child)      // controlliamo il valore di ritorno di fork()
            {
            case 0: // fork() ritorna 0 se il processo è figlio
                // child

                snprintf(file_giocatore, STR_FILE_LENGTH, "./tmp/%d.txt", getpid()); // nome del nuovo file, come specificato deve essere <mioPID>.txt
                FILE *nuovoFile = fopen(file_giocatore, "w+");
                if (nuovoFile != NULL)
                {
                    // Registrazione, nuovo file <mioPID>.txt e scrivo ELO
                    int elo_valore_minimo = ELO_MIN;
                    int elo_valore_massimo = ELO_MAX;
                    // int elo = calculate_elo(&elo_valore_minimo, &elo_valore_massimo); // calcola un numero casuale multiplo di 100 tra 100 e 4000
                    int elo = (i + 1) * 100; // deve essere != 0 e unico, è il metodo più semplice
                    fprintf(nuovoFile, "%d\n", elo);
                    fflush(nuovoFile);

                    // I giocatori dovranno bloccare il segnale SIGUSR1
                    sigset_t sigset_block_giocatori, sigset_old;
                    int check_mask = 0;
                    check_mask = sigemptyset(&sigset_block_giocatori);        // returns 0 upon success
                    check_mask = sigaddset(&sigset_block_giocatori, SIGUSR1); // returns 0 upon successo
                    if (check_mask == 0 && sigismember(&sigset_block_giocatori, SIGUSR1) == 1)
                    {
                        printf("setting sigprocmask\n");
                        sigprocmask(SIG_BLOCK, &sigset_block_giocatori, &sigset_old);
                        sigset_t sigset_wait_giocatori;
                        check_mask = sigfillset(&sigset_wait_giocatori);         // let's bloack everything
                        check_mask = sigdelset(&sigset_wait_giocatori, SIGUSR2); // only SIGUSR2 can go through
                        if (check_mask == 0 && sigismember(&sigset_wait_giocatori, SIGUSR2) != 1)
                        {
                            printf("setting sigsuspend\n");
                            do
                            {
                                sigsuspend(&sigset_wait_giocatori);
                            } while (true);
                        }
                    }
                    else
                    {
                        printf("ERROR: custom signal mask error\n");
                    }
                }
                else
                {
                    printf("ERROR: couldn't open file '%s'\n", file_giocatore);
                }

                break;

            default: // fork() ritorna al genitore il PID del processo appena creato
                // parent
                list_insert_child(lista, &child);
                break;
            }
        } // ora abbiamo finito di generare i figli e di registrarli

        pid_t child_terminating_pid = 0; // use with wait()
        int child_status = 0;
        do
        {
            child_terminating_pid = wait(&child_status); // wait waits for a child process to terminate, and returns that child process's pid. On error (eg when there are no child processes), -1 is returned.
        } while (child_terminating_pid > 0); // keeps waiting for child processes to finish, wait() returns error -1 if it fails meaning there are no more childs
    }

    return result;
}

// check pre-conditions, such as correct file opening, correct number of parameters etc.
int check_preconditions(int *argc, char *argv[], int *numeroGiocatori, FILE *stream_arbitro)
{
    // Controlli pre-esecuzione
    // - il numero di parametri deve essere esattamente 3: il programma in sé, il filepath <arbitro> e l'intero <numeroGiocatori>. In caso contrario il programma termina con codice 50
    // - <numeroGiocatori> deve essere tra 2 e 32 compresi
    int result = EXIT_SUCCESS;
    if (*argc == 3) // check if the parameters are exactly 3
    {
        // char file_arbitro[50];
        // snprintf(file_arbitro, STR_FILE_LENGTH, "%s", argv[1]);
        // printf("file arbitro: %s\n", file_arbitro);
        stream_arbitro = fopen(argv[1], "r"); // parametro 1 <arbitro>, apre la stream in lettura
        *numeroGiocatori = atoi(argv[2]);     // parametro 2 <numeroGiocatori>, converte ASCII -> integer
        if (stream_arbitro != NULL)           // check if file exists
        {
            if (*numeroGiocatori >= 2 && *numeroGiocatori <= 32)
            {
            }
            else
            {
                printf("ERROR: Invalid parameter <numeroGiocatori>\n<numeroGiocatori> must be between 2 - 32 included\nUSAGE: ./main.out /percorso/arbitro <numeroGiocatori>\n");
                // è specificato che in caso di parametri errati il programma dovrà terminare con valore 50 tuttavia ho impostato un codice d'errore personalizzato
                result = 52;
            }
        }
        else
        {
            printf("ERROR: file error <arbitro>\nUSAGE: ./main.out /percorso/arbitro <numeroGiocatori>\n");
            // codice d'errore personalizzato
            result = 51;
        }
    }
    else
    {
        printf("ERROR: Invalid parameters\nUSAGE: ./main.out /percorso/arbitro 32\n");
        result = 50; // come specificato, in caso di parametri errati il programma dovrà terminare con valore 50
    }
    return result;
}

// get random number between [lower_limint, upper_limit] included
int calculate_elo(int *lower_limit, int *upper_limit)
{
    float correct_elo = 0; // return value
    // srand(time(NULL)); set seed with the current time, doesn't work because it's the same for all processes
    srand(getpid() + time(NULL)); // set seed with getpid(), unique for each process

    int pseudo_random = rand() % (*upper_limit + 1 - *lower_limit) + *lower_limit; // get a random number
    correct_elo = pseudo_random / 100;                                             // divide everything by 100 to get a number to floor or ceil
    correct_elo = nearbyintf(correct_elo);                                         // round the number to the closest integer value
    correct_elo *= 100;                                                            // multiply again by 100 since thye have to be multiples of 100
    return correct_elo;
}

// insert a node containing the child PID at the end of the list
void list_insert_child(node_t *head, pid_t *pid_child)
{
    node_t *current = head;

    if (current->next == NULL && current->pid == 0)
    {
        // if the list is empty

        head->pid = *pid_child;
        head->next = NULL;
    }
    else
    {
        // if the list is not empty

        while (current->next != NULL)
        {
            current = current->next;
        }
        // now current is the last element

        node_t *new_child = NULL;
        new_child = (node_t *)malloc(sizeof(node_t));
        if (new_child == NULL)
        {
            printf("ERROR: failed to allocate memory for new_child in list_insert_child\n");
        }
        new_child->pid = *pid_child;
        new_child->next = NULL;
        current->next = new_child;
    }
}

void list_print(node_t *head)
{
    node_t *current = head;
    printf("Printing child list:\n");
    while (current != NULL)
    {
        printf("child PID: %d\n", current->pid);
        current = current->next;
    }
}

// soluzione:
// coda IPC_PRIVAT o ftok
// creazione figli -> gestione errori input 2-32 figli, codici di errore, gestione elo
// otteniamo la coda con msgget, sigset con SIGURS1 sigprocmask sigusr1
// signal handler con sigaction per sig_info, con SIGUSR2
// controlliamo una flag che mi fa stare in attesa (penso volatile sig_atomic_t)
// coda con payload con score e elo, le code supportano l'invio di payload che sono struct, era possibile anche inviare una stringa e convertirla