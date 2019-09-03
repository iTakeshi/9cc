#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "9cc.h"

Token * token;
char * user_input;

void error(char * fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

int main(int argc, char ** argv) {
    if (argc != 2) {
        error("Wrong number of arguments.");
    }

    user_input = argv[1];
    token = tokenize(argv[1]);
    Node * node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    gen(node);

    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}
