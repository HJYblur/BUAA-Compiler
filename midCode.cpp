//
// Created by lemon on 2021/11/11.
//
/*
 * 待解决的问题：
 * 1、√√√√√√√√√建立符号表
 * 2、√√√√√√√√√数组的相关操作
 * 3、√√√√√√√√√while里的break&continue
 * 4、√√√√√√√√√函数的返回值
 * 5、cond->&& ||多个连接的情况
 */
#include "iostream"
#include "midCode.h"
#include "deque"
#include "string"
#include "stack"

using namespace std;
vector<MidCode> midCodeTable;
SymbolTable symTable;
SymbolLayer *currentLayer;
int tmpVarCnt = 0;
int ifLabelCnt = 1;
int whileLabelCnt = 1;
int printStrCnt = 0;
string funcName;
stack<int> whileStack;
AstNode retVal = AstNode(Ident, "$v0");

void postOrderTraversal(AstNode node) {
    symTable.forwardLayer();
    AstNode *root = &node;
    stack<AstNode *> s;
    s.push(root);

    while (!s.empty()) {
        AstNode *tmp = s.top();
//        printAstNode(tmp);
        toMidCode(tmp);
        s.pop();
        for (int i = tmp->children.size() - 1; i >= 0; i--) {
            if (!tmp->children[i].used) {
                s.push(&tmp->children[i]);
            } else {
                continue;
            }
        }
    }
//    symTable.print();
//    printMidCode();
}

void toMidCode(AstNode *node) {
    switch (node->nodeType) {
        case ConstDeclList:
            for (int i = 0; i < node->children.size(); i++) {
                constDeclMid(&node->children[i]);
            }
            break;
        case VarDeclList:
            for (int i = 0; i < node->children.size(); i++) {
                varDeclMid(&node->children[i]);
            }
            break;
        case FuncDefList:
            for (int i = 0; i < node->children.size(); i++) {
                funcDefMid(&node->children[i]);
            }
            break;
        case FuncDef:
            funcDefMid(node);
            break;
        case MainFuncDef: {
            MidCode main = MidCode(MainFuncMid, 1);
            main.setOp1("main");
            pushMidCode(main);
            blockMid(&node->children[0]);
            break;
        }
        case Block:
            blockMid(node);
            break;
        case ConstDecl:
            constDeclMid(node);
            break;
        case VarDecl:
            varDeclMid(node);
            break;
        case Stmt:
            for (int i = 0; i < node->children.size(); i++) {
                toMidCode(&node->children[i]);
            }
            break;
        case Assign:
            assignMid(node);
            break;
        case IfTK:
            ifMid(node);
            break;
        case WhileTK:
            whileMid(node);
            break;
        case BreakTK:
            breakMid(node);
            break;
        case ContinueTK:
            continueMid(node);
            break;
        case ReturnTK:
            returnMid(node);
            break;
        case Exp:
//            symTable.removeTmpVar();
            expMid(node);
            break;
        case PrintfTK:
            printfMid(node);
            break;
        case FuncCall:
            expItemMid(node);
        case CompUnit:
        case Number:
        case Ident:
        case FormatString:
        case ConstTK:
        case IntTK:
        case VoidTK:
        case IfCond:
        case IfBody:
        case ElseBody:
        case GetintTK:
        case AddExp:
        case MulExp:
        case Plus:
        case Minus:
        case Multi:
        case Div:
        case Mod:
        case ConstInitVal:
        case InitVal:
            break;
        default:
            cout << "You've missed " << getEnum(node->nodeType) << endl;
            break;
    }
}

void constDeclMid(AstNode *node) {
    node->setAllUsed();
    AstNode ident = node->children[2];
    if (ident.dim == 0) {
        SymbolItem constSymbol = SymbolItem(ConstSymbol, ident);
        MidCode constMid = MidCode(ConstDeclMid);
        AstNode var = initialMid(&node->children[4]);
        constMid.setOp1(ident.str);
        constMid.setOp2(to_string(var.val));
        if (var.nodeType != Number) {
            constMid.setResultType(1);
        }
        pushMidCode(constMid);
        constSymbol.setValue(var.val);
        currentLayer->addSymbol(constSymbol);
    } else {
        //数组
        SymbolItem arraySymbol = SymbolItem(ArraySymbol, ident);
        MidCode arrMid = MidCode(ArrayDeclMid);//arr[x][y]
        arrMid.setResult(ident.str);
        int xCnt = 0;
        int yCnt = 0;
        if (ident.dim > 0) {
            AstNode x = expMid(&ident.params[0]);
            arrMid.setOp1(to_string(x.val));
            arrMid.setOpNum(1);
            arraySymbol.setDim1(x.val);
            arraySymbol.setDim(1);
            xCnt = x.val;
        }
        if (ident.dim > 1) {
            AstNode y = expMid(&ident.params[1]);
            arrMid.setOp2(to_string(y.val));
            arrMid.setOpNum(2);
            arraySymbol.setDim2(y.val);
            arraySymbol.setDim(2);
            yCnt = y.val;
        }
        pushMidCode(arrMid);
        vector<AstNode> vars = initialMid(&node->children[4]).children;
        if (ident.dim == 1) {
            for (int i = 0; i < xCnt; i++) {
                //数组每个分量的值
                AstNode content = vars[i];
//                cout << getEnum(content.nodeType) << content.val<< endl;
                //符号表添加值
                SymbolItem var = SymbolItem(ConstSymbol);
                var.setValue(content.val);
                arraySymbol.addParam(var);
                //生成中间代码
                MidCode setArrayVar = MidCode(ArrayVarMid);
                setArrayVar.setOpRand(ident.str);//arr[i]=xxx;
                setArrayVar.setOp1(to_string(i * 4));
                setArrayVar.setResult(content.str);
                setArrayVar.setOpNum(1);
                if (content.nodeType != Number) {
                    setArrayVar.setResultType(1);
                }
                pushMidCode(setArrayVar);
            }
        } else {
            int cnt = 0;
            for (int i = 0; i < xCnt; i++) {
                for (int j = 0; j < yCnt; j++) {
                    AstNode content = vars[cnt++];
                    //符号表
                    SymbolItem var = SymbolItem(ConstSymbol);
                    var.setValue(content.val);
                    arraySymbol.addParam(var);
                    //中间代码
                    MidCode setArrayVar = MidCode(ArrayVarMid);
                    setArrayVar.setOpRand(ident.str);
                    setArrayVar.setOp1(to_string(i * 4));
                    setArrayVar.setOp2(to_string(j * 4));
                    setArrayVar.setResult(content.str);
                    if (content.nodeType != Number) {
                        setArrayVar.setResultType(1);
                    }
                    pushMidCode(setArrayVar);
                }
            }
        }
        currentLayer->addSymbol(arraySymbol);
    }
}

void varDeclMid(AstNode *node) {
    node->setAllUsed();
    AstNode ident = node->children[1];
    if (ident.dim == 0) {
        SymbolItem varSymbol = SymbolItem(VarSymbol, ident);
        MidCode varMid = MidCode(VarDeclMid);
        varMid.setOp1(ident.str);
        if (node->children.size() > 2) {
            AstNode var = initialMid(&node->children[3]);
            varMid.setOp2(var.str);
            varSymbol.setValue(var.val);
            if (var.nodeType != Number) {
                varMid.setResultType(1);
            }
        } else {
            varMid.setOpNum(1);
        }
        pushMidCode(varMid);
        currentLayer->addSymbol(varSymbol);
    } else {
        //数组
        SymbolItem arraySymbol = SymbolItem(ArraySymbol, ident);
        MidCode arrMid = MidCode(ArrayDeclMid);//arr[x][y]
        arrMid.setResult(ident.str);
        int xCnt = 0;
        int yCnt = 0;
        if (ident.dim > 0) {
            AstNode x = expMid(&ident.params[0]);
            arrMid.setOp1(to_string(x.val));
            arrMid.setOpNum(1);
            arraySymbol.setDim1(x.val);
            arraySymbol.setDim(1);
            xCnt = x.val;
        }
        if (ident.dim > 1) {
            AstNode y = expMid(&ident.params[1]);
            arrMid.setOp2(to_string(y.val));
            arrMid.setOpNum(2);
            arraySymbol.setDim2(y.val);
            arraySymbol.setDim(2);
            yCnt = y.val;
        }
        pushMidCode(arrMid);
        if (node->children.size() < 3) {
            if (ident.dim == 1) {
                for (int i = 0; i < xCnt; i++) {
                    SymbolItem num = SymbolItem(VarSymbol);
                    num.setDim(0);
                    num.setValue(0);
                    arraySymbol.addParam(num);
                }
            } else if (ident.dim == 2) {
                int cnt = xCnt * yCnt;
                for (int i = 0; i < cnt; i++) {
                    SymbolItem num = SymbolItem(VarSymbol);
                    num.setDim(0);
                    num.setValue(0);
                    arraySymbol.addParam(num);
                }
            }
            currentLayer->addSymbol(arraySymbol);
            return;
        }
        vector<AstNode> vars = initialMid(&node->children[3]).children;
        if (ident.dim == 1) {
            for (int i = 0; i < xCnt; i++) {
                //数组每个分量的值
                AstNode content = vars[i];
//                cout << getEnum(content.nodeType) << content.val<< endl;
                //符号表添加值
                SymbolItem var = SymbolItem(ConstSymbol);
                var.setValue(content.val);
                arraySymbol.addParam(var);
                //生成中间代码
                MidCode setArrayVar = MidCode(ArrayVarMid);
                setArrayVar.setOpRand(ident.str);//arr[i]=xxx;
                setArrayVar.setOp1(to_string(i * 4));
                setArrayVar.setResult(content.str);
                setArrayVar.setOpNum(1);
                if (content.nodeType != Number) {
                    setArrayVar.setResultType(1);
                }
                pushMidCode(setArrayVar);
            }
        } else {
            int cnt = 0;
            for (int i = 0; i < xCnt; i++) {
                for (int j = 0; j < yCnt; j++) {
                    AstNode content = vars[cnt++];
                    //符号表
                    SymbolItem var = SymbolItem(ConstSymbol);
                    var.setValue(content.val);
                    arraySymbol.addParam(var);
                    //中间代码
                    MidCode setArrayVar = MidCode(ArrayVarMid);
                    setArrayVar.setOpRand(ident.str);
                    setArrayVar.setOp1(to_string(i * 4));
                    setArrayVar.setOp2(to_string(j * 4));
                    setArrayVar.setResult(content.str);
                    if (content.nodeType != Number) {
                        setArrayVar.setResultType(1);
                    }
                    pushMidCode(setArrayVar);
                }
            }
        }
        currentLayer->addSymbol(arraySymbol);
    }
}

void funcDefMid(AstNode *node) {
    node->setAllUsed();
    MidCode funcMid = MidCode(FuncDefMid);
    AstNode ident = node->children[1];
    SymbolItem funcSymbol = SymbolItem(ident);
    funcName = ident.str;
    //int/void
    if (node->children[0].nodeType == IntTK) {
        funcMid.setResultType(0);
        funcSymbol.setSymbolType(IntFuncSymbol);
    } else {
        funcMid.setResultType(1);
        funcSymbol.setSymbolType(VoidFuncSymbol);
    }
    funcMid.setOp1(ident.str);
    pushMidCode(funcMid);
    //params
    if (!ident.params.empty()) {
        for (auto &param : ident.params) {
            funcParamMid(&param, &funcSymbol);
        }
    }
    currentLayer->addSymbol(funcSymbol);
    //funcBody
    if (node->children.size() > 2) {
        blockMid(&node->children[2]);
    }
    //label:FuncEnd
    MidCode funcLabel = MidCode(FuncEndMid, 1);
    funcLabel.setOp1(ident.str);
    pushMidCode(funcLabel);
}

void funcParamMid(AstNode *node, SymbolItem *item) {
    MidCode para = MidCode(FuncParamMid);
    AstNode ident = node->children[1];
    para.setResult(funcName);
    para.setOpRand(ident.str);
    para.paraNumber = item->params.size();
    SymbolItem paraSymbol = SymbolItem(VarSymbol, ident);
    paraSymbol.isFParam = true;
    if (ident.dim == 0) {
        para.setOpNum(0);
    } else if (ident.dim == 1) {
        //数组
//        if (ident.params[0].str == ""){
//            para.setOp1("0");
//
//        }
        AstNode x = expItemMid(&ident.params[0]);
        para.setOp1(x.str);
        para.setOpNum(1);
        paraSymbol.setDim1(x.val);
        paraSymbol.setDim(1);
    } else {
        AstNode x = expItemMid(&ident.params[0]);
        AstNode y = expMid(&ident.params[1]);
        para.setOp1(x.str);
        para.setOp2(y.str);
        paraSymbol.setDim1(x.val);
        paraSymbol.setDim2(y.val);
        paraSymbol.setDim(2);
    }
    pushMidCode(para);
    paraSymbol.setValue(0);
    item->addParam(paraSymbol);
    currentLayer->addSymbol(paraSymbol);
}

void blockMid(AstNode *node) {
    //新建一层符号表
    symTable.forwardLayer();
    MidCode begin = MidCode(BlockBegin);
    pushMidCode(begin);
    node->setAllUsed();
    for (int i = 0; i < node->children.size(); i++) {
        toMidCode(&node->children[i]);
    }
    //回退符号表
    symTable.backLayer();
    MidCode end = MidCode(BlockEnd);
    pushMidCode(end);
}

void assignMid(AstNode *node) {
    node->setAllUsed();
    //a = getint()
    if (node->children[1].nodeType == GetintTK) {
        AstNode ident = node->children[0];
        if (ident.dim == 0) {
            MidCode getInt = MidCode(GetIntMid);
            getInt.setResult(ident.str);
            getInt.setOp1("GetInt");
            getInt.setOpNum(1);
            pushMidCode(getInt);
        } else if (ident.dim == 1) {
            MidCode getInt = MidCode(GetIntMid);
            getInt.setResult(getRegisterT());
            getInt.setOp1("GetInt");
            getInt.setOpNum(1);
            pushMidCode(getInt);

            MidCode getArr = MidCode(ArrayVarMid);
            AstNode x = expMid(&ident.params[0]);
            AstNode ans = getAddress(&x);
            getArr.setResult(getInt.result);
            getArr.setOpRand(ident.str);
            getArr.setOp1(ans.str);
            getArr.setOpNum(1);
            getArr.setResultType(1);
            pushMidCode(getArr);
        } else {
            MidCode getInt = MidCode(GetIntMid);
            getInt.setResult(getRegisterT());
            getInt.setOp1("GetInt");
            getInt.setOpNum(1);
            pushMidCode(getInt);

            int dim2 = 0;
            if (symTable.checkExistedSymbol(&ident)) {
                SymbolItem *arr = symTable.getExistedSymbol(&ident);
                dim2 = arr->dim2;
            }
            MidCode getArr = MidCode(ArrayVarMid);
            AstNode x = expMid(&ident.params[0]);
            AstNode y = expMid(&ident.params[1]);
            AstNode ans = getAddress(&x, &y, dim2);
            getArr.setResult(getInt.result);
            getArr.setOpRand(ident.str);
            getArr.setOp1(ans.str);
            getArr.setOpNum(1);
            getArr.setResultType(1);
            pushMidCode(getArr);
        }
    }
        //a = b
    else {
        AstNode ident = node->children[0];
        AstNode expNode = expMid(&node->children[1]);
        if (!symTable.checkExistedSymbol(&ident)) {
            return;
        }
        SymbolItem *item = symTable.getExistedSymbol(&ident);
//        cout << item->str << " " << ident.dim << endl;
        if (ident.dim == 0) {
            //符号表赋值
            item->setValue(expNode.val);
            //中间代码
            MidCode exp = MidCode(ExpMid);
            exp.setResult(ident.str);
            exp.setOp1(expNode.str);
            exp.setOpNum(0);
            if (expNode.nodeType != Number) {
                exp.setResultType(1);
            }
            pushMidCode(exp);
        } else if (ident.dim == 1) {
            //赋值
            int index = getAddress(&ident.params[0]).val;
//            cout<<item->params.size()<<endl;
            item->params[index].setValue(expNode.val);
            //中间代码
            MidCode arr = MidCode(ArrayVarMid);
            arr.setOpRand(ident.str);
            arr.setOp1(to_string(index));
            arr.setResult(expNode.str);
            arr.setOpNum(1);
            if (expNode.nodeType != Number) {
                arr.setResultType(1);
            }
            pushMidCode(arr);
        } else {
            //赋值
            AstNode x = expMid(&ident.params[0]);
            AstNode y = expMid(&ident.params[1]);
//            cout << "1" <<item->str << endl;
            AstNode ans = getAddress(&x, &y, item->dim2);
//            cout << "2" <<item->str << endl;
            item->params[ans.val / 4].setValue(expNode.val);
            //中间代码
            MidCode arr = MidCode(ArrayVarMid);
            arr.setOpRand(ident.str);
            arr.setOp1(ans.str);
            arr.setResult(expNode.str);
            arr.setOpNum(1);
            if (expNode.nodeType != Number) {
                arr.setResultType(1);
            }
            pushMidCode(arr);
        }
    }
    tmpVarCnt = 0;
}

void ifMid(AstNode *node) {
    node->setAllUsed();
    bool hasElse = false;
    if (node->children.size() == 3) {
        hasElse = true;
    }
    //IfTK
    string endName = "if_end" + to_string(ifLabelCnt);
    string ifName = "if_body" + to_string(ifLabelCnt);
    string elseName = "else_body" + to_string(ifLabelCnt);
    ifLabelCnt++;
    //IfCond
    AstNode ifCond = node->children[0];
    if (hasElse) {
        condMid(&ifCond.children[0], ifName, elseName, true);
    } else {
        condMid(&ifCond.children[0], ifName, endName, true);
    }
    //IfBody
    MidCode ifLabel = MidCode(Label, 1);
    ifLabel.setOp1(ifName);
    pushMidCode(ifLabel);
    AstNode ifBody = node->children[1];
    toMidCode(&ifBody.children[0]);
    MidCode go = MidCode(Goto, 1);
    go.setOp1(endName);
    pushMidCode(go);
    //ElseBody
    if (hasElse) {
        MidCode elseLabel = MidCode(Label, 1);
        elseLabel.setOp1(elseName);
        pushMidCode(elseLabel);
        AstNode elseBody = node->children[2];
        toMidCode(&elseBody.children[0]);
    }
    //end
    MidCode endLabel = MidCode(Label, 1);
    endLabel.setOp1(endName);
    pushMidCode(endLabel);
}

void whileMid(AstNode *node) {
    node->setAllUsed();
    string bodyName = "while_body" + to_string(whileLabelCnt);
    string endName = "while_end" + to_string(whileLabelCnt);
    whileStack.push(whileLabelCnt);
    whileLabelCnt++;
    //whileCond
    AstNode cond = node->children[0];
    condMid(&cond.children[0], bodyName, endName, true);
    //whileBody
    MidCode bodyLabel = MidCode(Label, 1);
    bodyLabel.setOp1(bodyName);
    pushMidCode(bodyLabel);
    AstNode whileBody = node->children[1];
    toMidCode(&whileBody.children[0]);
    //end
    MidCode endLabel = MidCode(Label, 1);
    endLabel.setOp1(endName);
    pushMidCode(endLabel);
    whileStack.pop();
}


void breakMid(AstNode *node) {
    node->setAllUsed();
    MidCode b = MidCode(Goto, 1);
    int cnt = whileStack.top();
    b.setOp1("while_end" + to_string(cnt));
    pushMidCode(b);
}

void continueMid(AstNode *node) {
    node->setAllUsed();
    MidCode c = MidCode(Goto, 1);
    int cnt = whileStack.top();
    c.setOp1("while_body" + to_string(cnt));
    pushMidCode(c);
}

void returnMid(AstNode *node) {
    node->setAllUsed();
    MidCode re = MidCode(Return);
    re.setOp1("return");
    if (!node->children.empty()) {
        AstNode item = node->children[0].children[0];
        if (item.nodeType == Number) {
            re.setOp2(item.str);
            retVal.setVal(stoi(item.str));
            re.setOpNum(1);
        } else {
            //exp
            AstNode exp = expItemMid(&item);
            re.setOp2(exp.str);
            retVal.setVal(exp.val);
            re.setOpNum(2);
        }
    } else {
        re.setOpNum(0);
        retVal.setVal(0);
    }
    pushMidCode(re);
}

void printfMid(AstNode *node) {
    node->setAllUsed();
    string s = node->children[0].str;
    string t;
    int cnt = 0;
//    cout << s<< endl;
    for (int i = 0; i < s.size(); i++) {
        if (s[i] == '%' && s[i + 1] == 'd') {
            if (!t.empty()) {
                MidCode str = MidCode(PrintfMid, 1);
                str.setOp1(t);
                str.setOp2(to_string(printStrCnt++));
                pushMidCode(str);
            }
            MidCode ident = MidCode(PrintfMid, 2);
            AstNode exp = expItemMid(&node->params[cnt++]);
//            cout << exp.str << endl;
            ident.setOp2(exp.str);
            if (exp.nodeType != Number) {
                ident.setResultType(1);
            }
            pushMidCode(ident);
            t = "";
            i++;
        } else if (s[i] != '"') {
            t += s[i];
        }
    }
    MidCode str = MidCode(PrintfMid, 1);
    str.setOp1(t);
    str.setOp2(to_string(printStrCnt++));
    pushMidCode(str);
}

bool condMid(AstNode *node, const string &ifBody, const string &elseBody, bool flag) {
    node->setAllUsed();
    switch (node->nodeType) {
        case LOrExp:
            return lOrMid(node, ifBody, elseBody);
        case LAndExp:
            return lAndMid(node, ifBody, elseBody);
        case EqExp:
            return eqMid(node, ifBody, elseBody, flag);
        case RelExp:
            return relMid(node, ifBody, elseBody, flag);
        default:
            return false;
    }
}

bool lOrMid(AstNode *node, const string &ifBody, const string &elseBody) {
    //仅支持比较一次
    node->setAllUsed();
    for (auto &i : node->children) {
        if (i.nodeType == Or) {
            continue;
        }
        if (condMid(node, ifBody, elseBody, true)) {
            return true;
        }
    }
    MidCode go = MidCode(Goto, 1);
    go.setOp1(elseBody);
    pushMidCode(go);
    return false;
}

bool lAndMid(AstNode *node, const string &ifBody, const string &elseBody) {
    node->setAllUsed();
    for (auto &i : node->children) {
        if (i.nodeType == And) {
            continue;
        }
        if (condMid(&i, ifBody, elseBody, false)) {
            return true;
        }
    }
    MidCode go = MidCode(Goto, 1);
    go.setOp1(ifBody);
    pushMidCode(go);
    return false;
}

bool eqMid(AstNode *node, const string &ifBody, const string &elseBody, bool flag) {
    node->setAllUsed();
    vector<AstNode> list = node->children;//a==b/a!=b
    AstNode right = list[list.size() - 1];
    list.pop_back();
    AstNode op = list[list.size() - 1];
    list.pop_back();
    AstNode left = list[list.size() - 1];
    list.pop_back();
    MidCode cmp = MidCode(Cmp);//cmp a==b
    cmp.setOp1(left.str);
    cmp.setOp2(right.str);
    bool isTrue;
    if (flag) {
        //Or
        if (op.nodeType == EQL) {
            isTrue = (left.str == right.str);
            pushMidCode(cmp);
            MidCode beq = MidCode(Beq, 1);
            beq.setOp1(ifBody);
            pushMidCode(beq);
        } else {
            isTrue = (left.str != right.str);
            pushMidCode(cmp);
            MidCode bne = MidCode(Bne, 1);
            bne.setOp1(ifBody);
            pushMidCode(bne);
        }
    } else {
        //And
        if (op.nodeType == EQL) {
            isTrue = (left.str == right.str);
            pushMidCode(cmp);
            MidCode bne = MidCode(Bne, 1);
            bne.setOp1(elseBody);
            pushMidCode(bne);
        } else {
            isTrue = (left.str != right.str);
            pushMidCode(cmp);
            MidCode beq = MidCode(Beq, 1);
            beq.setOp1(elseBody);
            pushMidCode(beq);
        }
    }
    if (isTrue) {
        return true;
    } else {
        return false;
    }
}

bool relMid(AstNode *node, const string &ifBody, const string &elseBody, bool flag) {
    node->setAllUsed();
    vector<AstNode> list = node->children;
    AstNode right = list[list.size() - 1];
    list.pop_back();
    AstNode op = list[list.size() - 1];
    list.pop_back();
    AstNode left = list[list.size() - 1];
    list.pop_back();
    MidCode cmp = MidCode(Cmp);//cmp a>b a<b a>=b a<=b
    cmp.setOp1(left.str);
    cmp.setOp2(right.str);
    bool isTrue;
    if (flag) {
        //Or
        if (op.nodeType == GRE) {
            isTrue = (left.str > right.str);
            pushMidCode(cmp);
            MidCode bgt = MidCode(Bgt, 1);
            bgt.setOp1(ifBody);
            pushMidCode(bgt);
        } else if (op.nodeType == LSS) {
            isTrue = (left.str < right.str);
            pushMidCode(cmp);
            MidCode blt = MidCode(Blt, 1);
            blt.setOp1(ifBody);
            pushMidCode(blt);
        } else if (op.nodeType == GEQ) {
            isTrue = (left.str >= right.str);
            pushMidCode(cmp);
            MidCode bge = MidCode(Bge, 1);
            bge.setOp1(ifBody);
            pushMidCode(bge);
        } else {
            isTrue = (left.str <= right.str);
            pushMidCode(cmp);
            MidCode ble = MidCode(Ble, 1);
            ble.setOp1(ifBody);
            pushMidCode(ble);
        }
    } else {
        //And
        if (op.nodeType == GRE) {
            isTrue = (left.str > right.str);
            pushMidCode(cmp);
            MidCode blt = MidCode(Blt, 1);
            blt.setOp1(elseBody);
            pushMidCode(blt);
        } else if (op.nodeType == LSS) {
            isTrue = (left.str < right.str);
            pushMidCode(cmp);
            MidCode bgt = MidCode(Bgt, 1);
            bgt.setOp1(elseBody);
            pushMidCode(bgt);
        } else if (op.nodeType == GEQ) {
            isTrue = (left.str >= right.str);
            pushMidCode(cmp);
            MidCode ble = MidCode(Ble, 1);
            ble.setOp1(elseBody);
            pushMidCode(ble);
        } else {
            isTrue = (left.str <= right.str);
            pushMidCode(cmp);
            MidCode bge = MidCode(Bge, 1);
            bge.setOp1(elseBody);
            pushMidCode(bge);
        }
    }
    if (isTrue) {
        return true;
    } else {
        return false;
    }
}

AstNode expMid(AstNode *node) {
//    cout << node->children[0].nodeType << endl;
    node->setAllUsed();
    switch (node->children[0].nodeType) {
        case AddExp:
            return addExpMid(&node->children[0]);
        case MulExp:
            return mulExpMid(&node->children[0]);
        default:
            return unaryExpMid(&node->children[0]);
    }

}

AstNode expItemMid(AstNode *node) {
    node->setAllUsed();
    switch (node->nodeType) {
        case AddExp:
            return addExpMid(node);
        case MulExp:
            return mulExpMid(node);
        default:
            return unaryExpMid(node);
    }
}

AstNode addExpMid(AstNode *node) {
    node->setAllUsed();
//    vector<AstNode> list = node->children;
    deque<AstNode> list(node->children.begin(), node->children.end());
    while (list.size() > 1) {
        AstNode left = expItemMid(&list.front());
        list.pop_front();
        AstNode op = list.front();
        list.pop_front();
        AstNode right = expItemMid(&list.front());
        list.pop_front();
        AstNode result;
        if (left.nodeType != Number || right.nodeType != Number) {
            MidCode plus = MidCode(ExpMid);
            plus.setOp1(left.str);
            plus.setOpRand(op.str);
            plus.setOp2(right.str);
            string resultName = getRegisterT();
            result = AstNode(Ident, resultName);
            plus.setResult(resultName);
            pushMidCode(plus);
        } else {
            result = AstNode(Number);
        }
        if (op.nodeType == Plus) {
            result.setVal(left.val + right.val);
        } else {
            result.setVal(left.val - right.val);
        }
        if (left.nodeType == Number && right.nodeType == Number) {
            result.str = to_string(result.val);
        }
        list.push_front(result);
//        cout << result.str << result.val << endl;
    }
//    SymbolItem item = SymbolItem(VarSymbol, list[0]);
//    currentLayer->addSymbol(item);
    return list[0];
}

AstNode mulExpMid(AstNode *node) {
    node->setAllUsed();
//    vector<AstNode> list = node->children;
    deque<AstNode> list(node->children.begin(), node->children.end());
    while (list.size() > 1) {
        AstNode left = expItemMid(&list.front());
        list.pop_front();
        AstNode op = list.front();
        list.pop_front();
        AstNode right = expItemMid(&list.front());
        list.pop_front();
        AstNode result;
        if (left.nodeType != Number || right.nodeType != Number) {
            MidCode mul = MidCode(ExpMid);
            if (left.nodeType == Number) {
                AstNode tLeft = AstNode(Ident, getRegisterT());
                tLeft.val = left.val;
                left = tLeft;
                MidCode well = MidCode(ExpMid);
                well.opNum = 0;
                well.result = tLeft.str;
                well.op1 = to_string(left.val);
                pushMidCode(well);
            }
            mul.setOp1(left.str);
            mul.setOpRand(op.str);
            if (right.nodeType == Number) {
                AstNode tRight = AstNode(Ident, getRegisterT());
                tRight.val = right.val;
                right = tRight;
                MidCode well = MidCode(ExpMid);
                well.opNum = 0;
                well.result = tRight.str;
                well.op1 = to_string(right.val);
                pushMidCode(well);
            }
            mul.setOp2(right.str);
            string resultName = getRegisterT();
            mul.setResult(resultName);
            pushMidCode(mul);
            result = AstNode(Ident, resultName);
        } else {
            result = AstNode(Number);
        }
        if (op.nodeType == Multi) {
            result.setVal(left.val * right.val);
        } else if (op.nodeType == Div) {
            if (right.val != 0) {
                result.setVal(left.val / right.val);
            } else {
                result.setVal(0);
            }
        } else {
            if (right.val != 0) {
                result.setVal(left.val % right.val);
            } else {
                result.setVal(0);
            }
        }
        if (left.nodeType == Number && right.nodeType == Number) {
            result.str = to_string(result.val);
        }
        list.push_front(result);
//        cout << result.str << result.val << endl;
    }
//    SymbolItem item = SymbolItem(VarSymbol, list[0]);
//    currentLayer->addSymbol(item);
    return list[0];
}

AstNode unaryExpMid(AstNode *node) {
    node->setAllUsed();
    switch (node->nodeType) {
        case Exp: {
            return expMid(node);
        }
        case Number: {
//            cout << node->str<<endl;
            node->setVal(stoi(node->str));
//            node->setVal(1);
            return *node;
        }
        case Not: {
            MidCode exp = MidCode(ExpMid, 1);
            exp.setOpRand(node->str);
            AstNode var = expItemMid(&node->children[0]);
            exp.setOp1(var.str);
            string resultName = getRegisterT();
            exp.setResult(resultName);
            pushMidCode(exp);
            AstNode ident = AstNode(Ident, resultName);
            if (var.val == 0) {
                ident.setVal(1);
            } else {
                ident.setVal(0);
            }
//            SymbolItem item = SymbolItem(TmpVar, ident);
//            currentLayer->addSymbol(item);
            return ident;
        }
        case Plus: {
            MidCode exp = MidCode(ExpMid, 1);
            exp.setOpRand(node->str);
            AstNode var = expItemMid(&node->children[0]);
            exp.setOp1(var.str);
            string resultName = getRegisterT();
            exp.setResult(resultName);
            pushMidCode(exp);
            AstNode ident = AstNode(Ident, resultName);
            ident.setVal(var.val);
//            SymbolItem item = SymbolItem(TmpVar, ident);
//            currentLayer->addSymbol(item);
            return ident;
        }
        case Minus: {
            MidCode exp = MidCode(ExpMid, 1);
            exp.setOpRand(node->str);
            AstNode var = expItemMid(&node->children[0]);
            exp.setOp1(var.str);
            string resultName = getRegisterT();
            exp.setResult(resultName);
            pushMidCode(exp);
            AstNode ident = AstNode(Ident, resultName);
            ident.setVal(-1 * var.val);
//            SymbolItem item = SymbolItem(TmpVar, ident);
//            currentLayer->addSymbol(item);
            return ident;
        }
        case FuncCall: {
            //FuncParam
            MidCode returnVal = MidCode(PushVar, 1);
            returnVal.setOp1("$v0");
            returnVal.setResult(node->str);
            returnVal.setOpNum(-1);
            pushMidCode(returnVal);

            MidCode ra = MidCode(PushVar, 1);
            ra.setOp1("$ra");
            ra.setResult(node->str);
            ra.setOpNum(-1);
            pushMidCode(ra);
            if (!node->params.empty()) {
                for (int i = 0; i < node->params.size(); i++) {
                    MidCode push = MidCode(PushVar, 1);
                    AstNode var = expMid(&node->params[i]);
//                    if (var.dim == 0){
//                        push.setOp1(var.str);
//                    } else if (var.dim == 1){
//                        AstNode x = expItemMid(&var.params[0]);
//                        push.setOp1(var.str+"["+x.str+"]");
//                    } else {
//                        AstNode x = expItemMid(&var.params[0]);
//                        AstNode y = expItemMid(&var.params[1]);
//                        push.setOp1(var.str+"["+x.str+"]["+y.str+"]");
//                    }
                    push.setOp1(var.str);
                    push.setResult(node->str);
                    push.setOpNum(i);//对应第几个参数
                    pushMidCode(push);
                }
            }
            MidCode call = MidCode(FuncCallMid, 1);
            call.setOp1(node->str);
            call.setOpNum(node->params.size());
            pushMidCode(call);
            return retVal;
        }
        default:
            if (symTable.checkExistedSymbol(node)) {
                SymbolItem *sym = symTable.getExistedSymbol(node);
//                cout << sym->str << endl;
                if (node->dim == 0) {
                    node->setVal(sym->value);
                    return *node;
                } else if (node->dim == 1) {
                    int index = getAddress(&node->params[0]).val;
                    MidCode getArr = MidCode(ArrayGetMid);
                    getArr.result = getRegisterT();
                    getArr.setOp1(node->str + "[" + to_string(index) + "]");
                    getArr.setOp2(to_string(index));
                    getArr.setOpRand(node->str);
                    getArr.opNum = 1;
                    pushMidCode(getArr);
                    AstNode ans = AstNode(Ident);
                    ans.setVal(sym->params[index/4].value);
                    ans.str = getArr.result;
                    ans.dim = 1;
                    return ans;
                } else {
                    AstNode x = expMid(&node->params[0]);
                    AstNode y = expMid(&node->params[1]);
                    AstNode re = getAddress(&x, &y, sym->dim2);
                    int index = re.val;
                    MidCode getArr = MidCode(ArrayGetMid);
                    getArr.result = getRegisterT();
                    getArr.setOp1(node->str + "[" + to_string(index) + "]");
                    if (re.nodeType == Number) {
                        getArr.setOp2(to_string(index));
                        getArr.setOpNum(1);
                    } else {
                        getArr.setOp2(re.str);
                        getArr.setOpNum(2);
                    }
                    getArr.setOpRand(node->str);
                    pushMidCode(getArr);
                    AstNode ans = AstNode(Ident);
                    ans.setVal(sym->params[index/4].value);
                    ans.str = getArr.result;
                    ans.dim = 1;
                    return ans;
                }
            } else {
                node->setVal(0);
                return *node;
            }
    }
}

AstNode initialMid(AstNode *node) {
    if (node->params.size() == 1) {
        AstNode val = node->params[0].children[0];
        return expItemMid(&val);
    } else {
        for (int i = 0; i < node->params.size(); i++) {
            AstNode val = node->params[i];
            node->addChild(expMid(&val));
        }
        return *node;
    }
}

AstNode getAddress(AstNode *x) {
//    if (x->children[0].nodeType == Number) {
//        int n = stoi(x->children[0].str) * 4;
//        AstNode num = AstNode(Number, to_string(n));
//        num.setVal(n);
//        return num;
//    }
    AstNode tmp = AstNode(MulExp);
    AstNode four = AstNode(Number);
    four.str = "4";
    tmp.addChild(four);
    tmp.addChild(AstNode(Multi, "*"));
    tmp.addChild(*x);
    return expItemMid(&tmp);
}

AstNode getAddress(AstNode *x, AstNode *y, int dim2) {
    //arr[a][b] arr[x][y]
    //t1 = x * b
    //t2 = y + t1
    //arr[t2]
//    cout << dim2<<" "<<x->val << " "<< y->val<< endl;
    AstNode b = AstNode(Number);
    b.str = to_string(dim2);
    b.setVal(dim2);
    AstNode tmp1 = AstNode(MulExp);
    tmp1.addChild(*x);
    tmp1.addChild(AstNode(Multi, "*"));
    tmp1.addChild(b);
    AstNode tmp2 = AstNode(AddExp);
    tmp2.addChild(*y);
    tmp2.addChild(AstNode(Plus, "+"));
    tmp2.addChild(tmp1);
    AstNode tmp3 = AstNode(MulExp);
    AstNode four = AstNode(Number);
    four.str = "4";
    tmp3.addChild(four);
    tmp3.addChild(AstNode(Multi, "*"));
    tmp3.addChild(tmp2);
    return expItemMid(&tmp3);
}

string getRegisterT() {
    if (tmpVarCnt == 10) {
        tmpVarCnt = 0;
    }
    return "$t" + to_string(tmpVarCnt++);
}

MidCode::MidCode(MidCodeType t) {
    this->type = t;
}

MidCode::MidCode(MidCodeType t, int num) {
    this->type = t;
    this->opNum = num;
}

void MidCode::setOp1(string s) {
    this->op1 = std::move(s);
}

void MidCode::setOp2(string s) {
    this->op2 = std::move(s);
}

void MidCode::setOpNum(int num) {
    this->opNum = num;
}

void MidCode::setType(MidCodeType t) {
    this->type = t;
}

void MidCode::setResultType(int t) {
    this->resultType = t;
}

void MidCode::setResult(string s) {
    this->result = std::move(s);
}

string MidCode::toString() const {
    switch (type) {
        case ConstDeclMid:
            return "const int " + this->op1 + " = " + this->op2;
        case VarDeclMid:
            if (this->opNum == 2) {
                return "var int " + this->op1 + " = " + this->op2;
            } else {
                return "var int " + this->op1;
            }
        case ArrayDeclMid:
            if (this->opNum == 2) {
                return "arr int " + this->result + "[" + this->op1 + "][" + this->op2 + "]";
            } else {
                return "arr int " + this->result + "[" + this->op1 + "]";
            }
        case ArrayVarMid:
            if (this->opNum == 2) {
                return this->opRand + "[" + this->op1 + "][" + this->op2 + "] = " + this->result;
            } else {
                return this->opRand + "[" + this->op1 + "] = " + this->result;
            }
        case ArrayGetMid:
            return this->result + " = " + this->op1;
        case FuncDefMid:
            if (this->resultType == 0) {
                return "int " + this->op1 + "()";
            } else {
                return "void " + this->op1 + "()";
            }
        case FuncParamMid:
            if (this->opNum == 0) {
                return "para int " + this->opRand;
            } else if (this->opNum == 1) {
                return "para int " + this->opRand + "[]";
            } else {
                return "para int " + this->opRand + "[][" + this->op2 + "]";
            }
        case MainFuncMid:
            return this->op1;
        case ExpMid:
            if (this->opNum == 0) {
                return this->result + "=" + this->op1;
            } else if (this->opNum == 1) {
                return this->result + " = " + this->opRand + " " + this->op1;
            } else {
                return this->result + " = " + this->op1 + " " + this->opRand + " " + this->op2;
            }
        case GetIntMid:
            return "getint " + this->result;
        case PushVar:
            return "push " + this->op1;
        case FuncCallMid:
            return "call " + this->op1;
        case Label:
            return this->op1 + ":";
        case Goto:
            return "goto " + this->op1;
        case Cmp:
            return "cmp " + this->op1 + " " + this->op2;
        case Beq:
            return "beq " + this->op1;
        case Bne:
            return "bne " + this->op1;
        case Bgt:
            return "bgt " + this->op1;
        case Bge:
            return "bge " + this->op1;
        case Blt:
            return "blt " + this->op1;
        case Ble:
            return "ble " + this->op1;
        case Return:
            if (this->opNum == 0) {
                return "return";
            } else {
                return this->op1 + " " + this->op2;
            }
        case PrintfMid:
            if (this->opNum == 1) {
                return "print \"" + this->op1 + "\"";
            } else {
                return "print " + this->op2;
            }
    }
    return "";
}

void MidCode::setOpRand(string s) {
    this->opRand = std::move(s);
}


void pushMidCode(const MidCode &m) {
    midCodeTable.emplace_back(m);
//    cout <<m.toString() << endl;
}

void printAstNode(AstNode *tmp) {
    if (tmp->nodeType < 25) {
        cout << "type:" << getEnum(tmp->nodeType) << endl;
    } else {
        cout << "type:" << getEnum(tmp->nodeType)
             << "            str: " << tmp->str
             //                 << " paramSize: " << tmp->params.size()
             << endl;
        if (!tmp->params.empty()) {
            cout << "     params:" << tmp->params.size() << endl;
        }
    }
}

void printMidCode() {
    for (int i = 0; i < midCodeTable.size(); i++) {
        cout << midCodeTable[i].toString() << endl;
    }
}

string getEnum(NodeType type) {
    switch (type) {
        case ConstDeclList:
            return "ConstDeclList";
        case VarDeclList:
            return "VarDeclList";
        case FuncDefList:
            return "FuncDefList";
        case CompUnit:
            return "CompUnit";
        case ConstDecl:
            return "ConstDecl";
        case ConstInitVal:
            return "ConstInitVal";
        case VarDecl:
            return "VarDecl";
        case InitVal:
            return "InitVal";
        case FuncDef:
            return "FuncDef";
        case MainFuncDef:
            return "MainFuncDef";
        case FuncFParam:
            return "FuncFParam";
        case Block:
            return "Block";
        case Stmt:
            return "Stmt";
        case Exp:
            return "Exp";
        case Number:
            return "Number";
        case ConstExp:
            return "ConstExp";
        case Ident:
            return "Ident";
        case FormatString:
            return "FormatString";
        case IfCond:
            return "IfCond";
        case IfBody:
            return "IfBody";
        case WhileCond:
            return "WhileCond";
        case WhileBody:
            return "WhileBody";
        case MulExp:
            return "MulExp";
        case AddExp:
            return "AddExp";
        case EqExp:
            return "EqExp";
        case RelExp:
            return "RelExp";
        case LAndExp:
            return "LAndExp";
        case LOrExp:
            return "LOrExp";
        case ConstTK:
            return "ConstTK";
        case IntTK:
            return "IntTK";
        case VoidTK:
            return "VoidTK";
        case BreakTK:
            return "BreakTK";
        case ContinueTK:
            return "ContinueTK";
        case IfTK:
            return "IfTK";
        case ElseBody:
            return "ElseBody";
        case GetintTK:
            return "GetintTK";
        case PrintfTK:
            return "PrintfTK";
        case ReturnTK:
            return "ReturnTK";
        case WhileTK:
            return "WhileTK";
        case Not:
            return "Not";
        case Plus:
            return "Plus";
        case Minus:
            return "Minus";
        case Multi:
            return "Multi";
        case Div:
            return "Div";
        case Mod:
            return "Mod";
        case And:
            return "And";
        case Or:
            return "Or";
        case LSS:
            return "LSS";
        case LEQ:
            return "LEQ";
        case GRE:
            return "GRE";
        case GEQ:
            return "GEQ";
        case EQL:
            return "EQL";
        case NEQ:
            return "NEQ";
        case Assign:
            return "Assign";
        case FuncCall:
            return "FuncCall";
        default:
            return "";
    }
}