//
// Created by lemon on 2021/11/14.
//

#ifndef PROJECT5_SYMBOLTREE_H
#define PROJECT5_SYMBOLTREE_H

#include "vector"
#include "string"
#include "lexical.h"
#include "ast.h"

using namespace std;
enum symbolItemType {
    VarSymbol, ConstSymbol, IntFuncSymbol, VoidFuncSymbol, ArraySymbol,
};

class SymbolItem {

public:
    SymbolItem(AstNode node);

    SymbolItem(symbolItemType t);

    SymbolItem(symbolItemType t, AstNode node);

    symbolItemType type;        //符号类型
    string str;                 //内容
    int value = 0;              //常量的值
    int dim = 0;                //数组维数/函数参数个数
    int dim1 = 0, dim2 = 0;      //数组两个维数的值
    int layer = 0;
    vector<SymbolItem> params;  //函数/数组参数

    bool isFParam = false;
    bool isGlobal = false;
    bool isTmpVar = false;

    void setSymbolType(symbolItemType t);

    void setString(string str);

    void setValue(int v);

    void setDim(int dim);

    void setDim1(int dim);

    void setDim2(int dim);

    void addParam(SymbolItem &para);

    void setIsTmp();
};

#endif //PROJECT5_SYMBOLTREE_H
