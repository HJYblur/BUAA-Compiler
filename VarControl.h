//
// Created by lemon on 2021/12/11.
//

#ifndef PROJECT5_VARCONTROL_H
#define PROJECT5_VARCONTROL_H

#include "vector"
#include "string"

using namespace std;

class VarControl {
public:
    string str;
    int point = 0;
    int shift = 0;
    int dim = 0;
    int dim1, dim2;
    int params = 0;
    string address;
    vector<VarControl *> paramList;

    bool isFParam = false;
    bool isGlobal = false;
    bool isTmpVar = false;

    VarControl(string s);
};

#endif //PROJECT5_VARCONTROL_H
