#Scrivere uno script che mostri il contenuto della cartella corrente in ordine
#inverso rispetto all’output generato da “ls” (che si può usare ma senza opzioni).
#Per semplicità, assumere che tutti i file e le cartelle non abbiano spazi nel nome.

DATA=$(ls) #I think it returns a single string, thus I must break it down
for i in ${!DATA[@]}; do
  echo $i
  ((i++))
done
