#!/bin/bash

[ ! -d "./bin" ] && mkdir "./bin"

gcc -c main.c -o ./bin/main.o

gcc -x assembler -c auto_update.asm -o ./bin/git_update.o

cd bin

gcc main.o git_update.o -o main

chmod +x ./main

./main

echo -e ""

cd ..