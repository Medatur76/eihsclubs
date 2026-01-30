#!/bin/bash

[ ! -d "./bin" ] && mkdir "./bin"

gcc main.c -o ./bin/main

chmod +x ./bin/main

./bin/main

echo -e ""