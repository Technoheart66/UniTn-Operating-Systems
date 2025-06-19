#!/usr/bin/env bash
#Scrivere uno script che dato un qualunque numero di argomenti li restituisca in  output in ordine inverso.
nArgs=$#
argv=("$@") #be careful here! this is the right syntax, otherwise it could make a single string with all args or other
i=0
#while [[ $i < 9 ]]; do     no this doesn't work because < and > are for string comparison!
while [[ $i -lt $nArgs ]]; do
  echo "${argv[$i]}"
  ((i++))
done

#DOESN'T WORK
