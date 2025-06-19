// Lezione di laboratorio del 05/05/2025

// Titolo: Pipe su Shell

/*
Questa sarà la prima tecnica di IPC (Inter-Process Communication) che vedremo.
Il piping consente di connettere l'output (stdout e stderr) di un comando all'input (stding) di un altro.
ls . | sort -R #stout -> stdin
ls nonExistingDir |& wc #stdout e stderr -> stdin
cat /etc/passwd | wc | less #out -> in, out-> in

I processi sono eseguiti in concorrenza utilizzando un buffer:
  ● Se pieno lo scrittore (left) si sospende fino ad avere spazio libero
  ● Se vuoto il lettore (right) si sospende fino ad avere i dati
*/

// esempio 2, inverte le lettere maiuscole con lettere minuscole e viceversa
#include <stdio.h> //inv.c
int esempio2()
{
  int c, d;
  // loop into stdin until EOF (as CTRL+D)
  // read from stdin
  while ((c = getchar()) != EOF)
  {
    d = c;
    if (c >= 'a' && c <= 'z')
      d -= 32;
    if (c >= 'A' && c <= 'Z')
      d += 32;
    putchar(d); // write to stdout
  };
  return (0);
}

/*
Pipe anonime
Le pipe anonime connettono processi con un grado di parentela, il collegamento è unidirezionale-
ed avviene tramite un buffer di dimensione finita.
Ad esempio padre con figlio, padre con nipote etc.

Per interagire con il buffer (la pipe) si usano due file descriptors:
  ●  uno per il lato in scrittura
  ● uno per il lato in lettura
Visto che i processi figli ereditano i file descriptors (file table), questo consente-
la comunicazione tra i processi.

Creazione pipe
  int pipe(int pipefd[2]); //fd[0] lettura, fd[1] scrittura

L’array passato come argomento sarà popolato con due nuovi file descriptors (i primi due disponibili).
Il primo elemento sarà usato per la lettura della pipe, il secondo per la scrittura.
Normalmente un processo usa il lato in lettura, ed un altro processo usa il lato in-
scrittura (unidirezionale!).

*/

// Esempio creazione pipe

// Esempio limite creazione pipe

/*
Se noi scriviamo sul lato della pipe che è stato chiuso la write fallirà, ma nessun problema.
Se noi scriviamo sul lato della pipe di scrittura ma il lato opposto di lettura è stato chiuso, allora-
nessuno è in ascolto e non è possibile riaprire una pipe chiusa con close().

Pipe non bloccanti
  Per modificare le proprietà di una pipe, possiamo usare la system call fnctl, la
  quale manipola i file descriptors. È per esempio possibile impostare la pipe come non bloccante:
  int fcntl(int fd, F_SETFL, O_NONBLOCK);
*/

/*
Esercizi
Impostare una comunicazione bidirezionale tra due processi con due livelli di complessità:
  ● Alternando almeno due scambi (P1 → P2, P2 → P1, P1 → P2, P2 → P1)
  ● Estendendo il caso a mo’ di “ping-pong”, fino a un messaggio convenzionale di “fine comunicazione”
*/

/*
Pipe con nome/FIFO

Le pipe con nome, o FIFO, sono delle pipe che corrispondono a dei file speciali nel-
filesystem grazie ai quali i processi, senza vincoli di gerarchia, possono comunicare.
Un processo può accedere ad una di queste pipe se ha i permessi sul file-
corrispondente ed è vincolato, ovviamente, dall’esistenza del file stesso.

Le FIFO sono interpretate come dei file, perciò si possono usare le funzioni di
scrittura/lettura dei file viste nelle scorse lezioni. Restano però delle pipe, con i loro
vincoli e le loro capacità. NB: non sono dei file effettivi, quindi lseek non funziona, il-
loro contenuto è sempre vuoto e non vi ci si può scrivere con un editor!
Normalmente aprire una FIFO blocca il processo finchè anche l’altro lato non è stato aperto.
Le differenze tra pipe anonime e FIFO sono solo nella loro creazione e gestione.
*/

/*
Comunicazione sul terminale
  È possibile usare le FIFO da terminale, leggendo e scrivendo dati tramite gli
  operatori di redirezione.

  echo “message for pipe” > /path/nameOfPipe
  cat /path/nameOfPipe
  
  NB: non si possono scrivere dati usando editor di testo! Una volta consumati i dati,
  questi non sono più presenti sulla fifo. Inoltre, entrambi i comandi chiudono la pipe
  una volta completata l’operazione.
*/

int main()
{
  return esempio2;
}