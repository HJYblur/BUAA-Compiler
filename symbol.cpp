//
// Created by lemon on 2021/10/30.
//

#include "symbol.h"
#include "iostream"
#include "globalVar.h"

using namespace std;

Symbol::Symbol() {
    this->index = globalIndex;
}

Symbol::Symbol(symbolType t) {
    this->type = t;
    this->index = globalIndex;
}

Symbol::Symbol(symbolType t, std::string str) {
    this->type = t;
    this->str = std::move(str);
    this->index = globalIndex;
}

Symbol::Symbol(symbolType t, std::string str, enum bType bt) {
    this->type = t;
    this->str = std::move(str);
    this->index = globalIndex;
    this->btype = bt;
}

void Symbol::addParam(const Symbol &s) {
    this->params.emplace_back(s);
}

void Symbol::setDim(int d) {
    this->dim = d;
}

void Symbol::setIndex(int d) {
    this->index = d;
}

void Symbol::setReturnType(enum bType type) {
    this->returnType = type;
}

void Symbol::setBType(enum bType btype1) {
    this->btype = btype1;
}

void printSymbolTable() {
    std::cout << "----------------------------------------------------------" << std::endl;
    std::cout << globalIndex << std::endl;
    for (auto one : symbolTable) {
        int size = one.params.size();
        std::cout << "type: " << one.type << " "
                  << "index: " << one.index << " "
                  << "str: " << one.str << " "
                  << "paramSize: " << size << " "
                  << "dim: " << one.dim << " "
                  << "address: " << &one << " ";
        if (size > 0) {
            cout << "params:" << " ";
            for (int i = 0; i < size; i++) {
                std::cout << one.params[i].str << " ";
            }
        }
        std::cout << std::endl;
    }
}

void popAllSymbol() {
//    for (std::vector<Symbol>::iterator it = symbolTable.begin(); it != symbolTable.end(); it++) {
//        if (it->index > globalIndex) {
//            it = symbolTable.erase(it);
//        }
//    }
    int size = symbolTable.size();
    int cnt = 0;
    for (int i = size - 1; i >= 0; i--) {
        Symbol item = symbolTable[i];
        if (item.index > globalIndex) {
            cnt++;
        }
    }
    for (int i = 0; i < cnt; i++) {
        symbolTable.pop_back();
    }
}
