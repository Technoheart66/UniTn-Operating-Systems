listaArgs=("$@")
nArgs=$#
while [ $nArgs -ge 0 ]; do
echo "${listaArgs[$nArgs]}"
nArgs=$(( nArgs-1 ))
done
