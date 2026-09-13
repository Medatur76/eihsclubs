#!/bin/bash

[ ! -d "./bin" ] && mkdir "./bin"

git pull

gcc -c main.c -o ./bin/main.o

as auto_update.asm -o ./bin/git_update.o

cd bin

gcc main.o git_update.o -o eihsclubs

chmod +x ./eihsclubs

cd ..

./bin/eihsclubs

echo -e ""

read -n 1 -s -r -p "Press any ket to continue..."

echo ""
