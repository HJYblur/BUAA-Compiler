//
// Created by lemon on 2021/10/30.
//

#ifndef PROJECT5_SYMBOL_H
#define PROJECT5_SYMBOL_H

#include "vector"
#include "string"
#include "lexical.h"

using namespace std;
enum symbolType {
    var, constVar, func, returnParam, null
};
enum bType {
    intType, voidType, unknown
};

class Symbol {

public:
    symbolType type = null;     //符号类型
    string str;                 //内容
    int index;                  //层次
    int dim = 0;                //数组维数
    vector<Symbol> params;      //函数/数组参数
    bType btype = unknown;      //变量类型
    bType returnType = unknown; //函数返回值

    Symbol();

    Symbol(symbolType t);

    Symbol(symbolType t, std::string str);

    Symbol(symbolType t, std::string str, bType bt);

    void addParam(const Symbol &s);

    void setDim(int d);

    void setIndex(int d);

    void setReturnType(bType type);

    void setBType(bType btype);
};

void printSymbolTable();

void popAllSymbol();

#endif //PROJECT5_SYMBOL_H
