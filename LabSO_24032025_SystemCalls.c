// Lezione di laboratorio del 24/03/2025

// Titolo: System Calls

/*
Prima di vedere le system calls andiamo a vedere l'architettura di un calcolatore Unix-like

Cpitolo 1: Architettura

Kernel, vari tupi:
  -Monolithic, molte funzioni, pesante e spesso non è la soluzione più adatta
  -Micro kernel, si cerca di creare un set minimo di istruzioni

Livello applicativo e livello sistema operativo
L'applicazione non può fare molto, non sceglie l'utilizzo di CPU, core, memoria e tutte le risorse,
ma chiede al Kernel, che fa da mediatore per concedere le risorse

In ordine, all'avviamento di una macchina:
  Firmware ->
    Secure boot->
      Kernel (periferiche, processi iniziali, processo init etc).

Il linguaggio C è particolarmente portato per interagire con il kernel, siccome il C è già di basso livello
Per interagire con il kernel, una maniera (o unica maniera) è utilizzrae le system calls
Uno degli obbiettivi del corso è far interagire i processi tra di loro, anche in questo caso è il kernel che-
fa da mediatore tra i processi.

Se facessimo print(&bar) otterremmo l'indirizzo di memoria virtuale della variabile bar.
Attenzione, se riavviamo il processo notiamo che l'indirizzo stampato è lo stesso, questo non significa che la memoria fisica sia-
la stessa. Bensì a livello applicativo ha una visione privata (individuale) delle sue risorse, non significa che il processo-
abbia il monopolio sul sistema, tuttavia ogni programma vedrà se stesso come unico possessore della CPU.

Cosa impedisce ad un applicazione di modificare il sistema e gli indirizzi?
Ci sono dei meccanismi che variano tra architetture (visto x86, visto ARM, RISC etc.)
Sono i livelli, associati solitamente ad un entità software, con vari privilegi.
Tendenzialmente ci sono 4 ring di privligi, che perciò forniscono uno stack di 4 livelli.
ARM ha solo due livelli, mentre x86 ne ha quattro.
User space è il livello più esterno, dove noi eseguiamo le nostre applicazioni.
Kernel space: ambiente in cui viene eseguito il kernel

Come ci interfacciamo con il kernel per fargli richieste? Con le system calls
*/

/*
Capitolo 2: System Calls

Le interface con cui i programmi accedono all'hardware attraverso il kernel si chiamano system calls.
Ad esempio, vogliamo passare in maniera esplicita e controllata da user space a kernel space, per farlo-
facciamo una system calls e il kernel controllerà parametri, priviliegi etc. e vedrà se eseguire la system call.
Quasi tutte le system calls ritorneranno -1 in caso di errore. Ad esempio se un file è read-only e noi-
proviamo a scrivere sul file essa ritorna errore.

Spesso all'esame si sbaglia mancando di controllare se la system call fallisce.
Una buona code practice è controllare l'esito di una system call precisa in caso di problemi/errori.
Ad esempio una system call che fallisce a trovare un file ritorna il valore -127.

Una funzione che genera una system call in questo corso verrà chiamata direttamente system call, anche se non è corretto.
Ad esempio la funzione print non è una system call, ma quando scrive a terminale la system call sottostante lo farà.

Solitamente anche se noi non facciamo system calls e non includiamo librerie per le system calls, eseguendo-
il comando ld-linux.so possiamo notare con di default verranno aggiunte delle system calls per configurare lo stack, la-
memoria, etc. tutte queste cose vengono astratte ed inserite al di fuori del controllo del programmatore.

La libreria libc.so è quella che contiene le funzioni basilari più comuni

Nel codice seguento usiamo wrappers che tradurranno le system calls per qualsiasi architettura.
Notiamo che se uso codice assembly conosco e devo conoscere l'architettura in cui esegue l'applicaizone.
I wrapper andranno a tradurre le system calls nel codice assemly appropriato per l'architettura.
*/

//su Windows non funziona, è system-specific perciò includo unistd.h se voglio eseguire su windows oppure windows.h
//#include <libc.so>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
int main(void)
{
  pid_t pid, pid2;
  pid = getpid();
  pid2 = (pid_t)syscall(SYS_getpid);
  printf("Process ID: %d %d\n", pid, pid2);
  __asm__("movq $60, %rax"); // Set up the syscall ID (SYS_exit) (x86-64)
  __asm__("movq $10, %rdi"); // Set up the syscall parameter (ret code 10)
  __asm__("syscall");        // Trigger the syscall
  printf("This line will not be printed\n");
}
/*
Capitolo 3: Interazioni con i file

In Linux ci sono due modi per interagire con i file:
  Streams, interfaccia di alto livello, a basso livello utilizzano comunque i file descriptors.
    Forniscono strumenti come formattazione dati, buffering etc.
  File descriptors, interfaccia a basso livello costituita dalle system calls messe a disposizione dal SO.

Streams
  Un file è descritto da un puntatore a una struttura di tipo FILE (definita in stdio.h)
  Notiamo come quasi tutte le funzioni delle stream iniziano con una f, ad esempio fopen

  FILE *fopen(const char* filename, const char* mode)

  Restituisce un FILE pointer (o NULL se errore → controllare esito!) per gestire il
  filename nella modalità specificata da mode.
  ● r: read
  ● w: write or overwrite (create)
  ● r+: read and write
  ● w+: read and write. Create or overwrite
  ● a: write at end (create)
  ● a+: read and write at end (create)
  int fclose(FILE *stream);

  È buona pratica controllare l'esito di fopen

  Facendo ls -l si ottiene la lista dei file insieme ad i permissi degli utenti.
  Sulla sinistra notiamo una stringa che descrive i permessi per i vari utenti:
    -rw-r--r--
    Da sinistra gli utenti sono User, Group, Others

  int fgetc(FILE *stream)
    Restituisce un carattere dallo stream.
  char *fgets(char *str, int n, FILE *stream)
    Restituisce una stringa da stream e la salva in str. Si ferma quando n-1 caratteri-
    sono stati letti, una nuova linea (\n) è letta o la fine del file viene raggiunta.
    Inserisce anche il carattere di terminazione e, eventualmente, ‘\n’
  int fscanf(FILE *stream, const char *format, ...)
    Legge da stream dei dati, salvandoli nelle variabili fornite (simile a printf)-
    secondo la stringa format. Restituisce il numero di variabili lette
  int feof(FILE *stream)
    Restituisce 1 se lo stream ha raggiunto la fine del file, 0 altrimenti.

*/

// Leggere con file Streams
#include <stdio.h> //fscan1.c
int main(void)
{
  FILE *ptr;                         // Declare stream file
  ptr = fopen("filename.txt", "r+"); // Open
  if (ptr == NULL)
    return 4;
  int id;
  char str1[10], str2[10];
  while (!feof(ptr))
  { // Check end of file
    // Read int, word and word
    int numOfConversions = fscanf(ptr, "%d %s %s", &id, str1, str2);
    printf("%d %s %s\n", id, str1, str2);
  }
  printf("End of file\n");
  fclose(ptr); // Close file
}

// Leggere con file Streams - per carattere
int main(void)
{
  FILE *ptr;                         // Declare stream file
  ptr = fopen("filename.txt", "r+"); // Open
  char str[10][90];
  char newChar = fgetc(ptr);
  int row = 0, col = 0;
  while (newChar != EOF)
  {
    str[row][col++] = newChar;
    if (newChar == '\n')
    {
      str[row++][col] = '\0';
      col = 0;
    }
    newChar = fgetc(ptr); // Read char by char
  };
  str[row][col] = '\0';
  for (int i = 0; i <= row; i++)
  {
    printf("%s", str[i]);
  }
  printf("End of file\n");
  fclose(ptr); // Close file
}

// Leggere con file Streams - per riga
int main(void)
{
  FILE *ptr;                         // Declare stream file
  ptr = fopen("filename.txt", "r+"); // Open
  char str[10][90];
  int row = 0;
  while (feof(ptr) != 1)
  {
    fgets(str[row++], 90, ptr);
  };
  for (int i = 0; i <= row; i++)
  {
    printf("%s", str[i]);
  }
  printf("End of file\n");
  fclose(ptr); // Close file
}

/*
  Devo immaginare i file come un grande buffer che viene letto.
  Che sia per stringa, carattere o riga non fa differenza.
*/

// FLeggere con file Streams, occhio a scanf, funzione da usare solo in contesti conosciuti
#include <stdio.h> //fscan1.c
int main(void)
{
  FILE *ptr;                         // Declare stream file
  ptr = fopen("filename.txt", "r+"); // Open
  if (ptr == NULL)
    return 4;
  int id;
  char nome[10], cognome[10];
  while (!feof(ptr))
  { // Check end of file
    // Read int, word and word
    int numOfConversions = fscanf(ptr, "%d %s %s", &id, nome, cognome); // format, spazio come separatore
    printf("%d %s %s\n", id, nome, cognome);
  }
  printf("End of file\n");
  fclose(ptr); // Close file
}

/*
  Scrivere su file con streams

  int fputc(int char, FILE *stream)
    Scrive un singolo carattere char su stream.
  int fputs(const char *str, FILE *stream)
    Scrive una stringa str su stream senza includere il carattere null.
  int fprintf(FILE *stream, const char *format, ...)
    Scrive il contenuto di alcune variabili su stream, seguendo la stringa format,-
    senza includere il carattere null.

  E molte altre…. Quando scriviamo su un file non serve il terminatore!

  Devo tenere a mente della 'testina', ossia del puntatore che legge.
  Questo puntatore verrà usato anche per scrivere, perciò se voglio sovvrascrivere un file devo fare-
  un rewind e spostare la testina indietro.
*/

int write(void)
{
#include <stdio.h>
  FILE *ptr;
  char *str = "his is an interesting text to be written"; // Manca la T, voglio inserirla via codice dopo
  ptr = fopen("fileToWrite.txt", "w+");
  fputc('T', ptr);                           // Write only one char
  fputs(str, ptr);                           // Write one string without terminator
  rewind(ptr);                               // Reset pointer to begin of file
  fprintf(ptr, "Writing %d characters", 21); // Write string without terminator
  fclose(ptr);
}

/*
File descriptors

  Ad ogni file viene assegnato un intero, ogni volta che scriviamo su un file utilizziamo l'intero-
  per trovare il file, ogni processo assegna un intero locale ad un determinato file.
  Non viene astratto nulla, devo fare tutte le operazioni e nulla viene fatto in automatico.
  Più complesso ma consente più controllo.

  Ogni processo possiede una file table, ogni elemento della tabella rappresenta un file aperto ed è-
  individuato da un intero, che è appunto il 'file descriptor'

  Nella file table, in condizioni standard i file descriptor 0, 1 e 2 indicato stdin, stdout e stderr -
  essi vengono aperti automaticamente.

  Il kernel gestisce l'accesso ai file attraverso due strutture dati:
    la tabella dei file attivi, contiene una copia dell’i-node di ogni file aperto (per efficienza).
    la tabella dei file apertil, contiene un elemento per ogni file aperto e non ancora chiuso.
      Contiene:
        I/O pointer, posizione corrente nel file
        i-node pointer, puntatore a i-node corrente
      N.B. la tabella dei file aperti può avere più elementi corrispondenti allo stesso file! (Immagine nelle slide)

  Aprire e chiudere un file con file descriptors

  int open(const char *pathname, int flags[, mode_t mode]);

  flags: interi (ORed) che definiscono l’apertura del file. I più comuni:
    ● O_RDONLY, O_WRONLY, O_RDWR: almeno uno è obbligatorio.
    ● O_CREAT: crea il file se non esiste (con O_EXCL la chiamata fallisce se esiste)
    ● O_APPEND: apre il file in append-mode (auto lseek con ogni scrittura)
    ● O_TRUNC: cancella il contenuto del file (se usato con la modalità scrittura)

  mode: interi (ORed) per i privilegi da assegnare al nuovo file:
    S_IRUSR, S_IWUSR, S_IXUSR, S_IRWXU, S_IRGRP, …, S_IROTH

  La system call restituisce il primo file descriptor disponibile (3 se è il primo file)

  int close(int fd);

  ssize_t read (int fd, void *buf, size_t count);
    Legge dal file e salva nel buffer buf fino a count bytes di dati dal file associato con il file descriptor fd.
  ssize_t write(int fd, const void *buf, size_t count);
    Scrive sul file associato al file descriptor fd fino a count bytes di dati dal buffer buf.
  off_t lseek(int fd, off_t offset, int whence);
    Riposiziona l’offset del file a seconda dell’argomento offset partendo da una certa posizione whence.
      SEEK_SET: inizio del file
      SEEK_CUR: dalla posizione attuale
      SEEK_END: dalla fine del file

*/

// ricordiamo che una stringa ha il terminatore, non posso fare un print senza terminatore
int bytesRead(void)
{
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
  // Open existing file in Read only
  int openedFile = open("name.txt", O_RDONLY);
  if (openedFile == -1)
    return 4;
  char content[10];
  int bytesRead;
  do
  {
    bytesRead = read(openedFile, content, 9); // Read 9B to content
    content[bytesRead] = 0;                   // The file does not contain the null terminator
    printf("%s", content);                    // la stringa deve avere il terminatore
  } while (bytesRead > 0);
  close(openedFile);
}