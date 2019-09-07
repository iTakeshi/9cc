#include "9cc.h"

Token * token;
char * user_input;
Node * code[100] = {};
Local * locals = NULL;

int main(int argc, char ** argv) {
    if (argc != 2) {
        error("Wrong number of arguments.");
    }

    user_input = argv[1];
    tokenize();
    program();
    codegen();

    return 0;
}
