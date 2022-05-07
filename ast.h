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
    NodeType nodeType = Unknown;//ast节点类型
    string str;                 //名称
    int val = 0;                //值
    int dim = 0;                //数组维数，可取0,1,2
    bool used = false;          //判断是否已经用于生成中间代码
    bool isFuncParam = false;   //判断是否是函数参数
    vector<AstNode> params;     //函数、数组参数
    vector<AstNode> children;   //存储子节点
    AstNode *father;            //存储父节点（好像没用）

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
