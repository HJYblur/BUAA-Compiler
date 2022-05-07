//
// Created by lemon on 2021/11/11.
//

#ifndef PROJECT5_MIDCODE_H
#define PROJECT5_MIDCODE_H

#include "ast.h"
#include "vector"
#include "symbolTree.h"

using namespace std;

void postOrderTraversal(AstNode root);

string getEnum(NodeType type);

void addSymbol(SymbolItem item);

void printAstNode(AstNode *tmp);

enum MidCodeType {
    ConstDeclMid, VarDeclMid, ArrayDeclMid, ArrayVarMid, ArrayGetMid,
    FuncDefMid, FuncParamMid, PushVar, FuncCallMid, FuncEndMid, MainFuncMid, BlockBegin, BlockEnd,
    ExpMid, PrintfMid, GetIntMid, Label, Goto, Cmp, Return, Compare,
};

class MidCode {
public:
    MidCodeType type;
    int opNum = 2;
    string op1;
    string op2;
    string opRand = "";
    string result;
    int paraNumber = 0;
    int resultType = 0;//0:number 1:exp
    //0:left number 1:right number 2:both exp

    int dim1 = 0, dim2 = 0;

    explicit MidCode(MidCodeType t);

    MidCode(MidCodeType t, int num);

    string toString() const;

    void setType(MidCodeType type);

    void setOpNum(int num);

    void setOp1(string s);

    void setOp2(string s);

    void setOpRand(string s);

    void setResult(string s);

    void setResultType(int type);//0 int,1 void

};

void toMidCode(AstNode *node);

void pushMidCode(const MidCode &m);

void constDeclMid(AstNode *node);

void varDeclMid(AstNode *node);

void funcDefMid(AstNode *node);

void funcParamMid(AstNode *node, SymbolItem *item);

void blockMid(AstNode *node);

void assignMid(AstNode *node);

void ifMid(AstNode *node);

void whileMid(AstNode *node);

void breakMid(AstNode *node);

void continueMid(AstNode *node);

void returnMid(AstNode *node);

void printfMid(AstNode *node);

bool condMid(AstNode *node, const string &ifName, const string &elseName, bool flag);

bool condInitMid(AstNode *node, const string &ifName, const string &elseName, bool flag);

bool lOrMid(AstNode *node, const string &ifBody, const string &elseBody);

bool lAndMid(AstNode *node, const string &ifName, const string &elseName);

bool eqMid(AstNode *node, const string &ifName, const string &elseName, bool flag);

bool relMid(AstNode *node, const string &ifName, const string &elseName, bool flag);

AstNode parseRelMid(AstNode *node, const string &ifName, const string &elseName, bool flag);

bool zeroMid(AstNode *node, const string &ifName, const string &elseName, bool flag);

AstNode expMid(AstNode *node);

AstNode expItemMid(AstNode *node);

AstNode addExpMid(AstNode *node);

AstNode mulExpMid(AstNode *node);

AstNode unaryExpMid(AstNode *node);

void printMidCode();

AstNode initialMid(AstNode *node);

AstNode getAddress(AstNode *x);

AstNode getAddress(AstNode *x, AstNode *y, int dim2);

string getTmpVar();

bool checkExistedSymbol(const string &name);

SymbolItem *getExistedSymbol(const string &name);

#endif //PROJECT5_MIDCODE_H
