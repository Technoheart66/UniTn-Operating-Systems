#!/usr/bin/env bash
# Scrivere uno script bash, con shebang, che accetta 10 parole (gestire con un errore i casi
# in cui ne vengano date meno o più) e le restituisce su standard output in ordine alfabetico.

nArgs=$#
if [[ ${nArgs} -eq 10 ]]; then
    echo "correcly passed 10 args"
else
    echo "ERROR: there must be exacly 10 args"
    exit
fi
listArgs=("$@")
sorted=$(printf "%s\n" "${listArgs[@]}"| sort)
printf "%s\n" "${sorted}"
