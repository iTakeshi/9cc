#!/bin/bash

cat <<EOF | gcc -xc -c -o test_mock.o -
#include <stdio.h>
long fn_arg0_0() { return 1; }
long fn_arg0_1() { return 42; }
long fn_arg1_0(long a) { return a; }
long fn_arg1_1(long a) { return a + 10; }
long fn_arg2_0(long a, long b) { return a + b; }
long fn_arg3_0(long a, long b, long c) { return a + b + c; }
long fn_arg4_0(long a, long b, long c, long d) { return a + b + c + d; }
long fn_arg5_0(long a, long b, long c, long d, long e) { return a + b + c + d + e; }
long fn_arg6_0(long a, long b, long c, long d, long e, long f) { return a + b + c + d + e + f; }
long fn_arg6_1(long a, long b, long c, long d, long e, long f) { return a; }
long fn_arg6_2(long a, long b, long c, long d, long e, long f) { return b; }
long fn_arg6_3(long a, long b, long c, long d, long e, long f) { return c; }
long fn_arg6_4(long a, long b, long c, long d, long e, long f) { return d; }
long fn_arg6_5(long a, long b, long c, long d, long e, long f) { return e; }
long fn_arg6_6(long a, long b, long c, long d, long e, long f) { return f; }
EOF

try() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    gcc -g -o tmp tmp.s test_mock.o
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
try  0 "main() { return 0; }"
try 42 "main() { return 42; }"
try  0 "main() { 42; }"
try 21 "main() { return 5+20-4; }"
try 21 "main() { return 5 + 20 - 4; }"
try 87 "main() { return 3 * 5 * 4 + 20 - 6 / 2 + 10; }"
try 40 "main() { return 3 * 5 * (4 + 1 - 6 / 2) + 10; }"
try 10 "main() { return -10 + 20; }"
try 40 "main() { return 50 + 2 + -3 * 4; }"
try  4 "main() { return 2 * -(3 - 5); }"

echo "===== Relational Operators ====="
try 0 "main() { return 1 == 2; }"
try 1 "main() { return 1 == 1; }"
try 0 "main() { return 1 == 0; }"
try 1 "main() { return 1 != 2; }"
try 0 "main() { return 1 != 1; }"
try 1 "main() { return 1 != 0; }"
try 1 "main() { return 1 < 2; }"
try 0 "main() { return 1 < 1; }"
try 0 "main() { return 1 < 0; }"
try 1 "main() { return 1 <= 2; }"
try 1 "main() { return 1 <= 1; }"
try 0 "main() { return 1 <= 0; }"
try 0 "main() { return 1 > 2; }"
try 0 "main() { return 1 > 1; }"
try 1 "main() { return 1 > 0; }"
try 0 "main() { return 1 >= 2; }"
try 1 "main() { return 1 >= 1; }"
try 1 "main() { return 1 >= 0; }"
try 0 "main() { return 1 == 1 + 1; }" # deduced to 1 == 2

echo "===== Multiple Statements with Variables ====="
try  3 "main() { 1; 2; return 3; }"
try 14 "main() { a = 3; b = 5 * 6 - 8; return a + b / 2; }"
try 14 "main() { foo = 3; bar = 5 * 6 - 8; return foo + bar / 2; }"
try  1 "main() { a = 0; return ++a; }"
try  0 "main() { a = 0; return a++; }"
try  1 "main() { a = 0; a++; return a; }"
try  0 "main() { a = 1; return --a; }"
try  1 "main() { a = 1; return a--; }"
try  0 "main() { a = 1; a--; return a; }"
try  1 "main() { a = 0; a += 1; return a; }"
try  0 "main() { a = 1; a -= 1; return a; }"
try  3 "main() { a = 1; a *= 3; return a; }"
try  1 "main() { a = 2; a /= 2; return a; }"

echo "===== Control Statement ====="
try  2 "main() { 1; return 2; 3; }"
try 22 "main() { foo = 3; return bar = 5 * 6 - 8; return foo + bar / 2; }"
try 10 "main() { if (1) return 10; }"
try  0 "main() { if (0) return 10; }"
try 10 "main() { if (1) return 10; else return 20; }"
try 20 "main() { if (0) return 10; else return 20; }"
try 11 "main() { a = 1; if (a == 1) return b = a * 10 + 1; }"
try 10 "main() { i = 0; while (i < 10) i = i + 1; return i; }"
try 55 "main() { sum = 0; for (i = 1; i <= 10; i = i + 1) sum = sum + i; return sum; }"
try 42 "main() { for (;;) return 42; return 0; }"
try 55 "main() { i = 1; sum = 0; while (i <= 10) { sum = sum + i; i = i + 1; } return sum; }"

echo "===== Function ====="
try  0 "main() { fn_arg0_0(); }"
try  1 "main() { return fn_arg0_0(); }"
try 45 "main() { return fn_arg0_1() + 3; }"
try  1 "main() { return fn_arg1_0(1); }"
try 11 "main() { return fn_arg1_1(1); }"
try  3 "main() { return fn_arg2_0(1, 2); }"
try  6 "main() { return fn_arg3_0(1, 2, 3); }"
try 10 "main() { return fn_arg4_0(1, 2, 3, 4); }"
try 15 "main() { return fn_arg5_0(1, 2, 3, 4, 5); }"
try 21 "main() { return fn_arg6_0(1, 2, 3, 4, 5, 6); }"
try  1 "main() { return fn_arg6_1(1, 2, 3, 4, 5, 6); }"
try  2 "main() { return fn_arg6_2(1, 2, 3, 4, 5, 6); }"
try  3 "main() { return fn_arg6_3(1, 2, 3, 4, 5, 6); }"
try  4 "main() { return fn_arg6_4(1, 2, 3, 4, 5, 6); }"
try  5 "main() { return fn_arg6_5(1, 2, 3, 4, 5, 6); }"
try  6 "main() { return fn_arg6_6(1, 2, 3, 4, 5, 6); }"
try  0 "main() { fn_arg6_6(1, 2, 3, 4, 5, 6); }"

echo "===== Function Definition ====="
try 42 "main() { return ret42(); } ret42() { return 42; }"

echo "OK"
