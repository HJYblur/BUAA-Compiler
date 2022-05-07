//
// Created by lemon on 2021/11/18.
//

#ifndef PROJECT5_MIPS_H
#define PROJECT5_MIPS_H
#include "VarControl.h"
int getMips();

void toMips(MidCode *midCode);

void constGlobalDeclMips(MidCode *midCode);

void constDeclMips(MidCode *midCode);

void varGlobalDeclMips(MidCode *midCode);

void varDeclMips(MidCode *midCode);

void arrayGlobalDeclMips(MidCode *midCode);

void arrayVarMips(MidCode *midCode);

void arrayDeclMips(MidCode *midCode);

void arrayGetMips(MidCode *midCode);

void funcDefMips(MidCode *midCode);

void funcParamMips(MidCode *midCode);

void funcEndMips(MidCode( *midCode));

void pushVarMips(MidCode *midCode);

void funcCallMips(MidCode *midCode);

void mainFuncMips();

void expMips(MidCode *midCode);

void printStrMips(MidCode *midCode);

void printMips(MidCode *midCode);

void getIntMips(MidCode *midCode);

void labelMips(MidCode *midCode);

void gotoMips(MidCode *midCode);

void cmpMips(MidCode *midCode);

void compareMips(MidCode *midCode);

void returnMips(MidCode *midCode);

void pushMips(const string &s);

void printMips(ofstream &file);

void blockBegin();

void blockEnd();

void getVar(const VarControl& var, const string &s);

void storeVar(VarControl *var, const string &s);

void storeVar(VarControl *var);

void putVar(const VarControl& var, const string &s);

void putVar(int shift, const string &s);

void pushSP(int shift);

void popSP(int shift);

void controlPush(VarControl var);

bool checkVar(const string& name);

VarControl *searchVar(const string& name);

#endif //PROJECT5_MIPS_H
