#include <stdlib.h>
#include <string.h>

#include "9cc.h"

static Token * token;
static char * user_input;
static Local * locals;

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
        error_at(user_input, token->str, "Unexpected character; expected '%s'", str);
    }
    token = token->next;
}

int expect_number() {
    if (token->kind != TK_NUM) {
        error_at(user_input, token->str, "Unexpected token; expected a number");
    }
    int val = token->val;
    token = token->next;
    return val;
}

char * expect_ident() {
    if (token->kind != TK_IDENT) {
        error_at(user_input, token->str, "Unexpected token; expected a identifier");
    }
    char * str = strndup(token->str, token->len);
    token = token->next;
    return str;
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

// forward declarations
Node * expr();
Node * assign();

/*
 * call_args ::= assign*
 */
Node * call_args() {
    if (consume(")")) return NULL;

    Node * head = assign();
    head->next = NULL;
    Node * cur = head;
    while (consume(",")) {
        Node * next = assign();
        next->next = NULL;
        cur->next = next;
        cur = next;
    }
    expect(")");

    return head;
}

/*
 * primary ::= "(" expr ")"
 *           | ident ("(" call_args ")")?
 *           | num
 */
Node * primary() {
    if (consume("(")) {
        Node * node = expr();
        expect(")");
        return node;
    }

    Token * token = consume_ident();
    if (token) {
        if (consume("(")) {
            // function call
            Node * node = calloc(1, sizeof(Node));
            node->kind = ND_CALL;
            node->name = strndup(token->str, token->len);
            node->args = call_args();
            return node;

        } else {
            // variable
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
                    local->offset = 8;
                }
                locals = local;
            }
            node->offset = local->offset;

            return node;
        }
    }

    return new_node_num(expect_number());
}

/*
 * postfix ::= postfix ("++" | "--")
 *           | primary
 */
Node * postfix() {
    Node * node = primary();

    while (true) {
        if (consume("++")) {
            node = new_node(ND_PST_INC, node, NULL);
        } else if (consume("--")) {
            node = new_node(ND_PST_DEC, node, NULL);
        } else {
            return node;
        }
    }
}

/*
 * unary ::= ("++" | "--") unary
 *         | ("+" | "-") unary
 *         | postfix
 */
Node * unary() {
    if (consume("++")) {
        return new_node(ND_PRE_INC, NULL, unary());
    } else if (consume("--")) {
        return new_node(ND_PRE_DEC, NULL, unary());
    } else if (consume("+")) {
        return unary();
    } else if (consume("-")) {
        return new_node(ND_SUB, new_node_num(0), unary());
    } else {
        return postfix();
    }
}

/*
 * mul ::= unary (("*" | "/") unary)*
 */
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

/*
 * add ::= mul (("+" | "-") mul)*
 */
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

/*
 * relational ::= add (("<" | ">" | "<=" | ">=") add)*
 */
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

/*
 * equality ::= relational (("==" | "!=") relational)*
 */
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

/*
 * assign ::= equality (("=" | "+=" | "-=" | "*=" | "/=") assign)?
 */
Node * assign() {
    Node * node = equality();

    if (consume("=")) {
        return new_node(ND_ASSIGN, node, assign());
    } else if (consume("+=")) {
        return new_node(ND_ASSIGN, node, new_node(ND_ADD, node, assign()));
    } else if (consume("-=")) {
        return new_node(ND_ASSIGN, node, new_node(ND_SUB, node, assign()));
    } else if (consume("*=")) {
        return new_node(ND_ASSIGN, node, new_node(ND_MUL, node, assign()));
    } else if (consume("/=")) {
        return new_node(ND_ASSIGN, node, new_node(ND_DIV, node, assign()));
    } else {
        return node;
    }
}

/*
 * expr ::= assign
 */
Node * expr() {
    Node * node = assign();
    return node;
}

/*
 * stmt ::= block
 *        | "if" "(" expr ")" stmt ("else" stmt)?
 *        | "while" "(" expr ")" stmt
 *        | "for" "(" expr? ";" expr? ";" expr" ")" stmt
 *        | "return" expr ";"
 *        | expr ";"
 */
Node * stmt() {
    Node * node;

    if (consume("{")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        Node * cur = NULL;
        while (!consume("}")) {
            Node * next = stmt();
            next->next = NULL;
            if (cur) {
                cur->next = next;
                cur = next;
            } else {
                // retain head
                node->child = next;
                cur = node->child;
            }
        }

    } else if (consume("if")) {
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
        node = new_node(ND_EXPR_STMT, NULL, expr());
        expect(";");
    }

    return node;
}

/*
 * function ::= ident "(" param* ")" "{" stmt* "}"
 */
Function * function() {
    locals = NULL;

    Function * fn = calloc(1, sizeof(Function));
    fn->name = expect_ident();

    expect("(");
    expect(")");
    expect("{");

    Node * cur = NULL;
    while (!consume("}")) {
        Node * next = stmt();
        next->next = NULL;
        if (cur) {
            cur->next = next;
            cur = next;
        } else {
            // retain head
            fn->body = next;
            cur = next;
        }
    }

    fn->locals = locals;
    return fn;
}

/*
 * program ::= function*
 */
Function * parse(Token * _token, char * _user_input) {
    token = _token;
    user_input = _user_input;

    Function * head;
    Function * cur = NULL;
    while (!at_eof()) {
        Function * next = function();
        next->next = NULL;
        if (cur) {
            cur->next = next;
            cur = next;
        } else {
            // retain head
            head = next;
            cur = next;
        }
    }

    return head;
}
