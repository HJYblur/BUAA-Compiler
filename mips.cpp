//
// Created by lemon on 2021/11/18.
//
#include "iostream"
#include "midCode.h"
#include "mips.h"
#include "globalVar.h"
#include "stack"
#include "VarControl.h"

#define PRINT 0

using namespace std;
vector<string> mipsTable;
stack<int> preShift;
vector<VarControl> varControl;
int controlPoint = 0;
int currentShift = 0;
bool isJr = false;
bool isMain = false;
string syscall = "\tsyscall";

int getMips() {
//    ofstream mipsFile(R"(H:\3rd-1stsemi\compile\mips\test.asm)", ios::out);
    ofstream mipsFile("mips.txt", ios::out);
    if (!mipsFile) {
        cout << "error opening mips file." << endl;
        return 0;
    }
    //.data
    pushMips(".data");
    int i = 0;
    //global Var
    while (midCodeTable[i].type != FuncDefMid && midCodeTable[i].type != MainFuncMid) {
        MidCode *midCode = &midCodeTable[i++];
        switch (midCode->type) {
            case ConstDeclMid:
                constGlobalDeclMips(midCode);
                break;
            case VarDeclMid:
                varGlobalDeclMips(midCode);
                break;
            case ArrayDeclMid:
                arrayGlobalDeclMips(midCode);
                break;
        }
    }
    int point = i;
    //string
    for (int i = 0; i < midCodeTable.size(); i++) {
        MidCode midCode = midCodeTable[i];
        if (midCode.type == PrintfMid && midCode.opNum == 1) {
            printStrMips(&midCode);
        }
    }
    //text
    pushMips(".text");
    //initial array
    for (int i = 0; i < point; i++) {
        MidCode midCode = midCodeTable[i];
        if (midCode.type == ArrayVarMid) {
            arrayVarMips(&midCode);
        } else if (midCode.type == ExpMid) {
            expMips(&midCode);
        } else if (midCode.type == ArrayGetMid) {
            arrayGetMips(&midCode);
        }
    }
    pushMips("\tj main");
    //exp
    for (int i = point; i < midCodeTable.size(); i++) {
        MidCode midCode = midCodeTable[i];
        toMips(&midCode);
    }
    pushMips("\tli $v0,10");
    pushMips(syscall);
    printMips(mipsFile);
    return 0;
}

void toMips(MidCode *midCode) {
//    cout << midCode->type << endl;
    switch (midCode->type) {
        case ConstDeclMid:
            constDeclMips(midCode);
            break;
        case VarDeclMid:
            varDeclMips(midCode);
            break;
        case ArrayVarMid:
            arrayVarMips(midCode);
            break;
        case ArrayDeclMid:
            arrayDeclMips(midCode);
            break;
        case ArrayGetMid:
            arrayGetMips(midCode);
            break;
        case FuncDefMid:
            isJr = false;
            funcDefMips(midCode);
            break;
        case FuncParamMid:
//            cout << "param" << endl;
            funcParamMips(midCode);
            break;
        case FuncEndMid:
            funcEndMips(midCode);
            break;
        case PushVar:
            pushVarMips(midCode);
            break;
        case FuncCallMid:
            funcCallMips(midCode);
            break;
        case MainFuncMid:
            mainFuncMips();
            break;
        case ExpMid:
            expMips(midCode);
            break;
        case PrintfMid:
            printMips(midCode);
            break;
        case GetIntMid:
            getIntMips(midCode);
            break;
        case Label:
            labelMips(midCode);
            break;
        case Goto:
            gotoMips(midCode);
            break;
        case Cmp:
            cmpMips(midCode);
            break;
        case Compare:
            compareMips(midCode);
            break;
        case Return:
            returnMips(midCode);
            break;
        case BlockBegin:
//            cout <<"blockBegin" << endl;
            blockBegin();
            break;
        case BlockEnd:
//            cout <<"blockEnd" << endl;
            blockEnd();
            break;
    }
}

void constGlobalDeclMips(MidCode *midCode) {
//    cout << midCode->op1 << endl;
    VarControl item = VarControl(midCode->op1);
    item.isGlobal = true;
    if (midCode->resultType == 0) {
        string ans = "\t" + midCode->op1 + ":.word " + midCode->op2;
        pushMips(ans);
    } else {
        string ans = "\t" + midCode->op1 + ":.word " + midCode->opRand;
        pushMips(ans);
    }
    controlPush(item);
}

void constDeclMips(MidCode *midCode) {
    VarControl item = VarControl(midCode->op1);
    if (midCode->resultType == 0) {
        pushMips("\tli $s0," + midCode->op2);
    } else {
        if (checkVar(midCode->op2)) {
            VarControl *var = searchVar(midCode->op2);
            getVar(*var, "$s0");
        } else {
            cout << "I can't find symbol " << midCode->op2 << endl;
        }
    }
    storeVar(&item, "$s0");
    controlPush(item);
}

void varGlobalDeclMips(MidCode *midCode) {
    VarControl item = VarControl(midCode->op1);
    item.isGlobal = true;
    if (midCode->opNum == 2) {
        if (midCode->resultType == 0) {
            string ans = "\t" + midCode->op1 + ":.word " + midCode->op2;
            pushMips(ans);
        } else {
            string ans = "\t" + midCode->op1 + ":.word " + midCode->opRand;
            pushMips(ans);
        }
    } else {
        string ans = "\t" + midCode->op1 + ":.word " + "0";
        pushMips(ans);
    }
    controlPush(item);
}

void varDeclMips(MidCode *midCode) {
    VarControl item = VarControl(midCode->op1);
    if (midCode->opNum == 2) {
        //有初值
        if (midCode->resultType == 0) {
            pushMips("\tli $s0," + midCode->op2);
        } else {
            if (checkVar(midCode->op2)) {
                VarControl *var = searchVar(midCode->op2);
                getVar(*var, "$s0");
            } else {
                cout << "I can't find symbol " << midCode->op2 << endl;
            }
        }
        storeVar(&item, "$s0");
    } else {
        //无初值
        pushMips("\tli $s0,0");
        storeVar(&item, "$s0");
    }
    controlPush(item);
}

void arrayGlobalDeclMips(MidCode *midCode) {
    VarControl item = VarControl(midCode->result);
    item.address = midCode->result;
    item.isGlobal = true;
    int address;
    if (midCode->opNum == 1) {
//        cout << item->dim1 << endl;
        address = midCode->dim1 * 4;
        item.dim1 = midCode->dim1;
        item.dim = 1;
    } else {
//        cout << item->dim1 << " " << item->dim2 << endl;
        address = midCode->dim1 * midCode->dim2 * 4;
        item.dim1 = midCode->dim1;
        item.dim2 = midCode->dim2;
        item.dim = 2;
    }
    address += 4;
    string ans = "\t" + midCode->result + ":.space " + to_string(address);
    pushMips(ans);
    controlPush(item);
}

void arrayDeclMips(MidCode *midCode) {
    VarControl item = VarControl(midCode->result);
    int addr;
    if (midCode->opNum == 1) {
        item.dim1 = midCode->dim1;
        item.dim = 1;
        addr = item.dim1 * 4;
    } else {
        item.dim1 = midCode->dim1;
        item.dim2 = midCode->dim2;
        item.dim = 2;
        addr = item.dim1 * item.dim2 * 4;
    }
    currentShift += addr;
    //存储真实地址
    pushMips("\tsubu $s0,$sp," + to_string(currentShift));
    storeVar(&item, "$s0");
//    cout << item.shift<<" current: "<<currentShift << endl;
    controlPush(item);
}

void arrayVarMips(MidCode *midCode) {
    pushMips("#arrayVar: " + midCode->opRand + " :");
    //$s0计算偏移量 $s1值
    VarControl *item = searchVar(midCode->opRand);
    string address;
    string val;
    int t1 = 0;
    if (midCode->opNum == 2) {
        int y = item->dim2;
        t1 = stoi(midCode->op1) * y + stoi(midCode->op2);
        pushMips("\tli $s0," + to_string(t1));
        if (midCode->resultType == 0) {
            pushMips("\tli $s1," + midCode->result);
        } else {
//            val = "\tmove $s1," + midCode->result;
//            cout<<"hihiiiiiiiiiiiiiiiiiiii" << endl;
            VarControl *var = searchVar(midCode->result);
            getVar(*var, "$s1");
        }
    } else {
//        cout << midCode->opRand << " " << midCode->dim1 << endl;
        if (midCode->dim1 == 1) {
            VarControl *var = searchVar(midCode->op1);
            getVar(*var, "$s0");
        } else {
            t1 = stoi(midCode->op1);
//            cout << midCode->opRand << " " << to_string(t1) << endl;
            pushMips("\tli $s0," + to_string(t1));
        }
        if (midCode->resultType == 0) {
            pushMips("\tli $s1," + midCode->result);
        } else {
//            val = "\tmove $s1," + midCode->result;
            VarControl *addr = searchVar(midCode->result);
            getVar(*addr, "$s1");
        }
    }
    pushMips("\taddu $s0,$s0,4");
    if (item->isGlobal) {
        pushMips("\tsw $s1," + item->address + "($s0)");
    } else {
        //当前元素相对于数组首地址的偏移量+数组真实地址
        getVar(*item, "$s2");
        pushMips("\taddu $s0,$s0,$s2");
        pushMips("\tsw $s1,($s0)");
//        cout << "item: " << item->shift << ", shift:" << t1
//             << ", currentShift: " << currentShift << endl;
    }
}

void arrayGetMips(MidCode *midCode) {
    pushMips("#arrayGet: " + midCode->opRand + " " + midCode->opRand);
    //$s0偏移,$s1值
    VarControl *item = searchVar(midCode->result);
    VarControl *var = searchVar(midCode->opRand);
    if (midCode->opNum == 1) {
        if (midCode->resultType == 0) {
            pushMips("\tli $s0," + midCode->op1);
        } else {
            VarControl *index = searchVar(midCode->op1);
            getVar(*index, "$s0");
        }
        pushMips("\taddi $s0,$s0,4");
        if (var->isGlobal) {
            pushMips("\tlw $s1," + var->address + "($s0)");
        } else {
            getVar(*var, "$s2");
            pushMips("\taddu $s0,$s2,$s0");
            pushMips("\tlw $s1,($s0)");
        }
        storeVar(item, "$s1");
    } else {
//        cout << "here we need address!!!!!!!!!!!!!" << endl;
        if (midCode->resultType == 0) {
//            cout << midCode->op1 <<" " << var->dim2 <<endl;
            int shift = stoi(midCode->op1) * var->dim2;
            pushMips("\tli $s0," + to_string(shift));
        } else {
            VarControl *index = searchVar(midCode->op1);
            getVar(*index, "$s0");
            pushMips("\tmul $s0,$s0," + to_string(var->dim2));
        }
        if (var->isGlobal) {
            pushMips("\tla $s1," + var->address + "($s0)");
            pushMips("\tmove $s0,$s1");
//            item->isGlobal = true;
        } else {
            getVar(*var, "$s2");
            pushMips("\taddu $s0,$s2,$s0");
        }
        storeVar(item, "$s0");
    }
}

void funcDefMips(MidCode *midCode) {
    VarControl func = VarControl(midCode->op1);
    controlPush(func);
    pushMips(midCode->op1 + ":");
    currentShift = 0;
}

void funcParamMips(MidCode *midCode) {
    VarControl param = VarControl(midCode->opRand);
    param.point = controlPoint + 1;
    param.isFParam = true;
    param.dim = midCode->opNum;
    param.dim2 = midCode->dim2;
    storeVar(&param);
    controlPush(param);

    VarControl *func = searchVar(midCode->result);
    func->shift = currentShift;
    func->paramList.emplace_back(&param);
    func->params++;
}

void funcEndMips(MidCode *midCode) {
    if (!isJr) {
//        cout << "Func_end" <<endl;
        isJr = true;
    }
    VarControl *func = searchVar(midCode->op1);
    func->shift = currentShift;
    pushMips("\tjr $ra");
}

void pushVarMips(MidCode *midCode) {
    VarControl *func = searchVar(midCode->result);
    if (midCode->paraNumber < 0) {
        return;
    }
    //形参
//    string str = funcItem->params[midCode->paraNumber].str;
//    SymbolItem *expItem = symTable.getExistedParamSymbol(str);
//    SymbolItem *expItem = &funcItem->params[midCode->paraNumber];
    if (midCode->resultType == 0) {
        //Number
        pushMips("\tli $s0," + midCode->op1);
    } else {
        //实参
        VarControl *valItem = searchVar(midCode->op1);
//        cout << valItem->str << " " << valItem->isGlobal << endl;
        if (valItem->isGlobal && valItem->dim > 0) {
            pushMips("\tla $s0," + valItem->address);
        } else {
            getVar(*valItem, "$s0");
        }
    }
    putVar(currentShift + 4 * (func->params + 1), "$s0");
}

void funcCallMips(MidCode *midCode) {
    pushSP(currentShift);
    //push $ra
    pushMips("\tsw $ra,0($sp)");
    pushSP(4);

    preShift.push(currentShift);
    currentShift = 0;

    pushMips("\tjal " + midCode->op1);

    //pop $ra
    popSP(4);
    pushMips("\tlw $ra,0($sp)");

    currentShift = preShift.top();
    preShift.pop();
    popSP(currentShift);
}

void mainFuncMips() {
    pushMips("main:");
    isMain = true;
    currentShift = 0;
}

void expMips(MidCode *midCode) {
    if (midCode->opNum == 0) {
        //a=b
        VarControl *item = searchVar(midCode->result);
        if (midCode->resultType == 0) {
            pushMips("\tli $s0," + midCode->op1);
        } else {
            if (checkVar(midCode->op1)) {
                VarControl *var = searchVar(midCode->op1);
                getVar(*var, "$s0");
            } else {
                pushMips("\tmove $s0," + midCode->op1);
            }
        }
        if (item->isTmpVar) {
            storeVar(item, "$s0");
        } else {
            putVar(*item, "$s0");
        }
    } else if (midCode->opNum == 1) {
        //a = !b
        VarControl *item = searchVar(midCode->result);
        if (midCode->resultType == 0) {
            pushMips("\tli $s0," + midCode->op1);
        } else {
            VarControl *var = searchVar(midCode->op1);
            getVar(*var, "$s0");
        }
        string op = midCode->opRand;
        if (op == "!") {
            pushMips("\tseq $s1,$0,$s0");
        } else if (op == "+") {
            pushMips("\tadd $s1,$0,$s0");
        } else if (op == "-") {
            pushMips("\tsub $s1,$0,$s0");
        }
        if (item->isTmpVar) {
            storeVar(item, "$s1");
        } else {
            putVar(*item, "$s1");
        }
    } else {
        //a = b + c
        string op = midCode->opRand;
        VarControl *item = searchVar(midCode->result);
        if (midCode->resultType == 0) {
            //left number
            pushMips("\tli $s0," + midCode->op1);
            VarControl *right = searchVar(midCode->op2);
            getVar(*right, "$s1");
        } else if (midCode->resultType == 1) {
            //right number
            VarControl *left = searchVar(midCode->op1);
            getVar(*left, "$s0");
            pushMips("\tli $s1," + midCode->op2);
        } else {
            //both exp
            VarControl *left = searchVar(midCode->op1);
            getVar(*left, "$s0");
            VarControl *right = searchVar(midCode->op2);
            getVar(*right, "$s1");
        }
        if (op == "+") {
            pushMips("\taddu $s2,$s0,$s1");
        } else if (op == "-") {
            pushMips("\tsubu $s2,$s0,$s1");
        } else if (op == "*") {
            pushMips("\tmul $s2,$s0,$s1");
        } else {
            pushMips("\tdiv $s0,$s1");
            if (op == "/") {
                pushMips("\tmflo $s2");
            } else {
                pushMips("\tmfhi $s2");
            }
        }
        if (item->isTmpVar) {
            storeVar(item, "$s2");
        } else {
            putVar(*item, "$s2");
        }
    }
}

void printStrMips(MidCode *midCode) {
    string ans = "\tstr" + midCode->op2 + ":.asciiz \"" + midCode->op1 + "\"\0";
    pushMips(ans);
}

void printMips(MidCode *midCode) {
    if (midCode->opNum == 1) {
        pushMips("\tla $a0,str" + midCode->op2);
        pushMips("\tli $v0,4");
    } else {
        if (midCode->resultType == 0) {
            pushMips("\tli $a0," + midCode->op2);
        } else {
            VarControl *item = searchVar(midCode->op2);
//          cout <<item->str<<currentLayer->blockNum<<"  "<< item->blockNum << endl;
            getVar(*item, "$a0");
        }
        pushMips("\tli $v0,1");
    }
    pushMips(syscall);
}

void getIntMips(MidCode *midCode) {
//    cout << midCode->op1 << endl;
    pushMips("\tli $v0,5");
    pushMips(syscall);
    VarControl *item = searchVar(midCode->result);
    if (item->isTmpVar) {
        storeVar(item, "$v0");
    } else {
        putVar(*item, "$v0");
    }
}

void labelMips(MidCode *midCode) {
    pushMips(midCode->op1 + ":");
}

void gotoMips(MidCode *midCode) {
    pushMips("\tj " + midCode->op1);
}

void cmpMips(MidCode *midCode) {
    VarControl *item1 = searchVar(midCode->op1);
    getVar(*item1, "$s0");
    if (midCode->resultType == 0) {
        pushMips("\tli $s1," + midCode->op2);
    } else {
        VarControl *item2 = searchVar(midCode->op2);
        getVar(*item2, "$s1");
    }
    pushMips("\t" + midCode->opRand + " $s0,$s1," + midCode->result);
}

void compareMips(MidCode *midCode) {
    VarControl *item1 = searchVar(midCode->op1);
    getVar(*item1, "$s1");
    if (midCode->resultType == 0) {
        pushMips("\tli $s2," + midCode->op2);
    } else {
        VarControl *item2 = searchVar(midCode->op2);
        getVar(*item2, "$s2");
    }
    pushMips("\t" + midCode->opRand + " $s0,$s1,$s2");
    VarControl *item = searchVar(midCode->result);
    storeVar(item, "$s0");
}

void returnMips(MidCode *midCode) {
    if (midCode->opNum == 2) {
        if (checkVar(midCode->op2)) {
            VarControl *item = searchVar(midCode->op2);
            getVar(*item, "$v0");
        } else {
            pushMips("\tli $v0," + midCode->op2);
        }
    } else if (midCode->opNum == 1) {
        pushMips("\tli $v0," + midCode->op2);
    }
    if (!isMain) {
        pushMips("\tjr $ra");
    }
    isJr = true;
}

void pushMips(const string &s) {
    if (PRINT) {
        cout << s << endl;
    }
    mipsTable.emplace_back(s);
}

void printMips(ofstream &file) {
    for (int i = 0; i < mipsTable.size(); i++) {
        file << mipsTable[i] << endl;
    }
}

void blockBegin() {
    controlPoint++;
}

void blockEnd() {
    int cnt = 0;
//    cout << "controlPoint: " << controlPoint << endl;
    for (int i = varControl.size() - 1; i >= 0; i--) {
        VarControl var = varControl[i];
        if (var.point == controlPoint) {
//        cout << "name: " << var.str << " point: " << var.point << endl;
            cnt++;
        }
    }
    for (int i = 0; i < cnt; i++) {
//        cout << "pop " << varControl[varControl.size() - 1].str
//             << " Now,varControl's size is " << varControl.size()
//             << endl;
        varControl.pop_back();
    }
    controlPoint--;
}

void getVar(const VarControl &var, const string &s) {
    //取值
//    cout << var->str << var->isGlobal << endl;
    if (var.isGlobal) {
        pushMips("\tlw " + s + "," + var.str);//s->$s0
    } else {
        pushMips("\tlw " + s + ",-" + to_string(var.shift) + "($sp)");//s->$s0
    }
}

void storeVar(VarControl *var, const string &s) {
    //开辟新空间，并存值
    pushMips("\tsw " + s + ",-" + to_string(currentShift) + "($sp)");//s->$s0
    var->shift = currentShift;
    currentShift += 4;
//    cout << var->shift << " " << var->str  << endl;
}

void storeVar(VarControl *var) {
    //开辟新空间，并存值
    var->shift = currentShift;
    currentShift += 4;
//    cout << var->shift << " " << var->str  << endl;
}

void putVar(const VarControl &var, const string &s) {
    //存值
    if (var.isGlobal) {
        pushMips("\tsw " + s + "," + var.str);
    } else {
//        cout << var.str << " " << var.shift << endl;
        pushMips("\tsw " + s + ",-" + to_string(var.shift) + "($sp)");//s->$s0
    }
}

void putVar(int shift, const string &s) {
    pushMips("\tsw " + s + ",-" + to_string(shift) + "($sp)");//s->$s0
    currentShift += 4;
}

void pushSP(int shift) {
    pushMips("\tsubi $sp,$sp," + to_string(shift));
}

void popSP(int shift) {
    pushMips("\taddi $sp,$sp," + to_string(shift));
}

void controlPush(VarControl var) {
    varControl.emplace_back(var);
//    cout << "push " << var.str << " Now,varControl's size is " << varControl.size() << endl;
}

bool checkVar(const string &name) {
    for (int i = varControl.size() - 1; i >= 0; i--) {
        if (varControl[i].str == name) {
            return true;
        }
    }
    return false;
}

VarControl *searchVar(const string &name) {
    if (name[0] == '#' && !checkVar(name)) {
        VarControl tmp = VarControl(name);
        tmp.isTmpVar = true;
        controlPush(tmp);
    }
    for (int i = varControl.size() - 1; i >= 0; i--) {
        if (varControl[i].str == name) {
//            cout << name << endl;
            return &varControl[i];
        }
    }
    cout << "Sorry, there's no var " << name << "." << endl;
    return &varControl[0];
}