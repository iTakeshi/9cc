#include <stdbool.h>

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
    int len;
};

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_EQ,
    ND_NE,
    ND_LT,
    ND_LE,
    ND_NUM,
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node * lhs;
    Node * rhs;
    int val;
};

// gloval variables
extern Token * token;
extern char * user_input;

// tokenize.c
Token * tokenize(char * p);
bool consume();
void expect();
int expect_number();

// parse.c
Node * expr();

// codegen.c
void gen(Node * node);
