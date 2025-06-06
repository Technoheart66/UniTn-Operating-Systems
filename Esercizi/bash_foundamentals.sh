# esercizi dalle slide
# Scrivere delle sequenze di comandi (singola riga da eseguire tutta in blocco) che utilizzano come “input” il valore della variabile DATA per:
# 1. Stampa “T” (per True) o “F” (per False) a seconda che il valore rappresenti un file o una cartella esistente
# 2. Stampa “file”, “cartella” o “?” a seconda che il valore rappresenti un file (esistente), una cartella (esistente) o una voce non presente nel file-system
# 3. Stampa il risultato di una semplice operazione aritmetica (es: ‘1 < 2’) contenuta nel file indicato dal valore di DATA, oppure “?” se il file non esiste
# NB: usate le subshell se necessario!
# NB: è posibile copiare l'esercizio direttamente sulla shell

# esercizio 1
DATA=$(pwd)
[[ -e $DATA ]] && echo "T" || [[ ! -e $DATA ]] && echo "F"
# oppure
[[ ! -e ${DATA} ]] && echo "F" || [[ -e ${DATA} ]] && echo "T"
# oppure
[[ -e $DATA ]] && echo "T" || echo "F"

