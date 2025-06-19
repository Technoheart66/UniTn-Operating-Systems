echo $BASHPID #current bash process ID
echo $( echo $BASHPID ) #by using $() I'm invoking a subshell, that has a different id, then echo
