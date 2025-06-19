#!/usr/bin/env bash
#Scrivere uno script che dato un qualunque numero di argomenti li restituisca in  output in ordine inverso.

nArgs=$#
echo "There are $nArgs arguments"
listaArgs=("$@")
i=$nArgs
while [[ $i -ge 0 ]]; do
  echo ${listaArgs[$i]}
  (( i-- ))
done
