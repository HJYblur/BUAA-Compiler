//
// Created by lemon on 2021/11/8.
//

#ifndef PROJECT5_GLOBALVAR_H
#define PROJECT5_GLOBALVAR_H

#include "lexical.h"
#include "grammar.h"
#include "error.h"
#include "midCode.h"
#include "string"
#include "vector"
#include "symbolTree.h"

using namespace std;

extern string filteredText;
extern map<int, std::string> category;
extern vector<Content> storage;
extern vector<Content> storagePlus;
extern vector<Error> errorList;
extern int globalIndex;
extern vector<Symbol> symbolTable;
extern vector<MidCode> midCodeTable;
extern SymbolTable symTable;
extern SymbolLayer* currentLayer;
extern vector<string> mipsTable;

#endif //PROJECT5_GLOBALVAR_H
