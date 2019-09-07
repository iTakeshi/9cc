#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

bool isalpha_(char c) {
    return isalpha(c) || c == '_';
}

bool isalnum_(char c) {
    return isalnum(c) || c == '_';
}

Token * new_token(TokenKind kind, Token * cur, char * str, size_t len) {
    Token * tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

void tokenize() {
    Token head;
    head.next = NULL;
    Token * cur = &head;

    char * p = user_input;  // retain user_input for display in error_at
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
                *p == ')' ||
                *p == '=' ||
                *p == ';'
        ) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (strncmp(p, "return", 6) == 0 && !isalnum_(p[6])) {
            cur = new_token(TK_RESERVED, cur, p, 6);
            p += 6;
            continue;
        }

        if (isalpha_(*p)) {
            char * begin = p;
            while (isalnum_(*p)) p++;
            cur = new_token(TK_IDENT, cur, begin, p - begin);
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

    token = head.next;
}
