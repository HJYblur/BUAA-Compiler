#include <iostream>
#include "grammar.h"
#include "ast.h"
#include "globalVar.h"
#include "mips.h"

int main() {
    lexicalScan();
    AstNode root = parseAst();
    cout << "------------------parse over.------------------"<< endl;
    postOrderTraversal(root);
    cout << "-----------traverse and to MidCode over.-------"<< endl;
    getMips();
    cout << "----------------to Mips over.------------------"<< endl;
    return 0;
}
