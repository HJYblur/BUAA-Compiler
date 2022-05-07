//
// Created by lemon on 2021/12/11.
//

#include "VarControl.h"
#include "globalVar.h"

VarControl::VarControl(string s) {
    this->str = std::move(s);
    this->point = controlPoint;
    this->dim = 0;
}


