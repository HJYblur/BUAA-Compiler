//
// Created by lemon on 2021/11/14.
//

#include "symbolTree.h"
#include "iostream"
#include "globalVar.h"

SymbolItem::SymbolItem(AstNode node) {
    this->str = node.str;
    this->dim = node.dim;
    this->value = node.val;
    this->layer = currentLayer;
}

SymbolItem::SymbolItem(symbolItemType t) {
    this->type = t;
    this->layer = currentLayer;
}

SymbolItem::SymbolItem(symbolItemType t, AstNode node) {
    this->type = t;
    this->str = node.str;
    this->dim = node.dim;
    this->layer = currentLayer;
}

void SymbolItem::setString(string s) {
    this->str = std::move(s);
}

void SymbolItem::setValue(int v) {
    this->value = v;
}

void SymbolItem::setDim(int d) {
    this->dim = d;
}

void SymbolItem::setDim1(int dim) {
    this->dim1 = dim;
}

void SymbolItem::setDim2(int dim) {
    this->dim2 = dim;
}

void SymbolItem::addParam(SymbolItem &para) {
    this->params.emplace_back(para);
//    cout << this->params.size()<< endl;
}

void SymbolItem::setSymbolType(symbolItemType t) {
    this->type = t;
}

void SymbolItem::setIsTmp() {
    this->isTmpVar = true;
}