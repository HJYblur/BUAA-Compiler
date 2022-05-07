#include <iostream>
#include "grammar.h"
#include "ast.h"
#include "globalVar.h"
#include "mips.h"

#define DEBUG 0

int main() {
    lexicalScan();
    AstNode root = parseAst();
    if (DEBUG) {
        cout << "------------------parse over.------------------" << endl;
    }
    postOrderTraversal(root);
    if (DEBUG) {
        cout << "-----------traverse and to MidCode over.-------" << endl;
    }
    getMips();
    if (DEBUG) {
        cout << "----------------to Mips over.------------------" << endl;
    }
    return 0;
}
