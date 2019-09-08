#include "9cc.h"

Local * locals = NULL;

int main(int argc, char ** argv) {
    if (argc != 2) {
        error("Wrong number of arguments.");
    }

    Token * token = tokenize(argv[1]);
    Function * program = parse(token, argv[1]);
    codegen(program);

    return 0;
}
