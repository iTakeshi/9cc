#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

void error_at(char * loc, char * fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, ""); // `pos` spaces
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(char * op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(op, token->str, token->len) != 0) {
        return false;
    }
    token = token->next;
    return true;
}

void expect(char * op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(op, token->str, token->len) != 0) {
        error_at(token->str, "Unexpected character; expected '%c'", op);
    }
    token = token->next;
}

int expect_number() {
    if (token->kind != TK_NUM) {
        error_at(token->str, "Unexpected token; expected a number");
    }
    int val = token->val;
    token = token->next;
    return val;
}

Token * new_token(TokenKind kind, Token * cur, char * str, int len) {
    Token * tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

Token * tokenize(char * p) {
    Token head;
    head.next = NULL;
    Token * cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (
                memcmp(p, "==", 2) == 0 ||
                memcmp(p, "!=", 2) == 0 ||
                memcmp(p, "<=", 2) == 0 ||
                memcmp(p, ">=", 2) == 0
        ) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (
                *p == '<' ||
                *p == '>' ||
                *p == '+' ||
                *p == '-' ||
                *p == '*' ||
                *p == '/' ||
                *p == '(' ||
                *p == ')'
        ) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0); // NOTE: len is not relevant for numbers
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "Cannot tokenize");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}
