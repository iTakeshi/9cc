#!/bin/bash

try() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

echo "===== Arithmetic Operators ====="
try 0 0
try 42 42
try 21 "5+20-4"
try 21 "5 + 20 - 4"
try 87 "3 * 5 * 4 + 20 - 6 / 2 + 10"
try 40 "3 * 5 * (4 + 1 - 6 / 2) + 10"
try 10 "-10 + 20"
try 40 "50 + 2 + -3 * 4"
try 4 "2 * -(3 - 5)"

echo "===== Relational Operators ====="
try 0 "1 == 2"
try 1 "1 == 1"
try 0 "1 == 0"
try 1 "1 != 2"
try 0 "1 != 1"
try 1 "1 != 0"
try 1 "1 < 2"
try 0 "1 < 1"
try 0 "1 < 0"
try 1 "1 <= 2"
try 1 "1 <= 1"
try 0 "1 <= 0"
try 0 "1 > 2"
try 0 "1 > 1"
try 1 "1 > 0"
try 0 "1 >= 2"
try 1 "1 >= 1"
try 1 "1 >= 0"
try 0 "1 == 1 + 1" # deduced to 1 == 2

echo "OK"
