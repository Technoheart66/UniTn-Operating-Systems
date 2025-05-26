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
#define STR_LENGTH 20
#define ELO_MIN 100
#define ELO_MAX 4000

// Fate il flush dopo la scrittura su stdout e stderr!

// Section 5: global variables
volatile sig_atomic_t origin_PID; // volatile così il compilatore non può effettuare ottimizzazioni, sig_atomic_t così viene previsto l'accesso non sincronizzato (i.e. segnali)

// Section 6: function declaration
int get_random_number(int *min, int *max); // returns a pseudo-random number between min and max included

// Section 7: main
int main(int argc, char *argv[])
{
    int result = EXIT_SUCCESS;           // valore di tirono della funzione main()
    origin_PID = getpid();               // memorizziamo il processo origine
    char filename[20];                   // buffer del nome dei file da creare per ciascun giocatore
    int numeroGiocatori = atoi(argv[2]); // lettura del secondo parametro passato al programma <numeroGiocatori>
    printf("numeroGiocatori: %d\n", numeroGiocatori);

    // Controlli pre-esecuzione
    // - il numero di parametri deve essere esattamente 3: il programma in sé, il filepath <arbitro> e l'intero <numeroGiocatori>. In caso contrario il programma termina con codice 50
    // - <numeroGiocatori> deve essere tra 2 e 32 compresi
    if (argc == 3)
    {
        if (numeroGiocatori >= 2 && numeroGiocatori <= 32)
        {
            for (int i = 0; i < numeroGiocatori && getpid() == origin_PID; i++)
            {
                int child = fork(); // creiamo un nuovo proceso
                switch (child)      // controlliamo il valore di ritorno di fork()
                {
                case 0: // fork() ritorna 0 se il processo è figlio
                    // child
                    snprintf(filename, STR_LENGTH, "./tmp/%d.txt", getpid()); // nome del nuovo file, come specificato deve essere <mioPID>.txt
                    FILE *nuovoFile = fopen(filename, "w");
                    if (nuovoFile != NULL)
                    {
                        int elo_valore_minimo = ELO_MIN;
                        int elo_valore_massimo = ELO_MAX;
                        int elo = get_random_number(&elo_valore_minimo, &elo_valore_massimo); // calcoliamo l'ELO
                        // fwrite(&elo, sizeof(elo), 1, nuovoFile); // scrive byte generici sul file
                        printf("scrivo su file, ELO: %d\n", elo);
                        fprintf(nuovoFile, "%d\n", elo);
                        fflush(nuovoFile);
                        // soluzione:
                        // coda IPC_PRIVAT o fotk
                        // creazione figli -> gestione errori input 2-32 figli, codici di errore, gestione elo
                        // otteniamo la coda con msgget, sigset con SIGURS1 sigprocmask sigusr1
                        // signal handler con sigaction per sig_info, con SIGUSR2
                        // controlliamo una flag che mi fa stare in attesa (penso volatile sig_atomic_t)
                        // coda con payload con score e elo, le code supportano l'invio di payload che sono struct, era possibile anche inviare una stringa e convertirla
                    }
                    else
                    {
                        printf("ERROR: couldn't open file '%s'\n", filename);
                    }

                    break;

                default: // fork() ritorna al genitore il PID del processo appena creato
                    // parent
                    printf("Origin: creato processo figlio #%d con PID: %d\n", i, child);
                    break;
                }
            }

            do
            {
                // aspetta indefinitivamente
            } while (true);
        }
        else
        {
            printf("ERROR: Invalid parameter <numeroGiocatori>\n<numeroGiocatori> must be between 2 - 32 included\nUSAGE: ./main.out /percorso/arbitro <numeroGiocatori>\n");
            // è specificato che in caso di parametri errati il programma dovrà terminare con valore 50
            // tuttavia ho impostato un codice d'errore personalizzato
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
int get_random_number(int *lower_limit, int *upper_limit)
{
    float correct_elo = 0;
    // srand(time(NULL)); // randomize seed with the current time, doesn't work because it's the same for all processes
    srand(getpid());
    int pseudo_random = rand() % (*upper_limit + 1 - *lower_limit) + *lower_limit;
    correct_elo = pseudo_random % 100;
    correct_elo = nearbyintf(correct_elo);
    return correct_elo;
}