//
// Created by lemon on 2021/10/30.
//

#ifndef PROJECT5_ERROR_H
#define PROJECT5_ERROR_H

#include "error.h"
#include "fstream"
#include "vector"

class Error {
public:
    char type;
    int line;

    Error(char t, int l);
};

bool compare(Error a,Error b);

bool equal(Error a, Error b);

void printError(std::ofstream &file);
#endif //PROJECT5_ERROR_H
