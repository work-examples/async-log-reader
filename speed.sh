#!/bin/bash

# grep -i "16:01 " 200m.txt
# \d\temp-one-time\logs\200m.txt

file=/d/temp-one-time/logs/200m.txt

ts1=$(date +%s%N)


./LogReader.exe "$file" "*16:01 *"

# grep -e "16:01 " "$file"  # 340ms
# grep -E "16:01 " "$file"  # 360ms
# grep  "16:01 " "$file"  # 350ms

#  -E, --extended-regexp     PATTERN is an extended regular expression
#  -F, --fixed-strings       PATTERN is a set of newline-separated strings
#  -G, --basic-regexp        PATTERN is a basic regular expression (default)
#  -P, --perl-regexp         PATTERN is a Perl regular expression
#  -e, --regexp=PATTERN      use PATTERN for matching

ts2=$(date +%s%N)

echo "Time taken: $((($ts2 - $ts1)/1000000)) ms"



