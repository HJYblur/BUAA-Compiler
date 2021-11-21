//
// Created by lemon on 2021/10/10.
//

#include <iostream>
#include <fstream>
#include "string"
#include "vector"
#include "lexical.h"
#include "grammar.h"
#include "error.h"
#include "symbol.h"
#include "globalVar.h"

using namespace std;

int symNum = -1;
int globalIndex = 1;
int rightBrace = 0;
vector<Content> storagePlus;
Content token;
vector<Symbol> symbolTable;
vector<Symbol> currentParams;

int isCirculation = 0;
int globalDim = 0;
bool isRightFunc = true;
bool isReturn = false;

int grammarScan() {
    //step1:打开文件&初始化
    ofstream destFile("output.txt", ios::out); //以文本模式打开out.txt备写
    if (!destFile) {
        cout << "error opening destination file." << endl;
        return 0;
    }
    ofstream errorFile("error.txt", ios::out); //以文本模式打开out.txt备写
    if (!errorFile) {
        cout << "error opening error file." << endl;
        return 0;
    }

    //step2:调用词法分析程序
    lexicalScan();

    //step3:开始语法分析
    initGrammarCategory();
    if (!storage.empty()) {
        getNextToken();
        compUnit();
    }
//    for (int i = 0; i < storage.size(); i++) {
//        cout << (storage[i]).first << endl;
//    }


    //step4:输出分析结果
//    print(destFile, storagePlus);//将词法和语法分析的结果输出
//    printSymbolTable();
    printError(errorFile);
    destFile.close();
    return 0;
}

Content::Content() {
    str = "";
    type = 0;
    line = 0;
}

Content::Content(int t) {
    str = "";
    type = t;
    line = 0;
}

void push(int op) {
    Content one(op);
    storagePlus.emplace_back(one);
}

bool getNextToken() {
    if (symNum == -1 || symNum < storage.size()) {
        token = storage[++symNum];
        storagePlus.emplace_back(token);
        return true;
    } else return false;
}

bool stepBack() {
    if (storagePlus.empty()) {
        return false;
    }
    token = storage[--symNum];
    storagePlus.pop_back();
    return true;
}

bool match(int op) {
    if (token.type == op) {
        return true;
    }
    return false;
}

bool compUnit() {
    while (match(4) || match(5)) {
        if (match(4)) {
            decl();
        } else {
            getNextToken();
            if (match(0)) {
                getNextToken();
                if (match(32)) {
                    stepBack();
                    stepBack();
                    break;
                } else {
                    stepBack();
                    stepBack();
                    decl();
                }
            } else {
                stepBack();
                break;
            }
        }
    }
    while (match(5) || match(14)) {
        if (match(5)) {
            getNextToken();
            if (match(0)) {
                stepBack();
                funcDef();
            } else {
                stepBack();
                break;
            }
        } else {
            funcDef();
        }
    }
    if (mainFuncDef()) {
        stepBack();
        push(38);
//        globalIndex--;
//        popAllSymbol();
        return true;
    } else {
        return false;
    }
}

bool decl() {
    if (match(4)) {
        constDecl();
        return true;
    } else if (match(5)) {
        varDecl();
        return true;
    }
    return false;
}

bool constDecl() {
    if (match(4)) {
        getNextToken();
        if (bType()) {
            if (constDef()) {
                while (match(30)) {
                    getNextToken();
                    if (!constDef()) {
                        return false;
                    }
                }
                int line = token.line;
                if (match(31)) {
                    push(39);
                    getNextToken();
                } else {
                    stepBack();
                    errorList.emplace_back('i', token.line);
                    getNextToken();
                }
                return true;
            } else return false;
        } else return false;
    } else return false;
}

bool bType() {
    if (match(5)) {
        getNextToken();
        return true;
    } else return false;
}

bool constDef() {
    if (match(0)) {
        Symbol temp = Symbol(constVar, token.str);
        checkMultiIdent(temp, token.line, globalIndex);
        Symbol symbol1 = Symbol(constVar, token.str, intType);
        getNextToken();
        int cnt = 0;
        while (match(34)) {
            getNextToken();
            if (constExp()) {
                if (match(35)) {
                    getNextToken();
                } else {
                    stepBack();
                    errorList.emplace_back('k', token.line);
                    getNextToken();
                }
            } else return false;
            cnt++;
        }
        if (cnt != 0 && cnt != 1 && cnt != 2) {
            return false;
        }
        symbol1.setDim(cnt);
        if (match(29)) {
            getNextToken();
            if (constInitVal()) {
                stepBack();
                push(40);
                getNextToken();
                symbolTable.emplace_back(symbol1);
                return true;
            } else return false;
        } else return false;
    } else return false;
}

bool constInitVal() {
    if (constExp()) {
        stepBack();
        push(41);
        getNextToken();
        return true;
    } else if (match(36)) {
        getNextToken();
        if (constInitVal()) {
            while (match(30)) {
                getNextToken();
                if (!constInitVal()) {
                    return false;
                }
            }
        }
        if (match(37)) {
            push(41);
            getNextToken();
            return true;
        } else return false;
    } else return false;
}

bool varDecl() {
    if (bType()) {
        if (varDef()) {
            while (match(30)) {
                getNextToken();
                if (!varDef()) {
                    return false;
                }
            }
            if (match(31)) {
                push(42);
                getNextToken();
            } else {
                stepBack();
                errorList.emplace_back('i', token.line);
                getNextToken();
            }
            return true;
        } else return false;
    } else return false;
}

bool varDef() {
    if (match(0)) {
        Symbol temp = Symbol(var, token.str);
        checkMultiIdent(temp, token.line, globalIndex);
        Symbol symbol1 = Symbol(var, token.str, intType);
        getNextToken();
        int cnt = 0;
        while (match(34)) {
            getNextToken();
            if (constExp()) {
                if (match(35)) {
                    getNextToken();
                } else {
                    stepBack();
                    errorList.emplace_back('k', token.line);
                    getNextToken();
                }
            } else return false;
            cnt++;
        }
        if (cnt != 0 && cnt != 1 && cnt != 2) {
            return false;
        }
        symbol1.setDim(cnt);
        if (match(29)) {
            getNextToken();
            if (initVal()) {
                stepBack();
                push(43);
                getNextToken();
                symbolTable.emplace_back(symbol1);
                return true;
            } else return false;
        } else {
            stepBack();
            push(43);
            getNextToken();
            symbolTable.emplace_back(symbol1);
            return true;
        }
    } else return false;
}

bool initVal() {
    if (exp()) {
        stepBack();
        push(44);
        getNextToken();
        return true;
    } else if (match(36)) {
        getNextToken();
        if (initVal()) {
            while (match(30)) {
                getNextToken();
                if (!initVal()) {
                    return false;
                }
            }
        }
        if (match(37)) {
            push(44);
            getNextToken();
            return true;
        } else return false;
    } else return false;
}

bool funcDef() {
    int currentType = token.type;
    if (funcType()) {
        enum bType bt;
        if (currentType == 5) {
            bt = intType;
        } else {
            bt = voidType;
        }
        if (match(0)) {
            Symbol temp = Symbol(func, token.str);
            checkMultiIdent(temp, token.line, globalIndex);
            symbolTable.emplace_back(Symbol(func, token.str, bt));
            Symbol *currentSym = &symbolTable[symbolTable.size() - 1];
//            Symbol& currentSym = symbolTable.back();
//                printSymbolTable();
//            cout << currentSym.params.size() << endl;
//            cout<<currentSym->type<<" "<<currentSym->str<<endl;
//            cout << currentSym->str<<currentSym << endl;
//            cout << symbolTable[symbolTable.size()-1].str<<&symbolTable[symbolTable.size()-1] << endl;
            getNextToken();
            if (match(32)) {
                int line = token.line;
                getNextToken();
                if (funcFParams(currentSym)) {
                    checkParams(currentSym->params, line);
                    for (Symbol item:currentSym->params) {
                        symbolTable.emplace_back(item);
                    }
                }
//                cout << symbolTable.front().str << " " << symbolTable.front().params.size() << endl;
//                cout << currentSym->str << " " << currentSym->params.size() << endl;
                if (match(33)) {
                    getNextToken();
                } else {
                    stepBack();
                    errorList.emplace_back('j', token.line);
                    getNextToken();
                }
                if (block(currentSym)) {
                    if (bt == intType) {
                        if (!isReturn) {
                            errorList.emplace_back('g', rightBrace);
                        }
                    }
                    stepBack();
                    push(45);
                    getNextToken();
                    return true;
                } else return false;
            } else return false;
        } else return false;
    } else return false;
}

bool mainFuncDef() {
    if (match(5)) {
        getNextToken();
        if (match(3)) {
            getNextToken();
            if (match(32)) {
                getNextToken();
                if (match(33)) {
                    getNextToken();
                } else {
                    stepBack();
                    errorList.emplace_back('j', token.line);
                    getNextToken();
                }
                Symbol symbol = Symbol(func, "main", intType);
                if (block(&symbol)) {
                    if (!isReturn) {
                        errorList.emplace_back('g', rightBrace);
                    }
                    symbolTable.emplace_back(symbol);
                    stepBack();
                    push(46);
                    getNextToken();
                    return true;
                } else return false;
            } else return false;
        } else return false;
    } else return false;
}

bool funcType() {
    if (match(5) || match(14)) {
        push(47);
        getNextToken();
        return true;
    }
    return false;
}

bool funcFParams(Symbol *sym) {
//    cout <<sym->type<<" "<<sym->str<<" "<< "0" << endl;
    if (funcFParam(sym)) {
//        cout <<sym->type<<" "<<sym->str<<" "<< "1" << endl;
        while (match(30)) {
            getNextToken();
            if (!funcFParam(sym)) {
                return false;
            }
//            cout <<sym->type<<" "<<sym->str<<" "<< "2" << endl;
        }
        stepBack();
        push(48);
        getNextToken();
        return true;
    } else return false;
}

bool funcFParam(Symbol *sym) {
    if (bType()) {
        if (match(0)) {
            Symbol temp = Symbol(var, token.str);
            checkMultiIdent(temp, token.line, globalIndex + 1);
            Symbol symbol = Symbol(var, token.str, intType);
            getNextToken();
            int cnt = 0;
            if (match(34)) {
                cnt++;
                getNextToken();
                if (match(35)) {
                    getNextToken();
                } else {
                    stepBack();
                    errorList.emplace_back('k', token.line);
                    getNextToken();
                }
                while (match(34)) {
                    cnt++;
                    getNextToken();
                    if (constExp()) {
                        if (match(35)) {
                            getNextToken();
                        } else {
                            stepBack();
                            errorList.emplace_back('k', token.line);
                            getNextToken();
                        }
                    } else return false;
                }
            }
            symbol.setDim(cnt);
            symbol.setIndex(globalIndex + 1);
            (*sym).params.emplace_back(symbol);
            stepBack();
            push(49);
            getNextToken();
            return true;
        } else return false;
    } else return false;
}

bool block(Symbol *sym) {
    if (match(36)) {
        globalIndex++;
//        printSymbolTable();
        getNextToken();
        while (blockItem(sym));
        if (match(37)) {
//            cout << sym.str << sym.type << endl;
            rightBrace = token.line;
//            if (sym.type == func){
//                checkReturn(sym, rightBrace);
//            }
            globalIndex--;
            popAllSymbol();
            push(50);
            getNextToken();
            return true;
        } else return false;
    } else return false;
}

bool blockItem(Symbol *sym) {
    if (match(4) || match(5)) {
        isReturn = false;
        decl();
        return true;
    } else {
        if (stmt(sym)) {
            return true;
        } else return false;
    }
}

bool stmt(Symbol *sym) {
    if (match(0)) {
        isReturn = false;
        int initialLine = token.line;
        getNextToken();
        int cnt = 1;
        int currentLine;
        bool flag = false;
        while (!match(31)) {
            if (match(29)) {
                flag = true;
                break;
            }
            getNextToken();
            currentLine = token.line;
            cnt++;
            if (initialLine != currentLine) {
                break;
            }
        }
        while (cnt) {
            stepBack();
            cnt--;
        }
        if (flag) {
            checkIsConst();
            lVal();
            getNextToken();
            if (exp()) {
                if (match(31)) {
                    push(51);
                    getNextToken();
                } else {
                    stepBack();
                    errorList.emplace_back('i', token.line);
                    getNextToken();
                }
                return true;
            } else if (match(10)) {
                getNextToken();
                if (match(32)) {
                    getNextToken();
                    if (match(33)) {
                        getNextToken();
                    } else {
                        stepBack();
                        errorList.emplace_back('j', token.line);
                        getNextToken();
                    }
                    if (match(31)) {
                        push(51);
                        getNextToken();
                    } else {
                        stepBack();
                        errorList.emplace_back('i', token.line);
                        getNextToken();
                    }
                    return true;
                } else return false;
            } else return false;
        } else {
            if (exp()) {
                if (match(31)) {
                    push(51);
                    getNextToken();
                } else {
                    stepBack();
                    errorList.emplace_back('i', token.line);
                    getNextToken();
                }
                return true;
            } else return false;
        }
    } else if (exp()) {
        isReturn = false;
        if (match(31)) {
            push(51);
            getNextToken();
        } else {
            stepBack();
            errorList.emplace_back('i', token.line);
            getNextToken();
        }
        return true;
    } else if (match(31)) {
        isReturn = false;
        push(51);
        getNextToken();
        return true;
    } else if (block(sym)) {
        isReturn = false;
        stepBack();
        push(51);
        getNextToken();
        return true;
    } else if (match(8)) {
        isReturn = false;
        getNextToken();
        if (match(32)) {
            getNextToken();
            if (cond()) {
                if (match(33)) {
                    getNextToken();
                } else {
                    stepBack();
                    errorList.emplace_back('j', token.line);
                    getNextToken();
                }
                Symbol symbol = Symbol(null);
                if (stmt(&symbol)) {
                    if (match(9)) {
                        getNextToken();
                        if (stmt(&symbol)) {
                            stepBack();
                            push(51);
                            getNextToken();
                            return true;
                        } else return false;
                    } else {
                        stepBack();
                        push(51);
                        getNextToken();
                        return true;
                    }
                } else return false;
            } else return false;
        } else return false;
    } else if (match(13)) {
        isReturn = false;
        isCirculation++;
        getNextToken();
        if (match(32)) {
            getNextToken();
            if (cond()) {
                if (match(33)) {
                    getNextToken();
                } else {
                    stepBack();
                    errorList.emplace_back('j', token.line);
                    getNextToken();
                }
                Symbol symbol = Symbol(null);
                if (stmt(&symbol)) {
                    stepBack();
                    push(51);
                    getNextToken();
                    isCirculation--;
                    return true;
                } else return false;
            } else return false;
        } else return false;
    } else if (match(6) || match(7)) {
        isReturn = false;
        int l = token.line;
        getNextToken();
        if (match(31)) {
            push(51);
            getNextToken();
        } else {
            stepBack();
            errorList.emplace_back('i', token.line);
            getNextToken();
        }
        if (isCirculation == 0) {
            errorList.emplace_back('m', l);
        }
        return true;
    } else if (match(12)) {
        isReturn = true;
        Symbol symbol = Symbol(returnParam, "return", voidType);
        int l = token.line;
        getNextToken();
        if (exp()) {
            symbol.btype = intType;
            sym->returnType = intType;
        } else {
            sym->returnType = voidType;
        }
        if (match(31)) {
            push(51);
            getNextToken();
        } else {
            stepBack();
            errorList.emplace_back('i', token.line);
            getNextToken();
        }
        checkReturn(*sym, symbol, l);
        return true;
    } else if (match(11)) {
        isReturn = false;
        int formatLine = token.line;
        getNextToken();
        if (match(32)) {
            getNextToken();
            if (match(2)) {
                int formatCnt = checkFormatString(token);
                int expCnt = 0;
                getNextToken();
                while (match(30)) {
                    expCnt++;
                    getNextToken();
                    if (!exp()) {
                        return false;
                    }
                }
                if (formatCnt != expCnt) {
                    errorList.emplace_back('l', formatLine);
                }
                if (match(33)) {
                    getNextToken();
                } else {
                    stepBack();
                    errorList.emplace_back('j', token.line);
                    getNextToken();
                }
                if (match(31)) {
                    push(51);
                    getNextToken();
                } else {
                    stepBack();
                    errorList.emplace_back('i', token.line);
                    getNextToken();
                }
                return true;
            } else return false;
        } else return false;
    } else {
        return false;
    }
}

int checkFormatString(const Content &c) {
    int cnt = 0;
    string s = c.str;
    for (int i = 0; i < s.size(); i++) {
        if ((i < s.size() - 1) && (s[i] == '%' && s[i + 1] == 'd')) {
            cnt++;
        }
    }
    return cnt;
}

bool exp() {
    if (addExp()) {
        stepBack();
        push(52);
        getNextToken();
        return true;
    } else return false;
}

bool cond() {
    if (lOrExp()) {
        stepBack();
        push(53);
        getNextToken();
        return true;
    } else return false;
}

bool lVal() {
    if (match(0)) {
        Symbol symbol = Symbol(var, token.str, intType);
        getNextToken();
        int cnt = 0;
        globalDim = cnt;
        while (match(34)) {
            cnt++;
            getNextToken();
            if (!exp()) {
                return false;
            }
            if (match(35)) {
                getNextToken();
            } else {
                stepBack();
                errorList.emplace_back('k', token.line);
                getNextToken();
            }
        }
        globalDim = cnt;
        symbol.setDim(cnt);
        checkNoIdent(symbol);
//        cout << symbol.str <<" "<<symbol.index<< " "<< symbol.dim << endl;
        stepBack();
        push(54);
        getNextToken();
        return true;
    } else return false;
}

bool primaryExp() {
    if (match(32)) {
        getNextToken();
        if (exp()) {
            if (match(33)) {
                push(55);
                getNextToken();
            } else {
                stepBack();
                errorList.emplace_back('j', token.line);
                getNextToken();
            }
            return true;
        } else return false;
    } else if (lVal() || number()) {
        stepBack();
        push(55);
        getNextToken();
        return true;
    } else return false;
}

bool number() {
    if (match(1)) {
        push(56);
        getNextToken();
        return true;
    } else return false;
}

bool unaryExp() {
    if (match(0)) {
        Symbol symbol = Symbol(var, token.str, intType);
        Content one = token;
        getNextToken();
        if (match(32)) {
            symbol.type = func;
            isRightFunc = checkNoIdent(symbol);
            checkFuncParams(one);
//            cout << one.line << " "<<currentParams.size() << endl;
            getNextToken();
            if (funcRParams());
            if (match(33)) {
                push(57);
                getNextToken();
            } else {
                stepBack();
                errorList.emplace_back('j', token.line);
                getNextToken();
            }
            return true;
        } else {
            stepBack();
//            checkNoIdent(symbol);
            if (primaryExp()) {
                stepBack();
                push(57);
                getNextToken();
                return true;
            } else return false;
        }
    } else if (primaryExp()) {
        stepBack();
        push(57);
        getNextToken();
        return true;
    } else if (unaryOp()) {
        if (unaryExp()) {
            stepBack();
            push(57);
            getNextToken();
            return true;
        } else return false;
    } else return false;
}

bool unaryOp() {
    if (match(16) || match(17) || match(15)) {
        push(58);
        getNextToken();
        return true;
    } else return false;
}

bool funcRParams() {
    int cnt = 0;
    int size = currentParams.size();
    vector<Symbol> thisParams = currentParams;
    bool funcRFlag = true;
    Content temp = token;
    if (exp()) {
        if (isRightFunc && cnt < size) {
            funcRFlag = checkParam(thisParams[cnt], temp);
        }
        while (match(30)) {
            cnt++;
            getNextToken();
            temp = token;
            if (!exp()) {
                return false;
            }
            if (isRightFunc && cnt < size && funcRFlag) {
                funcRFlag = checkParam(thisParams[cnt], temp);
            }
        }
//        printSymbolTable();
//        for (int i = 0; i < currentParams.size(); i++) {
//            cout << currentParams[i].str << " ";
//        }
//        cout << endl;
//        cout << token.line << ":" << isRightFunc << " " << funcRFlag << " " << cnt + 1 << " " << currentParams.size()
//             << endl;
        if (isRightFunc && cnt + 1 != thisParams.size()) {
            errorList.emplace_back('d', token.line);
        }
        stepBack();
        push(59);
        getNextToken();
        return true;
    } else if (size == 0) {
        return true;
    } else {
        errorList.emplace_back('d', token.line);
        return true;
    }
}

bool mulExp() {
    if (unaryExp()) {
        stepBack();
        push(60);
        getNextToken();
        while (match(18) || match(19) || match(20)) {
            getNextToken();
            if (!unaryExp()) {
                return false;
            }
            stepBack();
            push(60);
            getNextToken();
        }
        return true;
    } else return false;
}

bool addExp() {
    if (mulExp()) {
        stepBack();
        push(61);
        getNextToken();
        while (match(16) || match(17)) {
            getNextToken();
            if (!mulExp()) {
                return false;
            }
            stepBack();
            push(61);
            getNextToken();
        }
        return true;
    } else return false;
}

bool relExp() {
    if (addExp()) {
        stepBack();
        push(62);
        getNextToken();
        while (match(23) || match(24) || match(25) || match(26)) {
            getNextToken();
            if (!addExp()) {
                return false;
            }
            stepBack();
            push(62);
            getNextToken();
        }
        return true;
    } else return false;
}

bool eqExp() {
    if (relExp()) {
        stepBack();
        push(63);
        getNextToken();
        while (match(27) || match(28)) {
            getNextToken();
            if (!relExp()) {
                return false;
            }
            stepBack();
            push(63);
            getNextToken();
        }
        return true;
    } else return false;
}

bool lAndExp() {
    if (eqExp()) {
        stepBack();
        push(64);
        getNextToken();
        while (match(21)) {
            getNextToken();
            if (!eqExp()) {
                return false;
            }
            stepBack();
            push(64);
            getNextToken();
        }
        return true;
    } else return false;
}

bool lOrExp() {
    if (lAndExp()) {
        stepBack();
        push(65);
        getNextToken();
        while (match(22)) {
            getNextToken();
            if (!lAndExp()) {
                return false;
            }
            stepBack();
            push(65);
            getNextToken();
        }
        return true;
    } else return false;
}

bool constExp() {
    if (addExp()) {
        stepBack();
        push(66);
        getNextToken();
        return true;
    } else return false;
}

bool checkMultiIdent(Symbol symbol, int line, int index) {
//    printSymbolTable();
    int size = symbolTable.size();
    for (int i = size - 1; i >= 0; i--) {
        Symbol item = symbolTable[i];
        if (item.index < index) {
            break;
        }
        if (item.str == symbol.str && item.index >= index) {
            if (item.type == func && symbol.type != func) {
                continue;
            } else {
                errorList.emplace_back('b', line);
                return false;
            }
        }
    }
    return true;
}

bool checkNoIdent(Symbol symbol) {
//    printSymbolTable();
    for (int i = symbolTable.size() - 1; i >= 0; i--) {
        Symbol item = symbolTable[i];
        if (item.str == symbol.str && item.index <= symbol.index) {
//            if (item.type == symbol.type) {
//                return true;
//            } else {
//                cout << symbol.str << endl;
//                cout << symbol.type << " " << symbol.dim << endl;
//                cout << item.type << " " << item.dim << endl;
//                errorList.emplace_back('c', token.line);
//                return false;
//            }
            return true;
        }
    }
//    cout << symbol.str << endl;
//    cout << symbol.type << " " << symbol.dim << endl;
    errorList.emplace_back('c', token.line);
    return false;
}

bool checkIsConst() {
    for (int i = symbolTable.size() - 1; i >= 0; i--) {
        Symbol item = symbolTable[i];
        if (item.str == token.str) {
            if (item.type == constVar) {
                errorList.emplace_back('h', token.line);
            } else if (item.type == var) {
                return true;
            }
        }
    }
    return false;
}

bool checkFuncParams(const Content &one) {
    for (int i = symbolTable.size() - 1; i >= 0; i--) {
        Symbol item = symbolTable[i];
        if (item.type == func && item.str == one.str) {
            currentParams = item.params;
//            cout << item.str<<" is "<<currentParams.size() << endl;
            return true;
        }
    }
    return false;
}

bool checkParam(Symbol s, Content c) {
    //形参     实参
    for (int i = symbolTable.size() - 1; i >= 0; i--) {
        Symbol item = symbolTable[i];
        if (item.str == c.str) {
            if (item.type != s.type || item.dim - globalDim != s.dim) {
                errorList.emplace_back('e', c.line);
                return false;
            } else {
                return true;
            }
        }
    }
    return false;
}

bool checkReturn(Symbol father, Symbol son, int line) {
    if (father.btype == son.btype) {
        return true;
    } else {
        if (father.btype == voidType) {
            errorList.emplace_back('f', line);
        }
        return false;
    }
}

void checkParams(vector<Symbol> params, int line) {
    for (int i = 0; i < params.size(); i++) {
        string s = params[i].str;
        for (int j = 0; j < i; j++) {
            if (s == params[j].str) {
                errorList.emplace_back('b', line);
                return;
            }
        }
    }
}

void print(ofstream &file, vector<Content> &store) {
    int i;
    for (i = 0; i < store.size(); i++) {
        int yard = (store[i]).type;
        if (category.count(yard) == 1) {
            if (yard < 38) {
                file << category[yard] << " " << store[i].str << endl;
//                cout <<' category[yard] << " " << store[i].first << endl;
            } else {
                file << category[yard] << endl;
//                cout << category[yard] << endl;
            }
        }
    }
}

void initGrammarCategory() {
    category.insert(sort(38, "<CompUnit>"));
    category.insert(sort(39, "<ConstDecl>"));
    category.insert(sort(40, "<ConstDef>"));
    category.insert(sort(41, "<ConstInitVal>"));
    category.insert(sort(42, "<VarDecl>"));
    category.insert(sort(43, "<VarDef>"));
    category.insert(sort(44, "<InitVal>"));
    category.insert(sort(45, "<FuncDef>"));
    category.insert(sort(46, "<MainFuncDef>"));
    category.insert(sort(47, "<FuncType>"));
    category.insert(sort(48, "<FuncFParams>"));
    category.insert(sort(49, "<FuncFParam>"));
    category.insert(sort(50, "<Block>"));
    category.insert(sort(51, "<Stmt>"));
    category.insert(sort(52, "<Exp>"));
    category.insert(sort(53, "<Cond>"));
    category.insert(sort(54, "<LVal>"));
    category.insert(sort(55, "<PrimaryExp>"));
    category.insert(sort(56, "<Number>"));
    category.insert(sort(57, "<UnaryExp>"));
    category.insert(sort(58, "<UnaryOp>"));
    category.insert(sort(59, "<FuncRParams>"));
    category.insert(sort(60, "<MulExp>"));
    category.insert(sort(61, "<AddExp>"));
    category.insert(sort(62, "<RelExp>"));
    category.insert(sort(63, "<EqExp>"));
    category.insert(sort(64, "<LAndExp>"));
    category.insert(sort(65, "<LOrExp>"));
    category.insert(sort(66, "<ConstExp>"));
}