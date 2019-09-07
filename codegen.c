#include <stdio.h>

#include "9cc.h"

void gen_lval(Node * node) {
    if (node->kind != ND_VAR) error("lvalue is not a variable");

    printf("  mov rax, rbp\n");
    printf("  sub rax, %ld\n", node->offset);
    printf("  push rax\n");
}

void gen_node(Node * node) {
    switch (node->kind) {
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

        case ND_NUM:
            printf("  push %d\n", node->val);
            return;

        default:
            break;
    }

    gen_node(node->lhs);
    gen_node(node->rhs);

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

        case ND_EQ:
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzb rax, al\n");
            break;

        case ND_NE:
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            break;

        case ND_LT:
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            break;

        case ND_LE:
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
            break;

        default:
            break;
    }

    printf("  push rax\n");
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

    for (int i = 0; code[i]; i++) {
        gen_node(code[i]);
        printf("  pop rax\n");
    }

    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}
