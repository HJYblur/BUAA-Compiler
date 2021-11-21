//
// Created by lemon on 2021/11/14.
//

#include "symbolTree.h"
#include "iostream"
#include <utility>
#include "globalVar.h"

SymbolItem::SymbolItem(AstNode node) {
    this->str = node.str;
    this->dim = node.dim;
}

SymbolItem::SymbolItem(symbolItemType t) {
    this->type = t;
}

SymbolItem::SymbolItem(symbolItemType t, AstNode node) {
    this->type = t;
    this->str = node.str;
    this->dim = node.dim;
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

void SymbolItem::addParam(SymbolItem para) {
    this->params.emplace_back(para);
//    cout << this->params.size()<< endl;
}

void SymbolItem::setSymbolType(symbolItemType t) {
    this->type = t;
}

void SymbolItem::setAddress(string add) {
    this->address = add;
}

SymbolLayer::SymbolLayer() {
    this->index = 0;
}

void SymbolLayer::addSymbol(const SymbolItem &item) {
    this->symbolLayer.emplace_back(item);
//    cout << item.str << " " << item.type << endl;
}

void SymbolLayer::print() {
    for (int i = 0; i < this->symbolLayer.size(); i++) {
        SymbolItem item = this->symbolLayer[i];
        cout << item.str << "'s value is " << item.value << endl;
    }
}

void SymbolTable::newSymbolLayer() {
    SymbolLayer layer = SymbolLayer();
    this->symbolTable.emplace_back(layer);
    this->globalIndex = this->symbolTable.size();
}

void SymbolTable::forwardLayer() {
    if (this->currentIndex + 1 < this->globalIndex) {
        this->currentIndex++;
        currentLayer = &this->symbolTable[currentIndex];
    } else {
        newSymbolLayer();
        this->currentIndex++;
        currentLayer = &this->symbolTable[currentIndex];
    }
}

void SymbolTable::backLayer() {
    this->currentIndex--;
    currentLayer = &this->symbolTable[currentIndex];
}

void SymbolTable::print() {
    for (int i = 0; i < this->globalIndex; i++) {
        cout << "-------------------Layer:" << i << "-----------------" << endl;
        this->symbolTable[i].print();
    }
}

bool SymbolTable::checkExistedSymbol(AstNode *node) {
    for (int i = this->currentIndex; i >= 0; i--) {
        SymbolLayer layer = this->symbolTable[i];
        for (int j = 0; j < layer.symbolLayer.size(); j++) {
            SymbolItem *item = &layer.symbolLayer[j];
            if (matchType(item, node)) {
//                cout << item->str<< endl;
                return true;
            }
        }
    }
//    cout << "I can't find the sym: " << node->str << endl;
    return false;
}

SymbolItem *SymbolTable::getExistedSymbol(AstNode *node) {
    for (int i = this->currentIndex; i >= 0; i--) {
        SymbolLayer *layer = &this->symbolTable[i];
        for (int j = 0; j < layer->symbolLayer.size(); j++) {
            SymbolItem *item = &layer->symbolLayer[j];
            if (matchType(item, node)) {
//                cout << item->str << " " << item->params.size() << endl;
                return item;
            }
        }
    }
    return nullptr;
}


bool SymbolTable::checkExistedSymbol(const string &s) {
    for (int i = this->currentIndex; i >= 0; i--) {
        SymbolLayer *layer = &this->symbolTable[i];
        for (int j = 0; j < layer->symbolLayer.size(); j++) {
            SymbolItem *item = &layer->symbolLayer[j];
            if (item->str == s) {
                return true;
            }
        }
    }
//    cout << "I can't find the sym: " << s << endl;
    return false;
}

SymbolItem *SymbolTable::getExistedSymbol(const string& s) {
    for (int i = this->currentIndex; i >= 0; i--) {
        SymbolLayer *layer = &this->symbolTable[i];
        for (int j = 0; j < layer->symbolLayer.size(); j++) {
            SymbolItem *item = &layer->symbolLayer[j];
            if (item->str == s) {
//                cout << item->str << " " << item->params.size() << endl;
                return item;
            }
        }
    }
    return nullptr;
}

bool SymbolTable::checkAllSymbol(const string &s) {
    for (int i = this->globalIndex-1; i >= 0; i--) {
        SymbolLayer *layer = &this->symbolTable[i];
        for (int j = 0; j < layer->symbolLayer.size(); j++) {
            SymbolItem *item = &layer->symbolLayer[j];
            if (item->str == s) {
                return true;
            }
        }
    }
    cout << "I can't find the sym: " << s << endl;
    return false;
}

SymbolItem *SymbolTable::getAllSymbol(const string& s) {
    for (int i = this->globalIndex-1; i >= 0; i--) {
        SymbolLayer *layer = &this->symbolTable[i];
        for (int j = 0; j < layer->symbolLayer.size(); j++) {
            SymbolItem *item = &layer->symbolLayer[j];
            if (item->str == s) {
//                cout << item->str << " " << item->params.size() << endl;
                return item;
            }
        }
    }
    return nullptr;
}

//void SymbolTable::removeTmpVar() {
//    for (int i = this->currentIndex; i >= 0; i--) {
//        vector<SymbolItem> layer = this->symbolTable[i].symbolLayer;
//        for (auto item = layer.begin(); item != layer.end(); item++) {
//            if (item->type == TmpVar) {
//                item = layer.erase(item);
//            }
//        }
//    }
//}

bool matchType(SymbolItem *item, AstNode *node) {
    if (item->str != node->str) {
        return false;
    }
    switch (item->type) {
        case VarSymbol:
        case ConstSymbol:
        case ArraySymbol:
            if (node->nodeType == Ident) {
                return true;
            } else {
                return false;
            }
        case IntFuncSymbol:
        case VoidFuncSymbol:
            if (node->nodeType == FuncCall) {
                return true;
            } else {
                return false;
            }
    }
    return false;
}