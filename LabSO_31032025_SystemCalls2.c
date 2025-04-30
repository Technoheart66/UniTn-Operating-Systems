// Lezione di laboratorio del 31/03/2025

// Titolo: Esempi di System Call e Forking

time_t time(time_t *second);
char *ctime(const time_t *timeSeconds);

#include <time.h> //time.c
#include <stdio.h>
int main()
{
  time_t theTime;
  time_t whatTime = time(&theTime); // seconds since 1/1/1970
  // Dopo le operazioni sopra ho l'orario sia in 'whatTime' sia in 'theTime'
  // Print date in Www Mmm dd hh:mm:ss yyyy
  printf("Current time = %s= %d\n", ctime(&whatTime), theTime); // stampo l'orario in due modi diversi
}

