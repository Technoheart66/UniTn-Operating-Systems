// simulazione_LabSO_26052025

// Section 0: author & copyright
// Francesco Dall'Agata

// Section 1: index
// 0) author & copyright
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
#include <math.h> // floor(), ceil(), neabyint(), remember to link with gcc <name.c> -lm

// Section 3: define
#define _GNU_SOURCE // or _POSIX_C_SOURCE 200809L
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
int check_preconditions(int *argc, char *argv[], int *numeroGiocatori, FILE *stream_arbitro); // controlli pre esecuzione, lettura parametri, lettura file, creazione fifo
int get_random_int(int *min, int *max);                                                       // returns a pseudo-random number between min and max included
void list_insert_child(node_t *head, pid_t *pid_child);                                       // inserts a child as a new node at the end of the list
void list_print(node_t *head);                                                                // prints the whole list (of childs)
int fai_giocare(node_t *head);                                                                // sends signals SIGUSR2, selects 2 childs (one and the next) until there is just 1 left
void handler_giocatori(int signo, siginfo_t *info, void *empty);                              // signal handler of the childs "giocatori", handles SIGUSR2

// Section 7: main
int main(int argc, char *argv[])
{
    int result = EXIT_SUCCESS; // valore di tirono della funzione main()
    origin_PID = getpid();     // memorizziamo il processo origine

    FILE *stream_arbitro = NULL; // primo parametro passato al programma: <arbitro>
    int numeroGiocatori = 0;     // secondo parametro passato al programma: <numeroGiocatori>

    char *fifo_punteggio = "./tmp/fifo_punteggio";       // FIFO o pipe con nome, utilizzata successivamente per accordare i giocatori sul punteggio
    if (mkfifo(fifo_punteggio, S_IRUSR | S_IWUSR) != -1) // origin process and children since they inherit UID of parent
    {
        printf("FIFO created succesfully...\n");
    }
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
                }
                else
                {
                    printf("ERROR: couldn't open file '%s'\n", file_giocatore);
                }

                break;

            default: // fork() ritorna al genitore il PID del processo appena creato
                // origin
                list_insert_child(lista, &child);
                break;
            }
        } // ora abbiamo finito di generare i figli e di registrarli

        if (getpid() == origin_PID) // se sono il processo origine
        {
            // origin parent
            sleep(1);
            do
            {
                // just send the signals until all childs are done
            } while (fai_giocare(lista) != 0);

            // send signals

            // ensure all childs are done by waiting for all of them, then we can terminate
            pid_t child_terminating_pid = 0; // use with wait()
            int child_status = 0;

            // keeps waiting for child processes to finish
            do
            {
                // wait() will await for a child process to terminate, and returns that child process's pid. On error (eg when there are no child processes), -1 is returned.
                child_terminating_pid = wait(&child_status);
                printf("waiting childs...\nterminating...%d\n", child_terminating_pid);
            } while (child_terminating_pid != -1);
        }
        else
        {
            // child

            // I giocatori dovranno bloccare il segnale SIGUSR1
            sigset_t sigset_block_giocatori, sigset_old;
            int check_mask = 0;
            check_mask = sigemptyset(&sigset_block_giocatori);        // returns 0 upon success
            check_mask = sigaddset(&sigset_block_giocatori, SIGUSR1); // returns 0 upon successo
            if (check_mask == 0 && sigismember(&sigset_block_giocatori, SIGUSR1) == 1)
            {
                sigprocmask(SIG_BLOCK, &sigset_block_giocatori, &sigset_old);
                printf("succesfully set mask: block SIGUSR1\n");

                // I giocatori dovranno rimanere in attesa del segnale SIGUSR2
                sigset_t sigset_wait_giocatori;
                check_mask = sigfillset(&sigset_wait_giocatori);         // let's bloack everything
                check_mask = sigdelset(&sigset_wait_giocatori, SIGUSR2); // only SIGUSR2 can go through
                if (check_mask == 0 && sigismember(&sigset_wait_giocatori, SIGUSR2) != 1)
                {
                    printf("succesfully set mask: sigsuspend SIGUSR2\n");

                    // set up sigaction with SIGUSR2
                    struct sigaction sa_giocatore, sa_oldact_giocatore;
                    sa_giocatore.sa_sigaction = handler_giocatori;
                    sa_giocatore.sa_flags = SA_SIGINFO; // Use sa_sigaction
                    sigemptyset(&sa_giocatore.sa_mask); // Define an empty mask
                    printf("succesfully set sigaction SIGUSR2\n");
                    sigaction(SIGUSR2, &sa_giocatore, &sa_oldact_giocatore);
                }
                else
                {
                    printf("ERROR: couldn't set sigsuspend mask\n");
                }
                sigsuspend(&sigset_wait_giocatori);
            }
            else
            {
                printf("ERROR: couldn't block SIGUSR1\n");
            }
        }
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
                // okay
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

// get random number between [min, max] included
int get_random_int(int *min, int *max)
{
    int pseudo_random = 0; // return value
    // srand(time(NULL)); set seed with the current time, doesn't work because it's the same for all processes
    srand(getpid() + time(NULL));                      // set seed with getpid(), unique for each process
    pseudo_random = rand() % (*max + 1 - *min) + *min; // get a random number                                           // divide everything by 100 to get a number to floor or ceil
    pseudo_random = nearbyintf(pseudo_random);         // round the number to the closest integer value
    return pseudo_random;
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

// print the whole list in stdout
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

// send a signal SIGUSR2 to two child processes
int fai_giocare(node_t *head)
{
    int result = -1; // default: fail
    node_t *current = head;

    while (current->next != NULL && getpid() == origin_PID)
    {
        printf("DEBUG: Sending SIGUSR2 to %d and %d\n", current->pid, current->next->pid); // debug
        union sigval value;
        value.sival_int = current->next->pid;
        sigqueue(current->pid, SIGUSR2, value); // send SIGUSR2 to first child with PID of second child

        value.sival_int = current->pid;
        sigqueue(current->next->pid, SIGUSR2, value); // send SIGUSR2 to second child with PID of first child

        current = current->next;
        result = 0; // signal sent, set result to success status
        sleep(1);
    }

    return result;
}

// receives SIGUSR2
void handler_giocatori(int signo, siginfo_t *info, void *empty)
{
    char file_giocatore[STR_FILE_LENGTH]; // buffer per la stringa del file del giocatore
    char file_avversario[STR_FILE_LENGTH];

    printf("PID: %d: gioco contro %d\n", getpid(), info->si_value.sival_int);

    snprintf(file_giocatore, STR_FILE_LENGTH, "./tmp/%d.txt", getpid());
    snprintf(file_avversario, STR_FILE_LENGTH, "./tmp/%d.txt", info->si_value.sival_int);

    FILE *stream_leggi = fopen(file_giocatore, "r");
    FILE *stream_leggi_avversario = fopen(file_avversario, "r");
    if (stream_leggi && stream_leggi_avversario)
    {
        char read_input[STR_FILE_LENGTH];
        char read_input_avversario[STR_FILE_LENGTH];
        int read_elo = 0;
        int read_elo_avversario = 0;

        if (fgets(read_input, sizeof(read_input), stream_leggi) && fgets(read_input_avversario, sizeof(read_input_avversario), stream_leggi_avversario))
        {
            int matches = sscanf(read_input, "%d", &read_elo);
            int matches_avversario = sscanf(read_input_avversario, "%d", &read_elo_avversario);
            int fifo_result;
            char punteggio[STR_FILE_LENGTH];

            if (matches == 1 && matches_avversario == 1) // 1 match, the variable has been assigned
            {
                if (read_elo > read_elo_avversario)
                {
                    printf("%d vinco, il mio elo era %d\n", getpid(), read_elo);
                    // leggo su FIFO il punteggio dell'altro giocatore
                    fifo_result = open("./tmp/fifo_punteggio", O_RDONLY); // open FIFO for read only
                    read(fifo_result, punteggio, STR_FILE_LENGTH);
                    close(fifo_result);
                    printf("%d ho letto %s\n", getpid(), punteggio);
                }
                else
                {
                    printf("%d perdo, il mio elo era %d\n", getpid(), read_elo);
                    int min_punti = 0;
                    int max_punti = 10;
                    // scrivo su FIFO il mio punteggio, il mio avversario avrà punteggio 11 siccome ha vinto
                    snprintf(punteggio, sizeof(punteggio), "%d", get_random_int(&min_punti, &max_punti));
                    fifo_result = open("./tmp/fifo_punteggio", O_WRONLY); // open FIFO for read only
                    write(fifo_result, punteggio, STR_FILE_LENGTH);
                    close(fifo_result);
                    printf("%d ho scritto %s\n", getpid(), punteggio);
                }
            }
            else
            {
                printf("ERROR: parsing elo\n");
            }
        }
        else
        {
            printf("ERROR: file reading error while trying to read ELO\n");
        }
    }
    else
    {
        printf("ERROR: couldn't open file streams\n");
    }

    fclose(stream_leggi);
    fclose(stream_leggi_avversario);
}

// soluzione:
// coda IPC_PRIVAT o ftok
// creazione figli -> gestione errori input 2-32 figli, codici di errore, gestione elo
// otteniamo la coda con msgget, sigset con SIGURS1 sigprocmask sigusr1
// signal handler con sigaction per sig_info, con SIGUSR2
// controlliamo una flag che mi fa stare in attesa (penso volatile sig_atomic_t)
// coda con payload con score e elo, le code supportano l'invio di payload che sono struct, era possibile anche inviare una stringa e convertirla