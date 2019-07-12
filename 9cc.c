#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TK_RESERVED, // symbols
    TK_NUM,      // integers
    TK_EOF,      // EOF
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token * next;
    int val;
    char * str;
};

Token * token;
char * user_input;

void error(char * fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

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

bool consume(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) {
        return false;
    }
    token = token->next;
    return true;
}

void expect(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) {
        error_at(token->str, "Unexpected character; expected '%c'", op);
    }
    token = token->next;
}

int expect_number() {
    if (token->kind != TK_NUM) {
        error_at(token->str, "Unexpected toekn; expected a number");
    }
    int val = token->val;
    token = token->next;
    return val;
}

Token * new_token(TokenKind kind, Token * cur, char * str) {
    Token * tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
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

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/') {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "Cannot tokenize");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node * lhs;
    Node * rhs;
    int val;
};

Node * new_node(NodeKind kind, Node * lhs, Node * rhs) {
    Node * node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node * new_node_num(int val) {
    Node * node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node * mul() {
    Node * node = new_node_num(expect_number());

    while (true) {
        if (consume('*')) {
            node = new_node(ND_MUL, node, mul());
        } else if (consume('/')) {
            node = new_node(ND_DIV, node, mul());
        } else {
            return node;
        }
    }
}

Node * expr() {
    Node * node = mul();

    while (true) {
        if (consume('+')) {
            node = new_node(ND_ADD, node, mul());
        } else if (consume('-')) {
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

void gen(Node *node) {
    if (node->kind == ND_NUM) {
        printf("  push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
        case ND_ADD:
            printf("  add rax, rdi\n");
            break;
        case ND_SUB:
            printf("  sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("  imul rax, rdi\n");
            break;
        case ND_DIV:
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;
    }

    printf("  push rax\n");
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
