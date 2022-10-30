#!/bin/bash

NUM_TESTS=10
#TESTFILE=input/email-Eu-core.txt
TESTFILE=input/ca-AstroPh.txt

RUNTIMES=()

touch tmp.log

printf "Running $NUM_TESTS tests on $TESTFILE\n"
for i in $(seq $NUM_TESTS)
do
	./brandes -i $TESTFILE -o output/ -d | tee tmp.log
	RUNTIMES+="$(grep -e "\[in " tmp.log | sed 's/^.*\[in \(.*s\), .*/ \1/')"
	#RUNTIMES+= $(grep -e "100%" tmp.log | sed 's/.*\[in \(.*\),.*/\1/')
done

rm tmp.log

printf "All results:\n"
for RUNTIME in "${RUNTIMES[@]}"
do
	echo -e "$RUNTIME\n"
done
