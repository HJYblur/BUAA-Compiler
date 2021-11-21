//
// Created by lemon on 2021/9/24.
//
#include <iostream>
#include <fstream>
#include "string"
#include "vector"
#include "map"
#include "lexical.h"
#include "error.h"
#include "globalVar.h"

using namespace std;

map<int, string> category;
string filteredText;
string text;
int fileLength;//输入程序的行数
int line = 1;//当前字符的行数
vector<Content> storage;

int lexicalScan() {
    //step1.打开文件&初始化
    ifstream srcFile("testfile.txt", ios::in); //以文本模式打开in.txt备读
    if (!srcFile) { //打开失败
        cout << "error opening source file." << endl;
        return 0;
    }
    initLexicalCategory();

    //step2.将文件整体读入容器中
    string s;
    while (getline(srcFile, s)) {
        text += s;
        text += '\n';
//        destFile << s << endl;
        fileLength++;
    }
//    cout << fileLength << endl; //输出源程序总行数

    //step3.对文件进行预处理，消除注释
    filterText(filteredText);
//    cout << filteredText << endl; //检查消除注释后的文本情况

    //step4.将容器传入分析程序进行分析
    if (filteredText.length() != 0) {
        int p = 0;
        while (p < filteredText.length()) {
            getToken(filteredText, p);
        }
    }
//    for (int i = 0; i < storage.size(); i++) {
//        cout << (storage[i]).str << ", in line " << (storage[i].line) << endl;
//    }

    //step5.关闭文件，结束程序
    srcFile.close();
    return 0;
}

void filterText(string &ft) {
    int i;
    for (i = 0; i < text.length(); i++) {
        if (text[i] == '"') {
            ft += text[i++];
            while (text[i] != '"') {
                ft += text[i++];
            }
            ft += text[i++];
        }
        if (text[i] == '/' && text[i + 1] == '/') {
            while (text[i] != '\n') {
                i++;
            }
        }
        if (text[i] == '/' && text[i + 1] == '*') {
            i += 2;
            while (text[i] != '*' || text[i + 1] != '/') {
                i++;
                if (i == text.length()) {
                    return;
                }
            }
            i += 2;
            ft += ' ';
        }
        if (text[i] == '/' && (text[i + 1] == '/' || text[i + 1] == '*')) {
            i--;
            continue;
        } else {
            ft += text[i];
        }
    }
}

void pushContent(string s, int type) {
    Content one(std::move(s), type, line);
    storage.emplace_back(one);
}

void getToken(string s, int &p) {
    while (s[p] == ' ' || s[p] == '\r' || s[p] == '\n' || s[p] == '\t') { //消除空格
        if (s[p] == '\n') {
            line++;
        }
        p++;
    }
    if (p == s.length()) return;
    string token;
    if (isLetter(s[p])) {
        token += s[p++];
        while (isLetter(s[p]) || isDigit(s[p])) token += s[p++];
        //token += '\0';
        if (isReserved(token)) return; //op4
        else { //op1
            pushContent(token, 0);
            return;
        }
    } else if (isDigit(s[p])) {
        token += s[p++];
        while (isDigit(s[p])) token += s[p++];
        pushContent(token, 1);//op2
        return;
    } else if (s[p] == '"') {
        token += s[p++];
        int temp = p;
        while (s[temp] != '"' && temp < s.length()) token += s[temp++];
        token += s[temp];
        if (isFormatString(token)) {
            pushContent(token, 2);//op3
            p = temp;
            p++;
            return;
        } else {
            return;
        }
    } else if (s[p] == '<' || s[p] == '>' || s[p] == '!' || s[p] == '=') {
        if (s[p + 1] == '=') {
            switch (s[p]) {
                case '<' :
                    pushContent("<=", 24);
                    break;
                case '>' :
                    pushContent(">=", 26);
                    break;
                case '!':
                    pushContent("!=", 28);
                    break;
                case '=':
                    pushContent("==", 27);
                    break;
                default:
                    return;
            }
            p += 2;
        } else {
            switch (s[p]) {
                case '<' :
                    pushContent("<", 23);
                    break;
                case '>' :
                    pushContent(">", 25);
                    break;
                case '!':
                    pushContent("!", 15);
                    break;
                case '=':
                    pushContent("=", 29);
                    break;
                default:
                    return;
            }
            p++;
        }
    } else if (isSingleSym(s[p])) {
        p++;
        return;
    } else if (isDoubleSym(s[p], s[p + 1])) {
        p += 2;
        return;
    } else {
        cout << "There's something wrong at " << p << " ,and s[p] is " << s[p] << endl;
        p++;
        return;
    }
}

Content::Content(std::string s, int t, int l) {
    str = std::move(s);
    type = t;
    line = l;
}

bool isDigit(char c) {
    if (c >= '0' && c <= '9') return true;
    else return false;
}

bool isLetter(char c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') return true;
    else return false;
}

bool isReserved(string &s) {
    if (s == "main") {
        pushContent(s, 3);
    } else if (s == "const") {
        pushContent(s, 4);
    } else if (s == "int") {
        pushContent(s, 5);
    } else if (s == "break") {
        pushContent(s, 6);
    } else if (s == "continue") {
        pushContent(s, 7);
    } else if (s == "if") {
        pushContent(s, 8);
    } else if (s == "else") {
        pushContent(s, 9);
    } else if (s == "getint") {
        pushContent(s, 10);
    } else if (s == "printf") {
        pushContent(s, 11);
    } else if (s == "return") {
        pushContent(s, 12);
    } else if (s == "while") {
        pushContent(s, 13);
    } else if (s == "void") {
        pushContent(s, 14);
    } else {
        return false;
    }
    return true;
}

bool isFormatString(string &s) {
    if (s.length() < 2) {
        return false;
    }
    int i;
    for (i = 1; i < s.length() - 1; i++) {
        if (s[i] == 32 || s[i] == 33 || (s[i] >= 40 && s[i] <= 126)) {
            if (s[i] == '\\' && i < s.size() - 1) {
                if (s[i + 1] != 'n') {
                    errorList.emplace_back('a', line);
                    return true;
                }
            }
        } else if (s[i] == '%') {
            if (i < s.size() - 1 && s[i + 1] == 'd') {
                continue;
            } else {
                errorList.emplace_back('a', line);
                return true;
            }
        } else {
            errorList.emplace_back('a', line);
            return true;
        }
    }
    return true;
}

bool isSingleSym(char ch) {
    if (ch == '+') {
        pushContent("+", 16);
    } else if (ch == '-') {
        pushContent("-", 17);
    } else if (ch == '*') {
        pushContent("*", 18);
    } else if (ch == '/') {
        pushContent("/", 19);
    } else if (ch == '%') {
        pushContent("%", 20);
    } else if (ch == ',') {
        pushContent(",", 30);
    } else if (ch == ';') {
        pushContent(";", 31);
    } else if (ch == '(') {
        pushContent("(", 32);
    } else if (ch == ')') {
        pushContent(")", 33);
    } else if (ch == '[') {
        pushContent("[", 34);
    } else if (ch == ']') {
        pushContent("]", 35);
    } else if (ch == '{') {
        pushContent("{", 36);
    } else if (ch == '}') {
        pushContent("}", 37);
    } else {
        return false;
    }
    return true;
}

bool isDoubleSym(char c1, char c2) {
    if (c1 == '&' && c2 == '&') {
        pushContent("&&", 21);
        return true;
    } else if (c1 == '|' && c2 == '|') {
        pushContent("||", 22);
        return true;
    } else {
        return false;
    }
}

void initLexicalCategory() {
    //op1：标识符
    category.insert(sort(0, "IDENFR"));//ident
    //op2：数值常量
    category.insert(sort(1, "INTCON"));//IntConst
    //op3：格式字符串终结符
    category.insert(sort(2, "STRCON"));//FormatString

    //op4：保留字
    category.insert(sort(3, "MAINTK"));//main
    category.insert(sort(4, "CONSTTK"));//const
    category.insert(sort(5, "INTTK"));//int
    category.insert(sort(6, "BREAKTK"));//break
    category.insert(sort(7, "CONTINUETK"));//continue
    category.insert(sort(8, "IFTK"));//if
    category.insert(sort(9, "ELSETK"));//else
    category.insert(sort(10, "GETINTTK"));// getint
    category.insert(sort(11, "PRINTFTK"));// printf
    category.insert(sort(12, "RETURNTK"));// return
    category.insert(sort(13, "WHILETK"));// while
    category.insert(sort(14, "VOIDTK"));// void

    //op5:单运算符
    category.insert(sort(15, "NOT"));// ! Mention!=
    category.insert(sort(16, "PLUS"));// +
    category.insert(sort(17, "MINU"));// -
    category.insert(sort(18, "MULT"));// *
    category.insert(sort(19, "DIV"));// /
    category.insert(sort(20, "MOD"));// %

    //op6:双运算符
    category.insert(sort(21, "AND"));// &&
    category.insert(sort(22, "OR"));// ||

    //op7:需判断的比较符号
    category.insert(sort(23, "LSS"));// <
    category.insert(sort(24, "LEQ"));// <=
    category.insert(sort(25, "GRE"));// >
    category.insert(sort(26, "GEQ"));// >=
    category.insert(sort(27, "EQL"));// ==
    category.insert(sort(28, "NEQ"));// !=
    category.insert(sort(29, "ASSIGN"));// =

    //op8：连接符
    category.insert(sort(30, "COMMA"));// ,
    category.insert(sort(31, "SEMICN"));// ;
    category.insert(sort(32, "LPARENT"));// (
    category.insert(sort(33, "RPARENT"));// )
    category.insert(sort(34, "LBRACK"));// [
    category.insert(sort(35, "RBRACK"));// ]
    category.insert(sort(36, "LBRACE"));// {
    category.insert(sort(37, "RBRACE"));// }
}