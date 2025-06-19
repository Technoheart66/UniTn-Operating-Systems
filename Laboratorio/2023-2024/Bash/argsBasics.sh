args=("$@") #here I am assigning the arguments list to a variable, note that it is quoted
echo $# printing the total number of arguments
echo $args "printing args=("$@"), just the first element will be printed becuase each argument is treated as a separated string"

args="$@"
echo $args "printing args="$@", it may appear like all elements are printed but it actually collapses all argument names in a single string"
