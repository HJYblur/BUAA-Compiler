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
    vector<SymbolItem> params;  //函数/数组参数

    bool isFParam = false;
    bool isGlobal = false;
    string address;

    void setSymbolType(symbolItemType t);

    void setString(string str);

    void setValue(int v);

    void setDim(int dim);

    void setDim1(int dim);

    void setDim2(int dim);

    void setAddress(string add);

    void addParam(SymbolItem para);
};

class SymbolLayer {
public:
    int index = 0;
    vector<SymbolItem> symbolLayer;

    SymbolLayer();

    void addSymbol(const SymbolItem &item);

    void print();
};

class SymbolTable {
public:
    int currentIndex = -1;
    int globalIndex = 0;
    vector<SymbolLayer> symbolTable;

    void newSymbolLayer();

    void forwardLayer();

    void backLayer();

    void print();

    bool checkExistedSymbol(AstNode *node);

    bool checkExistedSymbol(const string& s);

    SymbolItem *getExistedSymbol(AstNode *node);

    SymbolItem *getExistedSymbol(const string& s);

    void removeTmpVar();

    SymbolItem *getAllSymbol(const string &s);

    bool checkAllSymbol(const string &s);
};

bool matchType(SymbolItem *item, AstNode *node);

#endif //PROJECT5_SYMBOLTREE_H
