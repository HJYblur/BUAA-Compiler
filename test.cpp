////
//// Created by lemon on 2021/11/10.
////
//
//#include <iostream>
//#include "grammar.h"
//#include "ast.h"
//#include "globalVar.h"
//
//
////int main() {
////    lexicalScan();
////
////
////    return 0;
////}
//
//AstNode mulExpParser() {
//    AstNode unary = unaryExpParser();
//    AstNode top = AstNode();
//    AstNode* last;
//    AstNode constant;
//    int cnt = 0;
//    while (matchLexicon(18) || matchLexicon(19) || matchLexicon(20)) {
//        cnt++;
//        AstNode subtop = AstNode();
//        if (matchLexicon(18)) {
//            subtop.setNodeType(Multi);
//        } else if (matchLexicon(19)) {
//            subtop.setNodeType(Div);
//        } else {
//            subtop.setNodeType(Mod);
//        }
//        getNextLexicon();//'*/%'
//        subtop.addChild(unary);
//        top.addChild(subtop);
//        top = subtop;
//        last = subtop.getLastChildAddress();
//        unary = unaryExpParser();
//        if (cnt == 1) {
//            constant = top;
//        }
//    }
//    if (cnt) {
//        last->addChild(unary);
//        return constant.children[0];
//    } else {
//        return unary;
//    }
//}
//
//AstNode addExpParser() {
//    AstNode mul = mulExpParser();
//    AstNode top = AstNode();
//    AstNode* last;
//    AstNode constant;
//    int cnt = 0;
//    while (matchLexicon(16) || matchLexicon(17)) {
//        cnt++;
//        AstNode subtop = AstNode();
//        if (matchLexicon(16)) {
//            subtop.setNodeType(Plus);
//        } else {
//            subtop.setNodeType(Minus);
//        }
//        getNextLexicon();//'+-'
//        top.addChild(subtop);
//        subtop.addChild(mul);
//        last = subtop.getLastChildAddress();
//        mul = mulExpParser();
//        top = subtop;
//        if (cnt == 1) {
//            constant = top;
//        }
//    }
//    if (cnt) {
//        last->addChild(mul);
//        return constant.children[0];
//    } else {
//        return mul;
//    }
//}
//
//AstNode relExpParser() {
//    AstNode add = addExpParser();
//    AstNode top = AstNode();
//    AstNode* last;
//    AstNode constant;
//    int cnt = 0;
//    while (matchLexicon(23) || matchLexicon(24) || matchLexicon(25) || matchLexicon(26)) {
//        cnt++;
//        AstNode subtop = AstNode();
//        if (matchLexicon(23)) {
//            subtop.setNodeType(LSS);
//        } else if (matchLexicon(24)) {
//            subtop.setNodeType(LEQ);
//        } else if (matchLexicon(25)) {
//            subtop.setNodeType(GRE);
//        } else {
//            subtop.setNodeType(GEQ);
//        }
//        getNextLexicon();//'><='
//        top.addChild(subtop);
//        subtop.addChild(add);
//        last = subtop.getLastChildAddress();
//        add = addExpParser();
//        top = subtop;
//        if (cnt == 1) {
//            constant = top;
//        }
//    }
//    if (cnt) {
//        last->addChild(add);
//        return constant;
//    } else {
//        return add;
//    }
//}
//
//AstNode eqExpParser() {
//    AstNode rel = relExpParser();
//    AstNode top = AstNode();
//    AstNode* last;
//    AstNode constant;
//    int cnt = 0;
//    while (matchLexicon(27) || matchLexicon(28)) {
//        cnt++;
//        AstNode subtop = AstNode();
//        if (matchLexicon(27)) {
//            subtop.setNodeType(EQL);
//        } else {
//            subtop.setNodeType(NEQ);
//        }
//        getNextLexicon();//'!=/=='
//        top.addChild(subtop);
//        subtop.addChild(rel);
//        last = subtop.getLastChildAddress();
//        rel = relExpParser();
//        top = subtop;
//        if (cnt == 1) {
//            constant = top;
//        }
//    }
//    if (cnt) {
//        last->addChild(rel);
//        return constant.children[0];
//    } else {
//        return rel;
//    }
//}
//
//AstNode lAndExpParser() {
//    AstNode eq = eqExpParser();
//    AstNode top = AstNode();
//    AstNode* last;
//    AstNode constant;
//    int cnt = 0;
//    while (matchLexicon(21)) {
//        cnt++;
//        AstNode subtop = AstNode(And, lexicon.str);
//        getNextLexicon();//'&&'
//        top.addChild(subtop);
//        subtop.addChild(eq);
//        last = subtop.getLastChildAddress();
//        eq = eqExpParser();
//        top = subtop;
//        if (cnt == 1) {
//            constant = top;
//        }
//    }
//    if (cnt) {
//        top.addChild(eq);
//        return constant.children[0];
//    } else {
//        return eq;
//    }
//}
//
//AstNode lOrExpParser() {
//    AstNode lAnd = eqExpParser();
//    AstNode top = AstNode();
//    AstNode constant;
//    int cnt = 0;
//    while (matchLexicon(22)) {
//        cnt++;
//        AstNode subtop = AstNode(Or, lexicon.str);
//        getNextLexicon();//'||'
//        top.addChild(subtop);
//        subtop.addChild(lAnd);
//        lAnd = lAndExpParser();
//        top = subtop;
//        if (cnt == 1) {
//            constant = top;
//        }
//    }
//    if (cnt) {
//        top.addChild(lAnd);
//        return constant.children[0];
//    } else {
//        return lAnd;
//    }
//}