//
// Created by lemon on 2021/10/30.
//

#include "error.h"
#include <fstream>
#include "iostream"
#include "algorithm"
#include "globalVar.h"

using namespace std;
vector<Error> errorList;

Error::Error(char t, int l) {
    this->type = t;
    this->line = l;
}

bool compare(Error a, Error b) {
    if (a.line != b.line) {
        return a.line < b.line;
    } else {
        return a.type < b.type;
    }
}

bool equal(Error a, Error b) {
    return a.line == b.line && a.type == b.type;
}

void printError(ofstream &file) {
    std::sort(errorList.begin(), errorList.end(), compare);
    for (int i = 0; i < errorList.size(); i++) {
        bool flag = true;
        Error item = errorList[i];
        if (i > 0) {
            Error pre = errorList[i - 1];
            if (pre.line == item.line && pre.type == item.type) {
                flag = false;
            }
        }
        if (flag) {
//        cout << item.line << " " << item.type << endl;
            file << item.line << " " << item.type << endl;
        }
    }
}