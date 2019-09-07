#include <stdlib.h>
#include <string.h>

#include "9cc.h"

bool consume(char * op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(op, token->str, token->len) != 0) {
        return false;
    }
    token = token->next;
    return true;
}

char * consume_ident() {
    if (token->kind != TK_IDENT) {
        return NULL;
    }
    char * res = token->str;
    token = token->next;
    return res;
}

void expect(char * op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(op, token->str, token->len) != 0) {
        error_at(token->str, "Unexpected character; expected '%s'", op);
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

bool at_eof() {
    return token->kind == TK_EOF;
}

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

Node * new_var_node(char * name) {
    Node * node = calloc(1, sizeof(Node));
    node->kind = ND_VAR;
    node->offset = (name[0] - 'a' + 1) * 8;
    return node;
}

Node * expr();  // forward declaration

Node * primary() {
    if (consume("(")) {
        Node * node = expr();
        expect(")");
        return node;
    }

    char * name = consume_ident();
    if (name) {
        return new_var_node(name);
    }

    return new_node_num(expect_number());
}

Node * unary() {
    if (consume("+")) {
        return primary();
    } else if (consume("-")) {
        return new_node(ND_SUB, new_node_num(0), primary());
    } else {
        return primary();
    }
}

Node * mul() {
    Node * node = unary();

    while (true) {
        if (consume("*")) {
            node = new_node(ND_MUL, node, unary());
        } else if (consume("/")) {
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

Node * add() {
    Node * node = mul();

    while (true) {
        if (consume("+")) {
            node = new_node(ND_ADD, node, mul());
        } else if (consume("-")) {
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

Node * relational() {
    Node * node = add();

    while (true) {
        if (consume("<")) {
            node = new_node(ND_LT, node, add());
        } else if (consume("<=")) {
            node = new_node(ND_LE, node, add());
        } else if (consume(">")) {
            node = new_node(ND_LT, add(), node);
        } else if (consume(">=")) {
            node = new_node(ND_LE, add(), node);
        } else {
            return node;
        }
    }
}

Node * equality() {
    Node * node = relational();

    while (true) {
        if (consume("==")) {
            node = new_node(ND_EQ, node, relational());
        } else if (consume("!=")) {
            node = new_node(ND_NE, node, relational());
        } else {
            return node;
        }
    }
}

Node * assign() {
    Node * node = equality();

    if (consume("=")) {
        return new_node(ND_ASSIGN, node, assign());
    } else {
        return node;
    }
}

Node * expr() {
    Node * node = assign();
    return node;
}

Node * stmt() {
    Node * node = expr();
    expect(";");
    return node;
}

void program() {
    for (int i = 0; !at_eof(); i++) {
        code[i] = stmt();
    }
}
