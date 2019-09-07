#include <stdlib.h>
#include <string.h>

#include "9cc.h"

bool consume(char * str) {
    if (token->kind != TK_RESERVED || strlen(str) != token->len || memcmp(str, token->str, token->len) != 0) {
        return false;
    }
    token = token->next;
    return true;
}

Token * consume_ident() {
    if (token->kind != TK_IDENT) {
        return NULL;
    }
    Token * res = token;
    token = token->next;
    return res;
}

void expect(char * str) {
    if (token->kind != TK_RESERVED || strlen(str) != token->len || memcmp(str, token->str, token->len) != 0) {
        error_at(token->str, "Unexpected character; expected '%s'", str);
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

Local * find_var(Token * token) {
    for (Local * local = locals; local; local = local->next) {
        if (local->len == token->len && memcmp(local->name, token->str, local->len) == 0) return local;
    }
    return NULL;
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

Node * expr();  // forward declaration

Node * primary() {
    if (consume("(")) {
        Node * node = expr();
        expect(")");
        return node;
    }

    Token * token = consume_ident();
    if (token) {
        Node * node = calloc(1, sizeof(Node));
        node->kind = ND_VAR;

        Local * local = find_var(token);
        if (!local) {
            local = calloc(1, sizeof(Local));
            local->next = locals;
            local->name = token->str;
            local->len = token->len;
            if (local->next) {
                local->offset = local->next->offset + 8;
            } else {
                local->offset = 0;
            }
            locals = local;
        }
        node->offset = local->offset;

        return node;
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
    Node * node;

    if (consume("if")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        expect("(");
        node->cnd = expr();
        expect(")");
        node->thn = stmt();
        node->els = NULL;
        if (consume("else")) node->els = stmt();

    } else if (consume("while")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        expect("(");
        node->cnd = expr();
        expect(")");
        node->thn = stmt();
        node->els = NULL;

    } else if (consume("for")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        expect("(");
        node->ini = NULL;
        if (!consume(";")) {
            node->ini = expr();
            expect(";");
        }
        node->cnd = NULL;
        if (!consume(";")) {
            node->cnd = expr();
            expect(";");
        }
        node->inc = NULL;
        if (!consume(")")) {
            node->inc = expr();
            expect(")");
        }
        node->thn = stmt();
        node->els = NULL;

    } else if (consume("return")) {
        node = new_node(ND_RETURN, NULL, expr());
        expect(";");

    } else {
        node = expr();
        expect(";");
    }

    return node;
}

void parse() {
    Node * cur = NULL;
    while (!at_eof()) {
        Node * next = stmt();
        next->next = NULL;
        if (cur) {
            cur->next = next;
            cur = next;
        } else {
            // retain head
            program = next;
            cur = program;
        }
    }
}
