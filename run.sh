#!/bin/bash

[ ! -d "./bin" ] && mkdir "./bin"

git pull

gcc -c main.c -I./include -o ./bin/main.o

gcc -c api.c -I./include -o ./bin/api.o

as auto_update.asm -o ./bin/git_update.o

cd bin

gcc main.o git_update.o api.o -o main

chmod +x ./main

cd ..

./bin/main

echo -e ""