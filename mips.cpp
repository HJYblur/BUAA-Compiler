//
// Created by lemon on 2021/11/18.
//
#include "iostream"
#include "midCode.h"
#include "mips.h"
#include "globalVar.h"

using namespace std;
vector<string> mipsTable;
int registerS = 0;
bool isJar = false;
bool isJr = false;
string syscall = "\tsyscall";

int getMips() {
    ofstream mipsFile(R"(H:\3rd-1stsemi\compile\mips\test.asm)", ios::out); //以文本模式打开out.txt备写
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
        }
    }
    int point = i;
    //array
    for (int i = 0; i < midCodeTable.size(); i++) {
        MidCode midCode = midCodeTable[i];
        if (midCode.type == ArrayDeclMid) {
//            cout << midCode.result << endl;
            arrayGlobalDeclMips(&midCode);
        }
    }
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
        case ArrayGetMid:
            arrayGetMips(midCode);
            break;
        case FuncDefMid:
            funcDefMips(midCode);
            break;
        case FuncParamMid:
            funcParamMips(midCode);
            break;
        case FuncEndMid:
            funcEndMips();
            break;
        case PushVar:
            pushVarMips(midCode);
            break;
        case FuncCallMid:
            funcCallMips(midCode);
            break;
        case MainFuncMid:
            registerS = 0;
            pushMips("main:");
            isJar = false;
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
            break;
        case Beq:
            break;
        case Bne:
            break;
        case Bgt:
            break;
        case Bge:
            break;
        case Blt:
            break;
        case Ble:
            break;
        case Return:
            returnMips(midCode);
            break;
        case BlockBegin:
            symTable.forwardLayer();
            break;
        case BlockEnd:
            symTable.backLayer();
            break;
    }
}

void constGlobalDeclMips(MidCode *midCode) {
    SymbolItem *item = symTable.getExistedSymbol(midCode->op1);
    item->isGlobal = true;
    string ans = "\t" + midCode->op1 + ":.word " + midCode->op2;
    pushMips(ans);
}

void constDeclMips(MidCode *midCode) {
    SymbolItem *item;
    if (symTable.checkExistedSymbol(midCode->op1)) {
        item = symTable.getExistedSymbol(midCode->op1);
    } else {
        return;
    }
    item->setAddress(getRegisterS());
    string ans;
    if (midCode->resultType == 0) {
        ans = "\tli " + item->address + "," + midCode->op2;
    } else {
        ans = "\tmove " + item->address + "," + midCode->op2;
    }
    pushMips(ans);
}

void varGlobalDeclMips(MidCode *midCode) {
    SymbolItem *item = symTable.getExistedSymbol(midCode->op1);
    item->isGlobal = true;
    if (midCode->opNum == 2) {
        string ans = "\t" + midCode->op1 + ":.word " + midCode->op2;
        pushMips(ans);
    } else {
        string ans = "\t" + midCode->op1 + ":.word " + "0";
        pushMips(ans);
    }
}

void varDeclMips(MidCode *midCode) {
    SymbolItem *item;
    if (symTable.checkExistedSymbol(midCode->op1)) {
        item = symTable.getExistedSymbol(midCode->op1);
    } else {
        return;
    }
    item->setAddress(getRegisterS());
    if (midCode->opNum == 2) {
        string ans;
        if (midCode->resultType == 0) {
            ans = "\tli " + item->address + "," + midCode->op2;
        } else {
            ans = "\tmove " + item->address + "," + midCode->op2;
        }
        pushMips(ans);
    }
}

void arrayGlobalDeclMips(MidCode *midCode) {
    SymbolItem *item;
    if (symTable.checkAllSymbol(midCode->result)) {
        item = symTable.getAllSymbol(midCode->result);
    } else {
        return;
    }
    item->setAddress(midCode->result);
    int address;
    if (item->dim == 1) {
//        cout << item->dim1 << endl;
        address = item->dim1 * 4;
    } else {
//        cout << item->dim1 << " " << item->dim2 << endl;
        address = item->dim1 * item->dim2 * 4;
    }
    string ans = "\t" + midCode->result + ":.space " + to_string(address);
    pushMips(ans);
}

void arrayVarMips(MidCode *midCode) {
    //$t9计算地址 $t8值
    SymbolItem *item;
    if (symTable.checkExistedSymbol(midCode->opRand)) {
        item = symTable.getExistedSymbol(midCode->opRand);
    } else {
        return;
    }
    int y = item->dim2;
    string address;
    string val;
    if (midCode->opNum == 2) {
        int t1 = stoi(midCode->op1) * y + stoi(midCode->op2);
        address = "\tli $t9," + to_string(t1);
        if (midCode->resultType == 0) {
            val = "\tli $t8," + midCode->result;
        } else {
            val = "\tmove $t8," + midCode->result;
        }
    } else {
        int t1 = stoi(midCode->op1);
        address = "\tli $t9," + to_string(t1);
        if (midCode->resultType == 0) {
            val = "\tli $t8," + midCode->result;
        } else {
            val = "\tmove $t8," + midCode->result;
        }
    }
    string set = "\tsw $t8," + midCode->opRand + "($t9)";
    pushMips(address);
    pushMips(val);
    pushMips(set);
}

void arrayGetMips(MidCode *midCode) {
    SymbolItem *item;
    if (symTable.checkExistedSymbol(midCode->opRand)) {
        item = symTable.getExistedSymbol(midCode->opRand);
    } else {
        return;
    }
    if (midCode->opNum == 1) {
        if (stoi(midCode->op2) == 0) {
            pushMips("\tlw " + midCode->result + "," + item->str);
        } else {
            pushMips("\tli $t9,"+midCode->op2);
            pushMips("\tlw " + midCode->result + "," + item->str + "($t9)");
        }
    } else {
        pushMips("\tlw " + midCode->result + "," + item->str + "(" + midCode->op2 + ")");
    }

}

void funcDefMips(MidCode *midCode) {
    pushMips(midCode->op1 + ":");
    registerS = 0;
}

void funcParamMips(MidCode *midCode) {
    SymbolItem *funcItem = symTable.getExistedSymbol(midCode->result);
    int n = funcItem->params.size();
    int address = 28 + (n - midCode->paraNumber) * 4;
    SymbolItem *item = symTable.getExistedSymbol(midCode->opRand);
    item->address = getRegisterS();
    pushMips("\tlw " + item->address + "," + to_string(address) + "($sp)");
}

void funcEndMips() {
    if (!isJr) {
//        cout << "Func_end" <<endl;
        pushMips("\tjr $ra");
        isJr = true;
    }
}

void pushVarMips(MidCode *midCode) {
    cout << midCode->result << " " << midCode->opNum << " " << midCode->op1 << endl;
    SymbolItem *funcItem = symTable.getExistedSymbol(midCode->result);
    if (midCode->opNum < 0) {
        pushSP(midCode->op1);
        return;
    }
    if (symTable.checkExistedSymbol(midCode->op1)) {
        SymbolItem *expItem = &funcItem->params[midCode->opNum];
        SymbolItem *valItem = symTable.getExistedSymbol(midCode->op1);
        if (expItem->dim == 0) {
            expItem->setValue(valItem->value);
        } else {
            expItem->setAddress(valItem->address);
        }
        pushSP(valItem->address);
    } else {
        pushSP(midCode->op1);
    }
}

void funcCallMips(MidCode *midCode) {
    //push $s0-$s9
    for (int i = 0; i < 8; i++) {
        pushSP("$s" + to_string(i));
    }
    pushMips("\tjal " + midCode->op1);
    //pop $s9-$s0
    for (int i = 7; i >= 0; i--) {
        popSP("$s" + to_string(i));
    }
    popSP("$ra");
    popSP("$v0");
    isJar = true;
}

void expMips(MidCode *midCode) {
    if (midCode->opNum == 0) {
        if (symTable.checkExistedSymbol(midCode->result)) {
            SymbolItem *item = symTable.getExistedSymbol(midCode->result);
            string sw;
            if (item->isGlobal && item->address.empty()) {
                item->address = getRegisterS();
                sw = "\tsw " + item->address + "," + midCode->result;
            }
            if (midCode->resultType == 0) {
                pushMips("\tli " + item->address + "," + midCode->op1);
            } else {
                pushMips("\tmove " + item->address + "," + midCode->op1);
            }
            if (!sw.empty()) {
                pushMips(sw);
            }
        } else {
            if (midCode->resultType == 0) {
                pushMips("\tli " + midCode->result + "," + midCode->op1);
            } else {
                pushMips("\tmove " + midCode->result + "," + midCode->op1);
            }
        }
    } else if (midCode->opNum == 1) {
        string op = midCode->opRand;
        if (op == "!") {
            /////////////////////////////////////
            string ans = "\tsub " + midCode->result + ",$0," + midCode->op1;
            pushMips(ans);
        } else if (op == "+") {
            return;
        } else if (op == "-") {
            string ans = "\tsub " + midCode->result + ",$0," + midCode->op1;
            pushMips(ans);
        } else {
            cout << "We get a strange opRand: " + midCode->opRand << endl;
        }
    } else {
        string op = midCode->opRand;
        string ans;
        string x;
        string y;
        string result;
        string sw;
        if (symTable.checkExistedSymbol(midCode->result)) {
            SymbolItem *item = symTable.getExistedSymbol(midCode->result);
            if (item->isGlobal && item->address.empty()) {
                item->address = getRegisterS();
                sw = "\tsw " + item->address + "," + midCode->result;
            }
            result = item->address;
        } else {
            result = midCode->result;
        }
        if (symTable.checkExistedSymbol(midCode->op1)) {
            SymbolItem *item = symTable.getExistedSymbol(midCode->op1);
            if (item->isGlobal && item->address.empty()) {
                item->address = getRegisterS();
                pushMips("\tlw " + item->address + "," + midCode->op1);
            }
            x = item->address;
        } else {
            x = midCode->op1;
        }
        if (symTable.checkExistedSymbol(midCode->op2)) {
            SymbolItem *item = symTable.getExistedSymbol(midCode->op2);
            if (item->isGlobal && item->address.empty()) {
                item->address = getRegisterS();
                pushMips("\tlw " + item->address + "," + midCode->op2);
            }
            y = item->address;
        } else {
            y = midCode->op2;
        }
        if (op == "+") {
            ans = "\taddu " + result + "," + x + "," + y;
        } else if (op == "-") {
            ans = "\tsubu " + result + "," + x + "," + y;
        } else if (op == "*") {
            string tmp = "\tmult " + x + "," + y;
            pushMips(tmp);
            ans = "\tmflo " + result;
        } else {
            string tmp = "\tdiv " + x + "," + y;
            pushMips(tmp);
            if (op == "/") {
                ans = "\tmflo " + result;
            } else {
                ans = "\tmfhi " + result;
            };
        }
        pushMips(ans);
        if (!sw.empty()) {
            pushMips(sw);
        }
    }
}

void printStrMips(MidCode *midCode) {
    string ans = "\tstr" + midCode->op2 + ":.asciiz \"" + midCode->op1 + "\"\0";
    pushMips(ans);
}

void printMips(MidCode *midCode) {
    string load;
    string li;
    if (midCode->opNum == 1) {
        load = "\tla $a0,str" + midCode->op2;
        li = "\tli $v0,4";
    } else {
        if (midCode->resultType == 0) {
            load = "\tlw $a0," + midCode->op2;
        } else {
            SymbolItem *item;
            if (symTable.checkExistedSymbol(midCode->op2)) {
                item = symTable.getExistedSymbol(midCode->op2);
                if (item->isGlobal && item->address.empty()) {
                    item->address = getRegisterS();
                    pushMips("\tlw " + item->address + "," + midCode->op2);
                }
                load = "\tmove $a0," + item->address;
            } else {
                load = "\tmove $a0," + midCode->op2;
            }
        }
        li = "\tli $v0,1";
    }
    pushMips(load);
    pushMips(li);
    pushMips(syscall);
}

void getIntMips(MidCode *midCode) {
//    cout << midCode->op1 << endl;
    SymbolItem *item;
    string str;
    string sw;
    if (symTable.checkExistedSymbol(midCode->result)) {
        item = symTable.getExistedSymbol(midCode->result);
        if (item->isGlobal && item->address.empty()) {
            item->address = getRegisterS();
            sw = "\tsw " + item->address + "," + midCode->result;
        }
        str = item->address;
    } else {
        str = midCode->result;
    }
    string get = "\tli $v0,5";
    pushMips(get);
    pushMips(syscall);
    string move = "\tmove " + str + ",$v0";
    pushMips(move);
    if (!sw.empty()) {
        pushMips(sw);
    }
}

void labelMips(MidCode *midCode) {
    pushMips(midCode->op1 + ":");
}

void gotoMips(MidCode *midCode) {
    pushMips("\tj " + midCode->op1);
}

void cmpMips(MidCode *midCode) {

}

void beqMips(MidCode *midCode) {

}

void bneMips(MidCode *midCode) {

}

void bgtMips(MidCode *midCode) {

}

void bgeMips(MidCode *midCode) {

}

void bltMips(MidCode *midCode) {

}

void bleMips(MidCode *midCode) {

}

void returnMips(MidCode *midCode) {
    if (midCode->opNum == 2) {
        pushMips("\tmove $v0," + midCode->op2);
    } else if (midCode->opNum == 1) {
        pushMips("\tli $v0," + midCode->op2);
    }
    if (isJar) {
        pushMips("\tjr $ra");
        isJr = true;
        isJar = false;
    }
}

string getRegisterS() {
    if (registerS == 8) {
        registerS = 0;
    }
    return "$s" + to_string(registerS++);
}

void pushMips(const string &s) {
    cout << s << endl;
    mipsTable.emplace_back(s);
}

void printMips(ofstream &file) {
    for (int i = 0; i < mipsTable.size(); i++) {
        file << mipsTable[i] << endl;
    }
}

void pushSP(string s) {
    pushMips("\tsubi $sp,$sp,4");
    pushMips("\tsw " + s + ",0($sp)");
}

void popSP(string s) {
    pushMips("\tlw " + s + ",0($sp)");
    pushMips("\taddi $sp,$sp,4");
}

void popSP() {
    pushMips("\taddi $sp,$sp,4");
}