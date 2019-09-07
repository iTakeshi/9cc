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
try 0 "0;"
try 42 "42;"
try 21 "5+20-4;"
try 21 "5 + 20 - 4;"
try 87 "3 * 5 * 4 + 20 - 6 / 2 + 10;"
try 40 "3 * 5 * (4 + 1 - 6 / 2) + 10;"
try 10 "-10 + 20;"
try 40 "50 + 2 + -3 * 4;"
try 4 "2 * -(3 - 5);"

echo "===== Relational Operators ====="
try 0 "1 == 2;"
try 1 "1 == 1;"
try 0 "1 == 0;"
try 1 "1 != 2;"
try 0 "1 != 1;"
try 1 "1 != 0;"
try 1 "1 < 2;"
try 0 "1 < 1;"
try 0 "1 < 0;"
try 1 "1 <= 2;"
try 1 "1 <= 1;"
try 0 "1 <= 0;"
try 0 "1 > 2;"
try 0 "1 > 1;"
try 1 "1 > 0;"
try 0 "1 >= 2;"
try 1 "1 >= 1;"
try 1 "1 >= 0;"
try 0 "1 == 1 + 1;" # deduced to 1 == 2

echo "===== Multiple Statements with Variables ====="
try 3 "1; 2; 3;"
try 14 "a = 3; b = 5 * 6 - 8; a + b / 2;"
try 14 "foo = 3; bar = 5 * 6 - 8; foo + bar / 2;"
try 1 "a = 0; ++a;"
try 0 "a = 0; a++;"
try 1 "a = 0; a++; a;"
try 0 "a = 1; --a;"
try 1 "a = 1; a--;"
try 0 "a = 1; a--; a;"

echo "===== Control Statement ====="
try 2 "1; return 2; 3;"
try 22 "foo = 3; return bar = 5 * 6 - 8; return foo + bar / 2;"
try 10 "if (1) 10;"
try 10 "if (1) 10; else 20;"
try 20 "if (0) 10; else 20;"
try 11 "a = 1; if (a == 1) return b = a * 10 + 1;"
try 10 "i = 0; while (i < 10) i = i + 1; return i;"
try 55 "sum = 0; for (i = 1; i <= 10; i = i + 1) sum = sum + i; return sum;"
try 42 "for (;;) return 42; return 0;"
try 55 "i = 1; sum = 0; while (i <= 10) { sum = sum + i; i = i + 1; } return sum;"

echo "OK"
