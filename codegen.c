#include <stdio.h>

#include "9cc.h"

void gen_node(Node * node);  // forward declaration

void gen_lval(Node * node) {
    if (node->kind != ND_VAR) error("lvalue is not a variable");

    printf("  mov rax, rbp\n");
    printf("  sub rax, %ld\n", node->offset);
    printf("  push rax\n");
}

void gen_binop(Node * node) {
    if (
            node->kind != ND_ADD &&
            node->kind != ND_SUB &&
            node->kind != ND_MUL &&
            node->kind != ND_DIV &&
            node->kind != ND_EQ &&
            node->kind != ND_NE &&
            node->kind != ND_LT &&
            node->kind != ND_LE
    ) error("node is not a binary operation");

    gen_node(node->lhs);
    gen_node(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
}

void gen_node(Node * node) {
    static size_t id = 0;

    switch (node->kind) {
        case ND_BLOCK:
            for (Node * statement = node->child; statement; statement = statement->next)
                gen_node(statement);
            return;

        case ND_IF:
            id++;
            gen_node(node->cnd);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  .Lfalse%ld\n", id);
            gen_node(node->thn);
            printf("  jmp .Lend%ld\n", id);
            printf(".Lfalse%ld:\n", id);
            if (node->els) gen_node(node->els);
            printf(".Lend%ld:\n", id);
            return;

        case ND_WHILE:
            id++;
            printf(".Lbegin%ld:\n", id);
            gen_node(node->cnd);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  .Lend%ld\n", id);
            gen_node(node->thn);
            printf("  jmp .Lbegin%ld\n", id);
            printf(".Lend%ld:\n", id);
            return;

        case ND_FOR:
            id++;
            if (node->ini) gen_node(node->ini);
            printf(".Lbegin%ld:\n", id);
            if (node->cnd) {
                gen_node(node->cnd);
                printf("  pop rax\n");
                printf("  cmp rax, 0\n");
                printf("  je  .Lend%ld\n", id);
            }
            gen_node(node->thn);
            if (node->inc) gen_node(node->inc);
            printf("  jmp .Lbegin%ld\n", id);
            printf(".Lend%ld:\n", id);
            return;

        case ND_RETURN:
            gen_node(node->rhs);
            printf("  pop rax\n");
            printf("  mov rsp, rbp\n");
            printf("  pop rbp\n");
            printf("  ret\n");
            return;

        case ND_EXPR_STMT:
            gen_node(node->rhs);
            printf("  pop rax\n");
            return;

        case ND_ASSIGN:
            gen_lval(node->lhs);
            gen_node(node->rhs);
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mov [rax], rdi\n");
            printf("  push rdi\n");
            return;

        case ND_VAR:
            gen_lval(node);
            printf("  pop rax\n");
            printf("  mov rax, [rax]\n");
            printf("  push rax\n");
            return;

        case ND_CALL: {
            static char * reg[6] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };
            int i = 0;
            for (Node * arg = node->args; arg; arg = arg->next, i++) {
                if (i >= 6) error("Arguments more than 6 are not supported");
                gen_node(arg);
            }
            for (int j = i - 1; j >= 0; j--) printf("  pop %s\n", reg[j]);

            printf("  and rsp, 0xfffffffffffffff0\n");  // align rsp to 16-byte boundary
            printf("  call %s\n", node->name);
            printf("  push rax\n");
            return;
        }

        case ND_NUM:
            printf("  push %d\n", node->val);
            return;

        case ND_PRE_INC:
            gen_lval(node->rhs);
            printf("  mov rax, [rax]\n");
            printf("  add rax, 1\n");
            printf("  push rax\n");
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mov [rax], rdi\n");
            printf("  push rdi\n");
            return;

        case ND_PRE_DEC:
            gen_lval(node->rhs);
            printf("  mov rax, [rax]\n");
            printf("  sub rax, 1\n");
            printf("  push rax\n");
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mov [rax], rdi\n");
            printf("  push rdi\n");
            return;

        case ND_PST_INC:
            gen_lval(node->lhs);
            printf("  push [rax]\n");
            printf("  mov rax, [rax]\n");
            printf("  add rax, 1\n");
            printf("  push rax\n");
            printf("  pop rdi\n");
            printf("  pop rsi\n");
            printf("  pop rax\n");
            printf("  mov [rax], rdi\n");
            printf("  push rsi\n");
            return;

        case ND_PST_DEC:
            gen_lval(node->lhs);
            printf("  push [rax]\n");
            printf("  mov rax, [rax]\n");
            printf("  sub rax, 1\n");
            printf("  push rax\n");
            printf("  pop rdi\n");
            printf("  pop rsi\n");
            printf("  pop rax\n");
            printf("  mov [rax], rdi\n");
            printf("  push rsi\n");
            return;

        case ND_ADD:
            gen_binop(node);
            printf("  add rax, rdi\n");
            printf("  push rax\n");
            return;

        case ND_SUB:
            gen_binop(node);
            printf("  sub rax, rdi\n");
            printf("  push rax\n");
            return;

        case ND_MUL:
            gen_binop(node);
            printf("  imul rax, rdi\n");
            printf("  push rax\n");
            return;

        case ND_DIV:
            gen_binop(node);
            printf("  cqo\n");
            printf("  idiv rdi\n");
            printf("  push rax\n");
            return;

        case ND_EQ:
            gen_binop(node);
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzb rax, al\n");
            printf("  push rax\n");
            return;

        case ND_NE:
            gen_binop(node);
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            printf("  push rax\n");
            return;

        case ND_LT:
            gen_binop(node);
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            printf("  push rax\n");
            return;

        case ND_LE:
            gen_binop(node);
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
            printf("  push rax\n");
            return;
    }
}

size_t calc_stack_size() {
    size_t count = 0;
    for (Local * local = locals; local; local = local->next) count++;
    return count * 8;
}

void codegen() {
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %ld\n", calc_stack_size());

    for (Node * statement = program; statement; statement = statement->next)
        gen_node(statement);

    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}
