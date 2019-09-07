#include <stdbool.h>
#include <stddef.h>

typedef enum {
    TK_RESERVED,
    TK_IDENT,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token * next;
    int val;
    char * str;
    size_t len;
};

typedef struct Local Local;

struct Local {
    Local * next;
    char * name;
    size_t len;
    size_t offset;
};

typedef enum {
    ND_IF,
    ND_RETURN,
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

    Node * cnd;
    Node * thn;
    Node * els;

    int val;
    size_t offset;
};

// gloval variables
extern Token * token;
extern char * user_input;
extern Node * code[100];
extern Local * locals;

// tokenize.c
void tokenize();

// parse.c
void program();

// codegen.c
void codegen();

// util.c
void error(char * fmt, ...);
void error_at(char * loc, char * fmt, ...);
