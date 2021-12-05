#!/bin/bash

file=/d/temp-one-time/logs/2000m.txt


ts1=$(date +%s%N)
# ==================================

./Release-x64/LogReader.exe "$file" "*16:01 *"

# ./Release-Win32/LogReader.exe "$file" "*16:01 *"

# grep --line-regexp --basic-regexp -e ".*16:01 .*" "$file"

# ==================================
ts2=$(date +%s%N)

echo "Time taken: $((($ts2 - $ts1)/1000000)) ms"



