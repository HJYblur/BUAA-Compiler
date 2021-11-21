//
// Created by lemon on 2021/10/9.
//

#ifndef PROJECT5_GRAMMAR_H
#define PROJECT5_GRAMMAR_H

#include "symbol.h"

int grammarScan();

void initGrammarCategory();

void push(int op);

bool getNextToken();

bool stepBack();

bool match(int op);

bool compUnit();

bool decl();

bool constDecl();

bool bType();

bool constDef();

bool constInitVal();

bool varDecl();

bool varDef();

bool initVal();

bool funcDef();

bool mainFuncDef();

bool funcType();

bool funcFParams(Symbol *sym);

bool funcFParam(Symbol *sym);

bool block(Symbol *sym);

bool blockItem(Symbol *sym);

bool stmt(Symbol *sym);

int checkFormatString(const Content &c);

bool exp();

bool cond();

bool lVal();

bool primaryExp();

bool number();

bool unaryExp();

bool unaryOp();

bool funcRParams();

bool mulExp();

bool addExp();

bool relExp();

bool eqExp();

bool lAndExp();

bool lOrExp();

bool constExp();

bool checkMultiIdent(Symbol symbol,int line,int index);

bool checkIsConst();

bool checkNoIdent(Symbol symbol);

bool checkFuncParams(const Content &one);

bool checkParam(Symbol s, Content c);

bool checkReturn(Symbol father, Symbol son, int line);

void checkParams(std::vector<Symbol> params, int line);

void print(std::ofstream &file, std::vector<Content> &store);

#endif //PROJECT3_GRAMMAR_H
