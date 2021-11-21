//
// Created by lemon on 2021/11/8.
//

#ifndef PROJECT5_AST_H
#define PROJECT5_AST_H

#include "string"
#include "vector"
#include "lexical.h"


using namespace std;

enum NodeType {
    ConstDeclList, VarDeclList, FuncDefList, CompUnit, ConstDecl, ConstInitVal, VarDecl, InitVal,
    FuncDef, MainFuncDef, FuncFParam, Block, Stmt, Exp, IfCond, IfBody, WhileCond, WhileBody, ConstExp,
    MulExp, AddExp, RelExp, EqExp, LAndExp, LOrExp,
    Number, FuncCall, Ident, FormatString, ConstTK, IntTK, VoidTK, BreakTK, ContinueTK,
    IfTK, ElseBody, GetintTK, PrintfTK, ReturnTK, WhileTK,
    Not, Plus, Minus, Multi, Div, Mod, And, Or, LSS, LEQ, GRE, GEQ, EQL, NEQ, Assign,
    Unknown
};

class AstNode {
public:
    NodeType nodeType = Unknown;
    string str;
    int val = 0;
    int dim = 0;
    bool used = false;
    vector<AstNode> params;//函数、数组参数
    vector<AstNode> children;
    AstNode *father;

    AstNode();

    AstNode(NodeType type);

    AstNode(NodeType type, string string1);

    void addParam(const AstNode &node);

    void addChild(AstNode node);

    void addLexicon(NodeType type);

    void setVal(int v);

    void setDim(int n);

    void setNodeType(NodeType type);

    void setAllUsed();
};

AstNode parseAst();

AstNode compUnitParser();

void constDeclParser(AstNode *father);

void constDefParser(AstNode *father);

void constInitValParser(AstNode *father, int dim);

void varDeclParser(AstNode *father);

void varDefParser(AstNode *father);

void initValParser(AstNode *father, int dim);

AstNode funcDefParser();

AstNode mainFuncDefParser();

void funcFParamsParser(AstNode *father);

AstNode funcFParamParser();

AstNode blockParser();

void blockItemParser(AstNode *block);

AstNode stmtParser();

AstNode expParser(int dim);

AstNode expParser();

AstNode condParser();

AstNode lValParser();

AstNode primaryExpParser();

AstNode numberParser();

AstNode unaryExpParser();

void funcRParamsParser(AstNode *father);

AstNode mulExpParser();

AstNode addExpParser();

AstNode relExpParser();

AstNode eqExpParser();

AstNode lAndExpParser();

AstNode lOrExpParser();

AstNode constExpParser(int dim);

AstNode constExpParser();

void getNextLexicon();

void pushBackLexicon();

bool matchLexicon(int op);


#endif //PROJECT5_AST_H
