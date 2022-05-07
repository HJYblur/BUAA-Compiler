//
// Created by lemon on 2021/11/11.
//
#include <queue>
#include "iostream"
#include "midCode.h"
#include "deque"
#include "string"
#include "stack"

#define PRINT 1

using namespace std;
vector<MidCode> midCodeTable;
vector<SymbolItem> symTable;
int currentLayer = 0;
int tmpVarCnt = 0;
int ifLabelCnt = 1;
int whileLabelCnt = 1;
int condLabelCnt = 1;
int printStrCnt = 0;
bool isSingleCond = false;
bool countParam = false;
bool isComplex = false;
string funcName;
stack<int> whileStack;

void postOrderTraversal(AstNode node) {
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
        if (var.nodeType != Number) {
            constMid.setOp2(var.str);
            constMid.setOpRand(to_string(var.val));
//            cout << getEnum(var.nodeType)<<" " << constMid.opRand << endl;
            constMid.setResultType(1);
        } else {
            constMid.setOp2(to_string(var.val));
            constSymbol.setValue(var.val);
        }
        pushMidCode(constMid);
        constSymbol.setValue(var.val);
        addSymbol(constSymbol);
    } else {
        //数组
        SymbolItem arraySymbol = SymbolItem(ArraySymbol, ident);
        MidCode arrMid = MidCode(ArrayDeclMid);//arr[x][y]
        arrMid.setResult(ident.str);
        int xCnt = 0;
        int yCnt = 0;
        if (ident.dim > 0) {
            AstNode x = expMid(&ident.params[0]);
//            if (x.nodeType == Number) {
            arrMid.dim1 = x.val;
            arrMid.setOp1(to_string(x.val));
//            } else {
//                arrMid.dim1 = -1;
//                arrMid.setOp1(x.str);
//            }
            arrMid.setOpNum(1);
            arraySymbol.setDim1(x.val);
            arraySymbol.setDim(1);
            xCnt = x.val;
        }
        if (ident.dim > 1) {
            AstNode y = expMid(&ident.params[1]);
//            if (y.nodeType == Number) {
            arrMid.dim2 = y.val;
            arrMid.setOp2(to_string(y.val));
//            } else {
//                arrMid.dim2 = -1;
//                arrMid.setOp2(y.str);
//            }
            arrMid.setOpNum(2);
            arraySymbol.setDim2(y.val);
            arraySymbol.setDim(2);
            yCnt = y.val;
        }
        pushMidCode(arrMid);
//        for (int i = 0; i < node->children.size(); i++) {
//            cout << getEnum(node->children[i].nodeType) << endl;
//        }
        vector<AstNode> vars = initialMid(&node->children[4]).children;
//        cout << vars.size() << endl;
        if (vars.empty()) {
            AstNode content = initialMid(&node->children[4]);
//            cout << getEnum(content.nodeType) << content.val << endl;
            //符号表添加值
            SymbolItem var = SymbolItem(ConstSymbol);
            var.setValue(content.val);
            arraySymbol.addParam(var);
            //生成中间代码
            MidCode setArrayVar = MidCode(ArrayVarMid);
            setArrayVar.setOpRand(ident.str);//arr[i]=xxx;
            setArrayVar.setOp1(to_string(0));
            setArrayVar.setResult(content.str);
            if (ident.dim == 1) {
                setArrayVar.setOpNum(1);
            } else {
                setArrayVar.setOpNum(2);
                setArrayVar.setOp2("0");
            }
            if (content.nodeType != Number) {
                setArrayVar.setResultType(1);
            }
            pushMidCode(setArrayVar);
        } else {
            if (ident.dim == 1) {
//                cout << ident.str << endl;
                for (int i = 0; i < xCnt; i++) {
                    //数组每个分量的值
                    AstNode content = vars[i];
//                    cout << getEnum(content.nodeType) << content.val << endl;
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
        }
        addSymbol(arraySymbol);
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
//            cout <<ident.str <<getEnum(var.nodeType) <<var.val<< endl;
            if (var.nodeType != Number) {
                varMid.setOp2(var.str);
                varMid.setOpRand(to_string(var.val));
                varMid.setResultType(1);
                varSymbol.setValue(var.val);
            } else {
                varMid.setOp2(to_string(var.val));
                varSymbol.setValue(var.val);
            }
        } else {
            varMid.setOpNum(1);
        }
        pushMidCode(varMid);
        addSymbol(varSymbol);
    } else {
        //数组
        SymbolItem arraySymbol = SymbolItem(ArraySymbol, ident);
        MidCode arrMid = MidCode(ArrayDeclMid);//arr[x][y]
        arrMid.setResult(ident.str);
        int xCnt = 0;
        int yCnt = 0;
        if (ident.dim > 0) {
            AstNode x = expMid(&ident.params[0]);
//            if (x.nodeType == Number) {
            arrMid.dim1 = x.val;
            arrMid.setOp1(to_string(x.val));
//            cout << arrMid.result << " " << x.val << endl;
//            } else {
//                arrMid.dim1 = -1;
//                arrMid.setOp1(x.str);
//            }
            arrMid.setOpNum(1);
            arraySymbol.setDim1(x.val);
            arraySymbol.setDim(1);
            xCnt = x.val;
        }
        if (ident.dim > 1) {
            AstNode y = expMid(&ident.params[1]);
//            if (y.nodeType == Number) {
            arrMid.dim2 = y.val;
            arrMid.setOp2(to_string(y.val));
//            } else {
//                arrMid.dim2 = -1;
//                arrMid.setOp2(y.str);
//            }
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
            addSymbol(arraySymbol);
            return;
        }
        vector<AstNode> vars = initialMid(&node->children[3]).children;
        if (vars.empty()) {
            AstNode content = initialMid(&node->children[3]);
//                cout << getEnum(content.nodeType) << content.val<< endl;
            //符号表添加值
            SymbolItem var = SymbolItem(ConstSymbol);
            var.setValue(content.val);
            arraySymbol.addParam(var);
            //生成中间代码
            MidCode setArrayVar = MidCode(ArrayVarMid);
            setArrayVar.setOpRand(ident.str);//arr[i]=xxx;
            setArrayVar.setOp1(to_string(0));
            setArrayVar.setResult(content.str);
            if (ident.dim == 1) {
                setArrayVar.setOpNum(1);
            } else {
                setArrayVar.setOpNum(2);
                setArrayVar.setOp2("0");
            }
            if (content.nodeType != Number) {
                setArrayVar.setResultType(1);
            }
            pushMidCode(setArrayVar);
        } else {
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
        }
        addSymbol(arraySymbol);
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
//    cout << funcSymbol.str << " " << funcSymbol.layer << " " << currentLayer << endl;
    addSymbol(funcSymbol);

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
    }
        //数组
    else if (ident.dim == 1) {
        countParam = true;
        AstNode x = expItemMid(&ident.params[0]);
        countParam = false;
        para.setOp1("");
        para.setOpNum(1);
        paraSymbol.setDim1(x.val);
        paraSymbol.setDim(1);
    } else {
        countParam = true;
        AstNode x = expItemMid(&ident.params[0]);
//        cout << getEnum(ident.params[1].nodeType) << " " <<ident.params[1].val<< endl;
        AstNode y = expMid(&ident.params[1]);
        countParam = false;
        para.setOp1("");
        para.setOp2(to_string(y.val));
        para.dim2 = y.val;
        if (y.nodeType != Number) {
            para.resultType = 1;
        }
        paraSymbol.setDim1(x.val);
        paraSymbol.setDim2(y.val);
        paraSymbol.setDim(2);
    }
    pushMidCode(para);
    paraSymbol.setValue(0);
    paraSymbol.layer = currentLayer + 1;
    item->addParam(paraSymbol);
//    cout << "-------------------------------------"
//         << item->str << item->params[item->params.size() - 1].str
//         <<endl;
    addSymbol(paraSymbol);
}

void blockMid(AstNode *node) {
    //新建一层符号表
    currentLayer++;
    MidCode begin = MidCode(BlockBegin);
    pushMidCode(begin);
    node->setAllUsed();
    for (int i = 0; i < node->children.size(); i++) {
        toMidCode(&node->children[i]);
    }
//    cout << "In layer " << currentLayer << endl;
//    for (int i = 0; i < symTable.size(); i++) {
//        cout << "Whattttttttt " << symTable[i].str << " with layer:" << symTable[i].layer << endl;
//    }
    //回退符号表
    for (auto iter = symTable.begin(); iter != symTable.end();) {
        if (iter->layer == currentLayer) {
//            cout << "Now pop1: " << iter->str << " with layer " << iter->layer << endl;
            iter = symTable.erase(iter);
        } else {
            iter++;
        }
    }
//    cout << "In layer " << currentLayer << endl;
//    for (int i = 0; i < symTable.size(); i++) {
//        cout << "Nowwwwwww " << symTable[i].str << endl;
//    }
    currentLayer--;
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
            getInt.setResult(getTmpVar());
            getInt.setOp1("GetInt");
            getInt.setOpNum(1);
            pushMidCode(getInt);

            SymbolItem item = SymbolItem(Ident);
            item.setIsTmp();
            item.setDim(0);
            item.setString(getInt.result);
            addSymbol(item);

            MidCode getArr = MidCode(ArrayVarMid);
            AstNode x = expMid(&ident.params[0]);
            AstNode ans = getAddress(&x);
            getArr.setResult(getInt.result);
            getArr.setOpRand(ident.str);
            getArr.setOp1(ans.str);
            if (ans.nodeType != Number) {
                getArr.dim1 = 1;
            }
            getArr.setOpNum(1);
            getArr.setResultType(1);
            pushMidCode(getArr);
        } else {
            MidCode getInt = MidCode(GetIntMid);
            getInt.setResult(getTmpVar());
            getInt.setOp1("GetInt");
            getInt.setOpNum(1);
            pushMidCode(getInt);

            SymbolItem item = SymbolItem(Ident);
            item.setIsTmp();
            item.setDim(0);
            item.setString(getInt.result);
            addSymbol(item);

            int dim2 = 0;
            if (checkExistedSymbol(ident.str)) {
                SymbolItem *arr = getExistedSymbol(ident.str);
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
            if (ans.nodeType != Number) {
                getArr.dim1 = 1;
            }
            getArr.setResultType(1);
            pushMidCode(getArr);
        }
    }
        //ident = exp
    else {
        AstNode ident = node->children[0];
        AstNode expNode = expMid(&node->children[1]);
        bool flag = true;
        if (!checkExistedSymbol(ident.str)) {
            return;
        }
        SymbolItem *item = getExistedSymbol(ident.str);
        if (item->isFParam) {
            flag = false;
        }
//        cout << item->str << " " << ident.dim << endl;
        if (ident.dim == 0) {
            //符号表赋值
            if (flag) {
                item->setValue(expNode.val);
            }
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
//            cout <<  "111111111111111111111111111" << endl;
            //赋值
            AstNode tmp = getAddress(&ident.params[0]);
            int index = tmp.val;
//            cout<<item->params.size()<<endl;
//            if (flag) {
//                item->params[index].setValue(expNode.val);
//            }
            //中间代码
            MidCode arr = MidCode(ArrayVarMid);
            arr.setOpRand(ident.str);
            arr.setOp1(to_string(index));
            arr.setResult(expNode.str);
            arr.setOpNum(1);
            if (tmp.nodeType != Number) {
//                cout << "dim 1 : "<< tmp.str << getEnum(tmp.nodeType) << endl;
                arr.dim1 = 1;
                arr.setOp1(tmp.str);
            }
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
//            if (flag) {
//                item->params[ans.val / 4].setValue(expNode.val);
//            }
            //中间代码
            MidCode arr = MidCode(ArrayVarMid);
            arr.setOpRand(ident.str);
            arr.setOp1(ans.str);
            arr.setResult(expNode.str);
            arr.setOpNum(1);
            if (ans.nodeType != Number) {
//                cout << "dim 2 : "<< ans.str << getEnum(ans.nodeType) << endl;
                arr.dim1 = 1;
            }
            if (expNode.nodeType != Number) {
                arr.setResultType(1);
            }
            pushMidCode(arr);
        }
    }
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
        condInitMid(&ifCond.children[0], ifName, elseName, true);
    } else {
        condInitMid(&ifCond.children[0], ifName, endName, true);
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
    string condName = "while_cond" + to_string(whileLabelCnt);
    string bodyName = "while_body" + to_string(whileLabelCnt);
    string endName = "while_end" + to_string(whileLabelCnt);
    whileStack.push(whileLabelCnt);
    whileLabelCnt++;
    //whileCond
    MidCode condLabel = MidCode(Label, 1);
    condLabel.setOp1(condName);
    pushMidCode(condLabel);
    AstNode cond = node->children[0];
    condInitMid(&cond.children[0], bodyName, endName, true);
    //whileBody
    MidCode bodyLabel = MidCode(Label, 1);
    bodyLabel.setOp1(bodyName);
    pushMidCode(bodyLabel);
    AstNode whileBody = node->children[1];
    toMidCode(&whileBody.children[0]);
    MidCode go = MidCode(Goto, 1);
    go.setOp1(condName);
    pushMidCode(go);
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
    c.setOp1("while_cond" + to_string(cnt));
    pushMidCode(c);
}

void returnMid(AstNode *node) {
    node->setAllUsed();
    MidCode re = MidCode(Return);
    re.setOp1("return");
    AstNode retVal = AstNode(Ident, "$v0");
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
            re.setOpNum(2);
            retVal.setVal(exp.val);
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
    queue<MidCode> returnList;
    for (int i = 0; i < s.size(); i++) {
        if (s[i] == '%' && s[i + 1] == 'd') {
            if (!t.empty()) {
                MidCode str = MidCode(PrintfMid, 1);
                str.setOp1(t);
                str.setOp2(to_string(printStrCnt++));
                returnList.push(str);
            }
            MidCode ident = MidCode(PrintfMid, 2);
//            cout << node->params[cnt].str << endl;
            AstNode exp = expItemMid(&node->params[cnt++]);
//            cout << exp.str << endl;
            ident.setOp2(exp.str);
            if (exp.nodeType != Number) {
                ident.setResultType(1);
            }
            returnList.push(ident);
            t = "";
            i++;
        } else if (s[i] != '"') {
            t += s[i];
        }
    }
    MidCode str = MidCode(PrintfMid, 1);
    str.setOp1(t);
    str.setOp2(to_string(printStrCnt++));
    returnList.push(str);
    while (!returnList.empty()) {
        pushMidCode(returnList.front());
        returnList.pop();
    }
}

bool condMid(AstNode *node, const string &ifBody, const string &elseBody, bool flag) {
    node->setAllUsed();
//    cout << getEnum(node->nodeType) << node->str << endl;
    switch (node->nodeType) {
        case LOrExp:
            isSingleCond = false;
            return lOrMid(node, ifBody, elseBody);
        case LAndExp:
            isSingleCond = false;
            return lAndMid(node, ifBody, elseBody);
        case EqExp:
            return eqMid(node, ifBody, elseBody, flag);
        case RelExp:
            return relMid(node, ifBody, elseBody, flag);
        default:
            return zeroMid(node, ifBody, elseBody, flag);
    }
}

bool condInitMid(AstNode *node, const string &ifBody, const string &elseBody, bool flag) {
    node->setAllUsed();
    MidCode go = MidCode(Goto);
    go.setOp1(elseBody);
//    cout << getEnum(node->nodeType) << node->str << endl;
    switch (node->nodeType) {
        case LOrExp:
            isSingleCond = false;
            return lOrMid(node, ifBody, elseBody);
        case LAndExp:
            isSingleCond = false;
            return lAndMid(node, ifBody, elseBody);
        case EqExp:
            eqMid(node, ifBody, elseBody, flag);
            pushMidCode(go);
            return true;
        case RelExp:
            relMid(node, ifBody, elseBody, flag);
            pushMidCode(go);
            return true;
        default:
            zeroMid(node, ifBody, elseBody, flag);
            pushMidCode(go);
            return true;
    }
}

bool lOrMid(AstNode *node, const string &ifBody, const string &elseBody) {
    node->setAllUsed();
    string currentElse;
    for (int i = 0; i < node->children.size(); i++) {
        AstNode child = node->children[i];
        if (child.nodeType == Or) {
            MidCode label = MidCode(Label);
            label.setOp1(currentElse);
            pushMidCode(label);
            condLabelCnt++;
            continue;
        } else {
            currentElse = "OrLabel" + to_string(condLabelCnt);
            if (i == node->children.size() - 1) {
                condMid(&child, ifBody, elseBody, true);
            } else {
                condMid(&child, ifBody, currentElse, true);
            }
        }
    }
    MidCode go = MidCode(Goto, 1);
    go.setOp1(elseBody);
    pushMidCode(go);
    return true;
}

bool lAndMid(AstNode *node, const string &ifBody, const string &elseBody) {
    node->setAllUsed();
    string currentIf;
    for (int i = 0; i < node->children.size(); i++) {
        AstNode child = node->children[i];
        if (child.nodeType == And) {
            MidCode label = MidCode(Label);
            label.setOp1(currentIf);
            pushMidCode(label);
            condLabelCnt++;
            continue;
        } else {
            currentIf = "AndLabel" + to_string(condLabelCnt);
            if (i == node->children.size() - 1) {
                condMid(&child, ifBody, elseBody, false);
            } else {
                condMid(&child, currentIf, elseBody, false);
            }
        }
    }
    MidCode go = MidCode(Goto, 1);
    go.setOp1(ifBody);
    pushMidCode(go);
    return false;
}

bool eqMid(AstNode *node, const string &ifBody, const string &elseBody, bool flag) {
    node->setAllUsed();

    deque<AstNode> list(node->children.begin(), node->children.end());

    while (list.size() > 3) {
        AstNode left = list.front();
        if (left.nodeType == RelExp) {
            left = parseRelMid(&list.front(), ifBody, elseBody, flag);
        } else {
            left = expItemMid(&list.front());
        }
        list.pop_front();
        if (left.nodeType == Number) {
            AstNode tLeft = AstNode(Ident, getTmpVar());
            tLeft.val = left.val;
            left = tLeft;

            SymbolItem item = SymbolItem(tLeft);
            item.setIsTmp();
            item.setDim(0);
            addSymbol(item);

            MidCode well = MidCode(ExpMid);
            well.opNum = 0;
            well.result = tLeft.str;
            well.op1 = to_string(left.val);
            pushMidCode(well);
        }
        AstNode op = list.front();
        list.pop_front();
        AstNode right = list.front();
        if (right.nodeType == RelExp) {
            right = parseRelMid(&list.front(), ifBody, elseBody, flag);
        } else {
            right = expItemMid(&list.front());
        }
        list.pop_front();
        AstNode ans = AstNode(Ident, getTmpVar());

        SymbolItem item = SymbolItem(ans);
        item.setIsTmp();
        addSymbol(item);

        MidCode compare = MidCode(Compare);
        compare.setOp1(left.str);
        compare.setOp2(right.str);
        compare.setResult(ans.str);
        if (right.nodeType == Number) {
            compare.setResultType(0);
        } else {
            compare.setResultType(1);
        }

        if (op.nodeType == EQL) {
            compare.setOpRand("seq");
        } else {
            compare.setOpRand("sne");
        }
        pushMidCode(compare);

        list.push_front(ans);
    }

    AstNode left = list.front();
    if (left.nodeType == RelExp) {
        left = parseRelMid(&list.front(), ifBody, elseBody, flag);
    } else {
        left = expItemMid(&list.front());
    }
    list.pop_front();
    if (left.nodeType == Number) {
        AstNode tLeft = AstNode(Ident, getTmpVar());
        tLeft.val = left.val;
        left = tLeft;

        SymbolItem item = SymbolItem(tLeft);
        item.setIsTmp();
        item.setDim(0);
        addSymbol(item);

        MidCode well = MidCode(ExpMid);
        well.opNum = 0;
        well.result = tLeft.str;
        well.op1 = to_string(left.val);
        pushMidCode(well);
    }
    AstNode op = list.front();
    list.pop_front();
    AstNode right = list.front();
    if (right.nodeType == RelExp) {
        right = parseRelMid(&list.front(), ifBody, elseBody, flag);
    } else {
        right = expItemMid(&list.front());
    }
    list.pop_front();

    MidCode cmp = MidCode(Cmp);//cmp a==b
    cmp.setOp1(left.str);
    cmp.setOp2(right.str);
    if (right.nodeType == Number) {
        cmp.setResultType(0);
    } else {
        cmp.setResultType(1);
    }

    if (flag) {
        //Or
        if (op.nodeType == EQL) {
            cmp.setResult(ifBody);
            cmp.setOpRand("beq");
        } else {
            cmp.setResult(ifBody);
            cmp.setOpRand("bne");
        }
    } else {
        //And
        if (op.nodeType == EQL) {
            cmp.setResult(elseBody);
            cmp.setOpRand("bne");
        } else {
            cmp.setResult(elseBody);
            cmp.setOpRand("beq");
        }
    }
    pushMidCode(cmp);
    return true;
}

bool relMid(AstNode *node, const string &ifBody, const string &elseBody, bool flag) {
    node->setAllUsed();

    deque<AstNode> list(node->children.begin(), node->children.end());

    while (list.size() > 3) {
        AstNode left = expItemMid(&list.front());
        list.pop_front();
        if (left.nodeType == Number) {
            AstNode tLeft = AstNode(Ident, getTmpVar());
            tLeft.val = left.val;
            left = tLeft;

            SymbolItem item = SymbolItem(tLeft);
            item.setIsTmp();
            item.setDim(0);
            addSymbol(item);

            MidCode well = MidCode(ExpMid);
            well.opNum = 0;
            well.result = tLeft.str;
            well.op1 = to_string(left.val);
            pushMidCode(well);
        }
        AstNode op = list.front();
        list.pop_front();
        AstNode right = expItemMid(&list.front());
        list.pop_front();

        AstNode ans = AstNode(Ident, getTmpVar());

        SymbolItem item = SymbolItem(ans);
        item.setIsTmp();
        addSymbol(item);

        MidCode compare = MidCode(Compare);
        compare.setOp1(left.str);
        compare.setOp2(right.str);
        compare.setResult(ans.str);
        if (right.nodeType == Number) {
            compare.setResultType(0);
        } else {
            compare.setResultType(1);
        }

        if (op.nodeType == GRE) {
            compare.setOpRand("sgt");
        } else if (op.nodeType == LSS) {
            compare.setOpRand("slt");
        } else if (op.nodeType == GEQ) {
            compare.setOpRand("sge");
        } else {
            compare.setOpRand("sle");
        }
        pushMidCode(compare);

        list.push_front(ans);
    }

    AstNode left = expItemMid(&list.front());
    list.pop_front();
    if (left.nodeType == Number) {
        AstNode tLeft = AstNode(Ident, getTmpVar());
        tLeft.val = left.val;
        left = tLeft;

        SymbolItem item = SymbolItem(tLeft);
        item.setIsTmp();
        item.setDim(0);
        addSymbol(item);

        MidCode well = MidCode(ExpMid);
        well.opNum = 0;
        well.result = tLeft.str;
        well.op1 = to_string(left.val);
        pushMidCode(well);
    }
    AstNode op = list.front();
    list.pop_front();
    AstNode right = expItemMid(&list.front());
    list.pop_front();

    MidCode cmp = MidCode(Cmp);//cmp a>b a<b a>=b a<=b
    cmp.setOp1(left.str);
    cmp.setOp2(right.str);
    if (right.nodeType == Number) {
        cmp.setResultType(0);
    } else {
        cmp.setResultType(1);
    }

    if (flag) {
        //Or
        if (op.nodeType == GRE) {
            cmp.setResult(ifBody);
            cmp.setOpRand("bgt");
        } else if (op.nodeType == LSS) {
            cmp.setResult(ifBody);
            cmp.setOpRand("blt");
        } else if (op.nodeType == GEQ) {
            cmp.setResult(ifBody);
            cmp.setOpRand("bge");
        } else {
            cmp.setResult(ifBody);
            cmp.setOpRand("ble");
        }
    } else {
        //And
        if (op.nodeType == GRE) {
            cmp.setResult(elseBody);
            cmp.setOpRand("ble");
        } else if (op.nodeType == LSS) {
            cmp.setResult(elseBody);
            cmp.setOpRand("bge");
        } else if (op.nodeType == GEQ) {
            cmp.setResult(elseBody);
            cmp.setOpRand("blt");
        } else {
            cmp.setResult(elseBody);
            cmp.setOpRand("bgt");
        }
    }
    pushMidCode(cmp);
    return true;
}

AstNode parseRelMid(AstNode *node, const string &ifBody, const string &elseBody, bool flag) {
    node->setAllUsed();

    deque<AstNode> list(node->children.begin(), node->children.end());

    while (list.size() > 1) {
        AstNode left = expItemMid(&list.front());
        list.pop_front();
        if (left.nodeType == Number) {
            AstNode tLeft = AstNode(Ident, getTmpVar());
            tLeft.val = left.val;
            left = tLeft;

            SymbolItem item = SymbolItem(tLeft);
            item.setIsTmp();
            item.setDim(0);
            addSymbol(item);

            MidCode well = MidCode(ExpMid);
            well.opNum = 0;
            well.result = tLeft.str;
            well.op1 = to_string(left.val);
            pushMidCode(well);
        }
        AstNode op = list.front();
        list.pop_front();
        AstNode right = expItemMid(&list.front());
        list.pop_front();

        AstNode ans = AstNode(Ident, getTmpVar());

        SymbolItem item = SymbolItem(ans);
        item.setIsTmp();
        item.setDim(0);
        addSymbol(item);

        MidCode compare = MidCode(Compare);
        compare.setOp1(left.str);
        compare.setOp2(right.str);
        compare.setResult(ans.str);
        if (right.nodeType == Number) {
            compare.setResultType(0);
        } else {
            compare.setResultType(1);
        }

        if (op.nodeType == GRE) {
            compare.setOpRand("sgt");
        } else if (op.nodeType == LSS) {
            compare.setOpRand("slt");
        } else if (op.nodeType == GEQ) {
            compare.setOpRand("sge");
        } else {
            compare.setOpRand("sle");
        }
        pushMidCode(compare);

        list.push_front(ans);
    }
    return list[0];
}

bool zeroMid(AstNode *node, const string &ifName, const string &elseName, bool flag) {
    AstNode ans = expItemMid(node);
    if (ans.nodeType == Number) {
        if (ans.val == 0) {
            MidCode go = MidCode(Goto);
            go.setOp1(elseName);
            pushMidCode(go);
        } else {
            MidCode go = MidCode(Goto);
            go.setOp1(ifName);
            pushMidCode(go);
        }
        return true;
    }
    MidCode cmp = MidCode(Cmp);
    cmp.setOp1(ans.str);
    cmp.setOp2("0");
    if (flag) {
        cmp.setOpRand("bne");
        cmp.setResult(ifName);
    } else {
        cmp.setOpRand("beq");
        cmp.setResult(elseName);
    }
    pushMidCode(cmp);
//    MidCode go = MidCode(Goto);
//    go.setOp1(elseName);
//    pushMidCode(go);
    return true;
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
            if (left.nodeType == Number) {
                AstNode tLeft = AstNode(Ident, getTmpVar());
                tLeft.val = left.val;
                left = tLeft;

                SymbolItem item = SymbolItem(tLeft);
                item.setIsTmp();
                item.setDim(0);
                addSymbol(item);

                MidCode well = MidCode(ExpMid);
                well.opNum = 0;
                well.result = tLeft.str;
                well.op1 = to_string(left.val);
                pushMidCode(well);
                plus.setResultType(0);
            }
            plus.setOp1(left.str);

            plus.setOpRand(op.str);

            if (right.nodeType == Number) {
                AstNode tRight = AstNode(Ident, getTmpVar());
                tRight.val = right.val;
                right = tRight;

                SymbolItem item = SymbolItem(tRight);
                item.setIsTmp();
                item.setDim(0);
                addSymbol(item);

                MidCode well = MidCode(ExpMid);
                well.opNum = 0;
                well.result = tRight.str;
                well.op1 = to_string(right.val);
                pushMidCode(well);
                plus.setResultType(1);
            }
            plus.setOp2(right.str);

            if (left.nodeType != Number && right.nodeType != Number) {
                plus.setResultType(2);
            }

            string resultName = getTmpVar();
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
        } else {
            SymbolItem item = SymbolItem(result);
            item.setIsTmp();
            item.setDim(0);
            addSymbol(item);
        }
        list.push_front(result);
//        cout << result.str << result.val << endl;
    }
//    SymbolItem item = SymbolItem(VarSymbol, list[0]);
//     addSymbol(item);
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
//        cout << left.str << " " << op.str << " " << right.str << endl;
        AstNode result;
        if (left.nodeType != Number || right.nodeType != Number) {
            MidCode mul = MidCode(ExpMid);
            if (left.nodeType == Number) {
                AstNode tLeft = AstNode(Ident, getTmpVar());
                tLeft.val = left.val;
                left = tLeft;

                SymbolItem item = SymbolItem(tLeft);
                item.setIsTmp();
                item.setDim(0);
                addSymbol(item);

                MidCode well = MidCode(ExpMid);
                well.opNum = 0;
                well.result = tLeft.str;
                well.op1 = to_string(left.val);
                pushMidCode(well);

                mul.setResultType(0);
            }
            mul.setOp1(left.str);
            mul.setOpRand(op.str);
            if (right.nodeType == Number) {
                AstNode tRight = AstNode(Ident, getTmpVar());
                tRight.val = right.val;
                right = tRight;

                SymbolItem item = SymbolItem(tRight);
                item.setIsTmp();
                item.setDim(0);
                addSymbol(item);

                MidCode well = MidCode(ExpMid);
                well.opNum = 0;
                well.result = tRight.str;
                well.op1 = to_string(right.val);
                pushMidCode(well);
                mul.setResultType(1);
            }
            mul.setOp2(right.str);

            if (left.nodeType != Number && right.nodeType != Number) {
                mul.setResultType(2);
            }

            string resultName = getTmpVar();
            mul.setResult(resultName);
            pushMidCode(mul);
            result = AstNode(Ident, resultName);
        } else {
            result = AstNode(Number);
        }
        if (op.nodeType == Multi) {
//            cout << left.str<<left.val << " " << right.val<<endl;
            result.setVal(left.val * right.val);
        } else if (op.nodeType == Div) {
//            cout << left.str<<left.val << " " << right.val<<endl;
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
        } else {
            SymbolItem item = SymbolItem(result);
            item.setIsTmp();
            item.setDim(0);
            addSymbol(item);
        }
//        cout << result.str<<" "<<result.val << endl;
        list.push_front(result);
//        cout << result.str << result.val << endl;
    }
//    SymbolItem item = SymbolItem(VarSymbol, list[0]);
//     addSymbol(item);
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
            if (var.nodeType == Number) {
                exp.setResultType(0);
            } else {
                exp.setResultType(1);
            }
            string resultName = getTmpVar();
            exp.setResult(resultName);
            pushMidCode(exp);

            AstNode ident = AstNode(Ident, resultName);
            if (var.val == 0) {
                ident.setVal(1);
            } else {
                ident.setVal(0);
            }

            SymbolItem item = SymbolItem(Ident);
            item.setIsTmp();
            item.setString(resultName);
            item.setValue(ident.val);
            item.setDim(0);
            addSymbol(item);
//            SymbolItem item = SymbolItem(TmpVar, ident);
//             addSymbol(item);
            return ident;
        }
        case Plus: {
            MidCode exp = MidCode(ExpMid, 1);
            exp.setOpRand(node->str);
            AstNode var = expItemMid(&node->children[0]);
            exp.setOp1(var.str);
            if (var.nodeType == Number) {
                exp.setResultType(0);
            } else {
                exp.setResultType(1);
            }
            string resultName = getTmpVar();
            exp.setResult(resultName);
            pushMidCode(exp);
            AstNode ident = AstNode(Ident, resultName);
            ident.setVal(var.val);

            SymbolItem item = SymbolItem(Ident);
            item.setIsTmp();
            item.setString(resultName);
            item.setValue(ident.val);
            item.setDim(0);
            addSymbol(item);
//            SymbolItem item = SymbolItem(TmpVar, ident);
//             addSymbol(item);
            return ident;
        }
        case Minus: {
            MidCode exp = MidCode(ExpMid, 1);
            exp.setOpRand(node->str);
            AstNode var = expItemMid(&node->children[0]);
            exp.setOp1(var.str);
            if (var.nodeType == Number) {
                exp.setResultType(0);
            } else {
                exp.setResultType(1);
            }
            string resultName = getTmpVar();
            exp.setResult(resultName);
            pushMidCode(exp);
            AstNode ident = AstNode(Ident, resultName);
            ident.setVal(-1 * var.val);

            SymbolItem item = SymbolItem(Ident);
            item.setIsTmp();
            item.setString(resultName);
            item.setValue(ident.val);
            item.setDim(0);
            addSymbol(item);
//            SymbolItem item = SymbolItem(TmpVar, ident);
//             addSymbol(item);
            return ident;
        }
        case FuncCall: {
            //FuncParam
            queue<MidCode> params;
            SymbolItem *func = getExistedSymbol(node->str);
//            cout << "Func is :" << node->str << " ,with size: " << func->params.size() << endl;
            if (!func->params.empty()) {
//                cout << node->params.size() << " " << func->params.size() << endl;
                for (int i = 0; i < node->params.size(); i++) {
                    func = getExistedSymbol(node->str);
//                    cout << i << func->params[i].str << endl;
                    MidCode push = MidCode(PushVar, 1);
                    if (func->params[i].dim == 1) {
                        isComplex = true;
                    }
                    AstNode var = expMid(&node->params[i]);
                    isComplex = false;
                    push.setOp1(var.str);
                    push.setResult(node->str);
                    push.setOpNum(var.dim);
                    if (var.nodeType != Number) {
                        push.resultType = 1;
                    }
                    push.paraNumber = i;//对应第几个参数
                    params.push(push);
                }
            }
            while (!params.empty()) {
                pushMidCode(params.front());
                params.pop();
            }
            MidCode call = MidCode(FuncCallMid, 1);
            call.setOp1(node->str);
            call.setOpNum(node->params.size());
            pushMidCode(call);

            MidCode ass = MidCode(ExpMid, 0);
            ass.setResult(getTmpVar());
            ass.setOp1("$v0");
            ass.setResultType(1);
            pushMidCode(ass);

            SymbolItem item = SymbolItem(Ident);
            item.setIsTmp();
            item.setString(ass.result);
            item.setDim(0);
            addSymbol(item);

            AstNode re = AstNode(Ident, ass.result);
            return re;
        }
        default:
            if (checkExistedSymbol(node->str)) {
                SymbolItem *sym = getExistedSymbol(node->str);
                bool flag = true;
                if (sym->isFParam) {
                    flag = false;
                }
//                cout << sym->str << endl;
                if (node->dim == 0) {
                    if (flag) {
//                        cout <<node->str <<  sym->value << endl;
                        node->setVal(sym->value);
                    }
                    return *node;
                } else if (node->dim == 1) {
                    AstNode re = getAddress(&node->params[0]);
                    int index = re.val;
                    MidCode getArr = MidCode(ArrayGetMid);
                    getArr.result = getTmpVar();
                    if (re.nodeType == Number) {
                        getArr.setOp1(to_string(re.val));
                        getArr.resultType = 0;
                    } else {
                        getArr.setOp1(re.str);
                        getArr.resultType = 1;
                    }
                    getArr.setOpRand(node->str);//#tmp = node[index];
                    if (isComplex) {
                        getArr.opNum = 2;
//                        cout << sym->str << " " << node->str << endl;
                    } else {
                        getArr.opNum = 1;
                    }
                    pushMidCode(getArr);

                    SymbolItem item = SymbolItem(Ident);
                    item.setIsTmp();
                    item.setString(getArr.result);
                    item.setDim(0);

                    sym = getExistedSymbol(node->str);
                    AstNode ans = AstNode(Ident);
                    if (flag) {
                        ans.setVal(sym->params[index / 4].value);
                    } else {
                        ans.setVal(0);
                    }
                    ans.str = getArr.result;
                    ans.dim = 0;

                    //TODO
                    addSymbol(item);
                    return ans;
                } else {
                    AstNode x = expMid(&node->params[0]);
                    AstNode y = expMid(&node->params[1]);
                    AstNode re = getAddress(&x, &y, sym->dim2);
                    int index = re.val;
                    MidCode getArr = MidCode(ArrayGetMid);
                    getArr.result = getTmpVar();
                    if (re.nodeType == Number) {
                        getArr.setOp1(to_string(re.val));
                        getArr.resultType = 0;
                    } else {
                        getArr.setOp1(re.str);
                        getArr.resultType = 1;
                    }
                    getArr.setOpRand(node->str);//#tmp = node[index];
                    getArr.opNum = 1;
                    pushMidCode(getArr);

                    SymbolItem item = SymbolItem(Ident);
                    item.setIsTmp();
                    item.setString(getArr.result);
                    item.setDim(0);

                    sym = getExistedSymbol(node->str);
//                    cout << "here 1" <<sym->str << " " << sym->dim<< endl;
                    AstNode ans = AstNode(Ident);
                    if (flag) {
                        if (sym->dim == 0) {
                            ans.setVal(sym->value);
                        } else {
                            ans.setVal(sym->params[index / 4].value);
                        }
                    } else {
                        ans.setVal(0);
                    }
//                    cout << "here 2" << endl;
                    ans.str = getArr.result;
                    ans.dim = 0;
                    //TODO
                    addSymbol(item);
                    return ans;
                }
            } else {
                return *node;
            }
    }
}

AstNode initialMid(AstNode *node) {
    if (node->params.size() == 1) {
        AstNode val = node->params[0].children[0];
//        cout << getEnum(val.nodeType) << val.str << endl;
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

//string getTmpVar() {
//    if (tmpVarCnt == 8) {
//        tmpVarCnt = 0;
//    }
//    return "$t" + to_string(tmpVarCnt++);
//}
string getTmpVar() {
    return "#" + to_string(tmpVarCnt++);
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

bool checkExistedSymbol(const string &name) {
    for (int i = symTable.size() - 1; i >= 0; i--) {
        if (symTable[i].str == name) {
            return true;
        }
    }
    return false;
}

SymbolItem *getExistedSymbol(const string &name) {
//    cout<< "==================================" << endl;
    for (int i = symTable.size() - 1; i >= 0; i--) {
//        cout << symTable[i].str << endl;
        if (symTable[i].str == name) {
            return &symTable[i];
        }
    }
    cout << "Sorry, I can't find the sym:" << name << endl;
    return nullptr;
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
            return this->result + " = " + this->opRand + "[" + this->op1 + "]";
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
                return "para int " + this->opRand + "[" + this->op1 + "]";
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
            return this->opRand + " " + this->op1 + "," + this->op2 + "," + this->result;
        case Compare:
            return this->opRand + " " + this->result + "," + this->op1 + "," + this->op2;
//        case Beq:
//            return "beq " + this->op1;
//        case Bne:
//            return "bne " + this->op1;
//        case Bgt:
//            return "bgt " + this->op1;
//        case Bge:
//            return "bge " + this->op1;
//        case Blt:
//            return "blt " + this->op1;
//        case Ble:
//            return "ble " + this->op1;
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
        case BlockBegin:
            return "------------------------BlockBegin------------------------";
        case BlockEnd:
            return "------------------------BlockEnd------------------------";
    }
    return "";
}

void MidCode::setOpRand(string s) {
    this->opRand = std::move(s);
}


void pushMidCode(const MidCode &m) {
    if (countParam) {
        return;
    }
    midCodeTable.emplace_back(m);
    if (PRINT) {
        cout << m.toString() << endl;
    }
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

void addSymbol(SymbolItem item) {
//    cout << "Add symbol: " << item.str <<" in layer: " << item.layer << endl;
    symTable.emplace_back(item);
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
        case Unknown:
            return "UnKnown";
        default:
            return "";
    }
}