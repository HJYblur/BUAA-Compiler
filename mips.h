//
// Created by lemon on 2021/11/18.
//

#ifndef PROJECT5_MIPS_H
#define PROJECT5_MIPS_H

int getMips();

void toMips(MidCode *midCode);

void constGlobalDeclMips(MidCode *midCode);

void constDeclMips(MidCode *midCode);

void varGlobalDeclMips(MidCode *midCode);

void varDeclMips(MidCode *midCode);

void arrayGlobalDeclMips(MidCode *midCode);

void arrayVarMips(MidCode *midCode);

void arrayGetMips(MidCode *midCode);

void funcDefMips(MidCode *midCode);

void funcParamMips(MidCode *midCode);

void funcEndMips();

void pushVarMips(MidCode *midCode);

void funcCallMips(MidCode *midCode);

void expMips(MidCode *midCode);

void printStrMips(MidCode *midCode);

void printMips(MidCode *midCode);

void getIntMips(MidCode *midCode);

void labelMips(MidCode *midCode);

void gotoMips(MidCode *midCode);

void cmpMips(MidCode *midCode);

void beqMips(MidCode *midCode);

void bneMips(MidCode *midCode);

void bgtMips(MidCode *midCode);

void bgeMips(MidCode *midCode);

void bltMips(MidCode *midCode);

void bleMips(MidCode *midCode);

void returnMips(MidCode *midCode);

string getRegisterS();

void pushMips(const string &s);

void printMips(ofstream &file);

void pushSP(string s);

void popSP(string s);

void popSP();

#endif //PROJECT5_MIPS_H
