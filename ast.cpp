// Created by lemon on 2021/11/8.
//
#include "iostream"
#include "ast.h"
#include "globalVar.h"

using namespace std;
int lexiconNum = -1;
Content lexicon;
AstNode constList = AstNode(ConstDeclList);
AstNode varList = AstNode(VarDeclList);
AstNode funcList = AstNode(FuncDefList);

AstNode parseAst() {
    return compUnitParser();
}

AstNode compUnitParser() {
    AstNode compUnit = AstNode(CompUnit);
    getNextLexicon();
    //Decl
    while (matchLexicon(4) || matchLexicon(5)) {
        if (matchLexicon(4)) {
            constDeclParser(&constList);
        } else {
            getNextLexicon();
            if (matchLexicon(0)) {
                getNextLexicon();
                if (matchLexicon(32)) {
                    pushBackLexicon();
                    pushBackLexicon();
                    break;
                } else {
                    pushBackLexicon();
                    pushBackLexicon();
                    varDeclParser(&varList);
                }
            } else {
                pushBackLexicon();
                break;
            }
        }
    }
    compUnit.addChild(constList);
    compUnit.addChild(varList);
    //FuncDef
    while (matchLexicon(5) || matchLexicon(14)) {
        if (matchLexicon(5)) {
            getNextLexicon();
            if (matchLexicon(0)) {
                pushBackLexicon();
                funcList.addChild(funcDefParser());
                continue;
            } else {
                pushBackLexicon();
                break;
            }
        } else {
            funcList.addChild(funcDefParser());
            continue;
        }
    }
    compUnit.addChild(funcList);
    //mainFuncDef
    compUnit.addChild(mainFuncDefParser());
    return compUnit;
}

void constDeclParser(AstNode *father) {
    AstNode constDecl = AstNode(ConstDecl);
    constDecl.addLexicon(ConstTK);//const
    getNextLexicon();
    constDecl.addLexicon(IntTK);//int
    getNextLexicon();
    constDefParser(&constDecl);//constDef
    father->addChild(constDecl);
    while (matchLexicon(30)) {
        getNextLexicon();
        AstNode constDecl2 = AstNode(ConstDecl);
        AstNode child1 = AstNode(ConstTK, "const");
        constDecl2.addChild(child1);
        AstNode child2 = AstNode(IntTK, "int");
        constDecl2.addChild(child2);
        constDefParser(&constDecl2);
        father->addChild(constDecl2);
    }
    getNextLexicon();//';'
}

void constDefParser(AstNode *father) {
    AstNode ident = AstNode(Ident, lexicon.str);
    getNextLexicon();
    int cnt = 0;
    while (matchLexicon(34)) {
        cnt++;
        getNextLexicon();//'['
        ident.addParam(constExpParser());
        getNextLexicon();//']'
    }
    ident.setDim(cnt);
    father->addChild(ident);
    father->addLexicon(Assign);//'='
    getNextLexicon();
    AstNode constInitVal = AstNode(ConstInitVal);
    constInitValParser(&constInitVal, 0);
    father->addChild(constInitVal);
}

void constInitValParser(AstNode *father, int dim) {
    if (matchLexicon(36)) {
        getNextLexicon();//'{'
        if (!matchLexicon(37)) {
            constInitValParser(father, dim + 1);
            while (matchLexicon(30)) {
                getNextLexicon();//','
                constInitValParser(father, dim + 1);
            }
        }
        getNextLexicon();//'}'
    } else {
        father->addParam(constExpParser(dim));
    }
}

void varDeclParser(AstNode *father) {
    AstNode varDecl = AstNode(VarDecl);
    varDecl.addLexicon(IntTK);//int
    getNextLexicon();
    varDefParser(&varDecl);
    father->addChild(varDecl);
    while (matchLexicon(30)) {
        getNextLexicon();//','
        AstNode temp = AstNode(VarDecl);
        AstNode child1 = AstNode(IntTK, "int");
        temp.addChild(child1);
        varDefParser(&temp);
        father->addChild(temp);
    }
    getNextLexicon();//';'
}

void varDefParser(AstNode *father) {
    AstNode ident = AstNode(Ident, lexicon.str);
    getNextLexicon();
    int cnt = 0;
    while (matchLexicon(34)) {
        cnt++;
        getNextLexicon();//'['
        ident.addParam(constExpParser());
        getNextLexicon();//']'
    }
    ident.setDim(cnt);
    father->addChild(ident);
    if (matchLexicon(29)) {
        father->addLexicon(Assign);//'='
        getNextLexicon();
        AstNode initVar = AstNode(InitVal);
        initValParser(&initVar, 0);
        father->addChild(initVar);
    }
}

void initValParser(AstNode *father, int dim) {
    if (!matchLexicon(36)) {
        father->addParam(expParser(dim));
    } else {
        getNextLexicon();//'{'
        if (!matchLexicon(37)) {
            initValParser(father, dim + 1);
            while (matchLexicon(30)) {
                getNextLexicon();//','
                initValParser(father, dim + 1);
            }
        }
        getNextLexicon();
    }
}

AstNode funcDefParser() {
    AstNode funcDef = AstNode(FuncDef);
    if (matchLexicon(5)) {
        funcDef.addLexicon(IntTK);
    } else {
        funcDef.addLexicon(VoidTK);
    }
    getNextLexicon();
    AstNode ident = AstNode(Ident, lexicon.str);
    getNextLexicon();//ident
    getNextLexicon();//'('
    if (!matchLexicon(33)) {
        funcFParamsParser(&ident);
    }
    getNextLexicon();//')'
    funcDef.addChild(ident);
    funcDef.addChild(blockParser());
    return funcDef;
}

AstNode mainFuncDefParser() {
    AstNode mainFunc = AstNode(MainFuncDef);
    for (int i = 0; i < 4; i++) {
        getNextLexicon();
    }
    mainFunc.addChild(blockParser());
    return mainFunc;
}

void funcFParamsParser(AstNode *father) {
    father->addParam(funcFParamParser());
    while (matchLexicon(30)) {
        getNextLexicon();//','
        father->addParam(funcFParamParser());
    }
}

AstNode funcFParamParser() {
    AstNode fun = AstNode(FuncFParam);
    fun.addLexicon(IntTK);
    getNextLexicon();
    AstNode ident = AstNode(Ident,lexicon.str);
    getNextLexicon();
    int cnt = 0;
    if (matchLexicon(34)) {
        getNextLexicon();//'['
        if (lexicon.str != "]"){
            ident.addParam(constExpParser(1));
        } else {
            AstNode tmp = AstNode(Number, "0");
            tmp.setVal(0);
            ident.addParam(tmp);
        };
        getNextLexicon();//']'
        cnt++;
        while (matchLexicon(34)) {
            getNextLexicon();//'['
            ident.addParam(constExpParser(2));
            cnt++;
            getNextLexicon();//']'
        }
    }
    ident.setDim(cnt);
//    cout << ident.str << ident.params.size() << endl;
    fun.addChild(ident);
    return fun;
}

AstNode blockParser() {
    AstNode block = AstNode(Block);
    getNextLexicon();//'{'
    while (!matchLexicon(37)) {
        blockItemParser(&block);
    }
    getNextLexicon();//'}'
    return block;
}

void blockItemParser(AstNode *father) {
    if (matchLexicon(4)) {//const
        constDeclParser(father);
    } else if (matchLexicon(5)) {//int
        varDeclParser(father);
    } else {
        father->addChild(stmtParser());
    }
}

AstNode stmtParser() {
    AstNode statement = AstNode(Stmt);
    if (matchLexicon(0)) {
        getNextLexicon();
        int cnt = 1;
        bool flag = false;
        while (!matchLexicon(31)) {
            if (matchLexicon(29)) {
                flag = true;
                break;
            }
            getNextLexicon();
            cnt++;
        }
        while (cnt) {
            pushBackLexicon();
            cnt--;
        }
        if (flag) {
            AstNode assignment = AstNode(Assign, "=");
            assignment.addChild(lValParser());
            getNextLexicon();//'='
            if (!matchLexicon(10)) {
                assignment.addChild(expParser());
                statement.addChild(assignment);
                getNextLexicon();//';'
            } else {
                AstNode getint = AstNode(GetintTK, "getint");
                assignment.addChild(getint);//getint
                statement.addChild(assignment);
                getNextLexicon();
                for (int i = 0; i < 3; i++) {
                    getNextLexicon();//'();'
                }
            }
        } else {
            AstNode exp = expParser();
            getNextLexicon();//';'
            return exp;
        }
    } else if (matchLexicon(31)) {//';'
        getNextLexicon();
    } else if (matchLexicon(36)) {//block
        statement.addChild(blockParser());
    } else if (matchLexicon(8)) {//if
        AstNode ifStmt = AstNode(IfTK, lexicon.str);
        getNextLexicon();//if
        getNextLexicon();//'('
        AstNode ifCond = AstNode(IfCond);
        ifCond.addChild(condParser());
        ifStmt.addChild(ifCond);
        getNextLexicon();//')'
        AstNode ifBody = AstNode(IfBody);
        ifBody.addChild(stmtParser());
        ifStmt.addChild(ifBody);
        if (matchLexicon(9)) {
            AstNode elseBody = AstNode(ElseBody);
            getNextLexicon();//else
            elseBody.addChild(stmtParser());
            ifStmt.addChild(elseBody);
        }
        statement.addChild(ifStmt);
    } else if (matchLexicon(13)) {
        AstNode whileStmt = AstNode(WhileTK, lexicon.str);
        getNextLexicon();//while
        getNextLexicon();//'('
        AstNode whileCond = AstNode(WhileCond);
        whileCond.addChild(condParser());
        whileStmt.addChild(whileCond);
        getNextLexicon();//')'
        AstNode whileBody = AstNode(WhileBody);
        whileBody.addChild(stmtParser());
        whileStmt.addChild(whileBody);
        statement.addChild(whileStmt);
    } else if (matchLexicon(6)) {
        AstNode b = AstNode(BreakTK, lexicon.str);
        statement.addChild(b);
        getNextLexicon();//break
        getNextLexicon();//';'
    } else if (matchLexicon(7)) {
        AstNode c = AstNode(ContinueTK, lexicon.str);
        statement.addChild(c);
        getNextLexicon();//continue
        getNextLexicon();//';'
    } else if (matchLexicon(12)) {
        AstNode returnStmt = AstNode(ReturnTK, lexicon.str);
        getNextLexicon();//return
        if (!matchLexicon(31)) {
            returnStmt.addChild(expParser());
        }
        statement.addChild(returnStmt);
        getNextLexicon();//';'
    } else if (matchLexicon(11)) {
        AstNode pri = AstNode(PrintfTK, lexicon.str);
        getNextLexicon();//printf
        getNextLexicon();//'('
        pri.addLexicon(FormatString);//formatString
        getNextLexicon();
        while (matchLexicon(30)) {
            getNextLexicon();//','
            pri.addParam(expParser());
        }
        getNextLexicon();//')'
        getNextLexicon();//';'
        statement.addChild(pri);
    } else {
        AstNode exp = expParser();
        getNextLexicon();//';'
        return exp;
    }
    return statement;
}

AstNode expParser(int dim) {
    AstNode exp = AstNode(Exp);
    exp.setDim(dim);
    exp.addChild(addExpParser());
    return exp;
}

AstNode expParser() {
    AstNode exp = AstNode(Exp);
    exp.addChild(addExpParser());
    return exp;
}

AstNode condParser() {
    return lOrExpParser();
}

AstNode lValParser() {
    AstNode ident = AstNode(Ident, lexicon.str);
//    cout << lexicon.str << endl;
    getNextLexicon();
    int cnt = 0;
    while (matchLexicon(34)) {
        cnt++;
        getNextLexicon();//'['
//        cout << lexicon.str << endl;
        ident.addParam(expParser());
        getNextLexicon();//']'
    }
    ident.setDim(cnt);
    return ident;
}

AstNode primaryExpParser() {
    if (matchLexicon(32)) {
        getNextLexicon();//'('
        AstNode pri = expParser();
        getNextLexicon();//')'
        return pri;
    } else if (matchLexicon(1)) {
        return numberParser();
    } else {
        return lValParser();
    }
}

AstNode numberParser() {
    AstNode num = AstNode(Number, lexicon.str);
    getNextLexicon();
    return num;
}

AstNode unaryExpParser() {
    if (matchLexicon(0)) {
        string name = lexicon.str;
        getNextLexicon();
        if (matchLexicon(32)) {
            AstNode ident = AstNode(FuncCall, name);
            getNextLexicon();//'('
            if (!matchLexicon(33)) {
                funcRParamsParser(&ident);
            }
            getNextLexicon();//')'
            return ident;
        } else {
            pushBackLexicon();
            return primaryExpParser();
        }
    } else if (matchLexicon(16) || matchLexicon(17) || matchLexicon(15)) {
        AstNode op = AstNode();
        if (matchLexicon(15)) {
            op.setNodeType(Not);
        } else if (matchLexicon(16)) {
            op.setNodeType(Plus);
        } else {
            op.setNodeType(Minus);
        }
        getNextLexicon();
        op.addChild(unaryExpParser());
        return op;
    } else {
        return primaryExpParser();
    }
}

void funcRParamsParser(AstNode *father) {
    father->addParam(expParser());
    while (matchLexicon(30)) {
        getNextLexicon();//','
        father->addParam(expParser());
    }
//    cout << father->str << father->params.size() << endl;
}

AstNode mulExpParser() {
    AstNode mul = AstNode(MulExp);
    mul.addChild(unaryExpParser());
    while (lexicon.type >= 18 && lexicon.type <= 20) {
        AstNode op = AstNode();
        if (matchLexicon(18)) {
            op.setNodeType(Multi);
        } else if (matchLexicon(19)) {
            op.setNodeType(Div);
        } else {
            op.setNodeType(Mod);
        }
        mul.addChild(op);
        getNextLexicon();
        mul.addChild(unaryExpParser());
    }
    if (mul.children.size() == 1) {
        return mul.children[0];
    } else {
        return mul;
    }
}

AstNode addExpParser() {
    AstNode add = AstNode(AddExp);
    add.addChild(mulExpParser());
    while (matchLexicon(16) || matchLexicon(17)) {
        AstNode op = AstNode();
        if (matchLexicon(16)) {
            op.setNodeType(Plus);
        } else {
            op.setNodeType(Minus);
        }
        add.addChild(op);
        getNextLexicon();
        add.addChild(mulExpParser());
    }
//    cout << add.children[0].str << endl;
    if (add.children.size() == 1) {
        return add.children[0];
    } else {
        return add;
    }
}

AstNode relExpParser() {
    AstNode rel = AstNode(RelExp);
    rel.addChild(addExpParser());
    while (lexicon.type >= 23 && lexicon.type <= 26) {
        AstNode op = AstNode();
        if (matchLexicon(23)) {
            op.setNodeType(LSS);
        } else if (matchLexicon(24)) {
            op.setNodeType(LEQ);
        } else if (matchLexicon(25)) {
            op.setNodeType(GRE);
        } else {
            op.setNodeType(GEQ);
        }
        rel.addChild(op);
        getNextLexicon();
        rel.addChild(addExpParser());
    }
    if (rel.children.size() == 1) {
        return rel.children[0];
    } else {
        return rel;
    }
}

AstNode eqExpParser() {
    AstNode eq = AstNode(EqExp);
    eq.addChild(relExpParser());
    while (matchLexicon(27) || matchLexicon(28)) {
        AstNode op = AstNode();
        if (matchLexicon(27)) {
            op.setNodeType(EQL);
        } else {
            op.setNodeType(NEQ);
        }
        eq.addChild(op);
        getNextLexicon();
        eq.addChild(relExpParser());
    }
    if (eq.children.size() == 1) {
        return eq.children[0];
    } else {
        return eq;
    }
}

AstNode lAndExpParser() {
    AstNode lAnd = AstNode(LAndExp);
    lAnd.addChild(eqExpParser());
    while (matchLexicon(21)) {
        AstNode op = AstNode(And, lexicon.str);
        lAnd.addChild(op);
        getNextLexicon();
        lAnd.addChild(eqExpParser());
    }
    if (lAnd.children.size() == 1) {
        return lAnd.children[0];
    } else {
        return lAnd;
    }
}

AstNode lOrExpParser() {
    AstNode lOr = AstNode(LOrExp);
    lOr.addChild(lAndExpParser());
    while (matchLexicon(22)) {
        AstNode op = AstNode(Or, lexicon.str);
        lOr.addChild(op);
        getNextLexicon();
        lOr.addChild(lAndExpParser());
    }
    if (lOr.children.size() == 1) {
        return lOr.children[0];
    } else {
        return lOr;
    }
//    if (lOr.children[0].nodeType == LAndExp && lOr.children.size() == 1) {
//        return lOr.children[0];
//    } else {
//        return lOr;
//    }
}

AstNode constExpParser(int dim) {
    AstNode constExp = AstNode(ConstExp);
    constExp.setDim(dim);
    constExp.addChild(addExpParser());
    return constExp;
}

AstNode constExpParser() {
    AstNode constExp = AstNode(ConstExp);
    constExp.addChild(addExpParser());
    return constExp;
}

AstNode::AstNode() {
    this->str = lexicon.str;
}

AstNode::AstNode(NodeType type) {
    this->nodeType = type;
}

void AstNode::addChild(AstNode node) {
    this->children.push_back(node);
    node.father = this;
}

void AstNode::addLexicon(NodeType type) {
    AstNode temp = AstNode();
    temp.nodeType = type;
    this->addChild(temp);
}

void AstNode::setDim(int n) {
    this->dim = n;
}

AstNode::AstNode(NodeType type, string string1) {
    this->nodeType = type;
    this->str = std::move(string1);
}

void AstNode::addParam(const AstNode &node) {
    this->params.push_back(node);
}

void AstNode::setNodeType(NodeType type) {
    this->nodeType = type;
}

void AstNode::setAllUsed() {
    this->used = true;
    if (this->children.empty()) {
        return;
    }
    for (int i = 0; i < this->children.size(); i++) {
        this->children[i].used = true;
    }
}

void AstNode::setVal(int v) {
    this->val = v;
}

void getNextLexicon() {
    if (lexiconNum == -1 || lexiconNum < storage.size() - 1) {
        lexicon = storage[++lexiconNum];
//        cout << lexicon.str << endl;
    } else {
        return;
    }
//    cout << lexicon.line << " " << lexicon.str << endl;
}

void pushBackLexicon() {
    if (!storage.empty()) {
        lexicon = storage[--lexiconNum];
    }
}

bool matchLexicon(int op) {
    if (lexicon.type == op) {
        return true;
    } else {
        return false;
    }
}