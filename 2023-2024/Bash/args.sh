#!/usr/bin/env bash
nargs=$#
while [[ $1 != "" ]]; do
echo "ARG=$1"
shift #shift all argurments to the left, $3->$2 $2->$1 etc.
done
