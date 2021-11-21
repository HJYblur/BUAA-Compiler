//
// Created by lemon on 2021/10/9.
//

#ifndef LEXICAL_H
#define LEXICAL_H
#include "map"
#include "vector"
using namespace std;
typedef std::pair<int, const string> sort; //类别码
//typedef std::pair<std::string, int> content; //存储的单词＋类别码void initCategory();
class Content {
public:
    std::string str;
    int type;
    int line;

    Content(std::string s, int t, int l);

    Content(int t);

    Content();
};

int lexicalScan();

void filterText(std::string &t);

void pushContent(std::string s, int t);

bool isDigit(char c);

bool isLetter(char c);

bool isReserved(std::string &s);

bool isFormatString(std::string &s);

bool isSingleSym(char ch);

bool isDoubleSym(char c1, char c2);

void getToken(std::string s, int &p);

void error(const Content &c, int t);

void initLexicalCategory();

#endif