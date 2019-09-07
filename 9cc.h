#include <stdbool.h>

typedef enum {
    TK_RESERVED, // symbols
    TK_IDENT,    // identifier
    TK_NUM,      // integers
    TK_EOF,      // EOF
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token * next;
    int val;
    char * str;
    size_t len;
};

typedef enum {
    ND_ASSIGN,
    ND_VAR,
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
    int offset;
};

// gloval variables
extern Token * token;
extern char * user_input;
extern Node * code[100];

// tokenize.c
void tokenize();

// parse.c
void program();

// codegen.c
void gen(Node * node);

// util.c
void error(char * fmt, ...);
void error_at(char * loc, char * fmt, ...);
