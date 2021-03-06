[TOC]

## 整体架构

![流程图](./documents/编译流程图.png)

## 具体实现

### 词法分析

1. 设计思路

   将词法分析这一任务划分为四个阶段：读入文件、消除注释、解析词法以及输出变量。其中，用string类型变量text存取读入的所有文件内容，用vector容器storage存储经过词法分析后的所有变量。同时，由于词法分析的结果需要包含原内容和类别，因此定义`pair<std::string, int>`为`content`作为存储其输出结果的类型。

2. 主要操作

   ```c++
   void initCategory(); //用map型变量category存储各种词法的类型，initCategory进行初始化。
   
   void filterText(string &t);//消除文件中的所有注释
   
   bool isDigit(char c);//判断是否是数字
   
   bool isLetter(char c);//判断是否是字母或者下划线
   
   bool isReserved(string &s);//判断是否是关键字
   
   bool isFormatString(string &s);//判断是否是格式化字符串
   
   bool isSingleSym(char ch);//判断是否是单符号（如+，-，*，/等）
   
   bool isDoubleSym(char c1, char c2);//判断是否是双符号（如!=等）
   
   void scanner(string s, int &p);//扫描函数，将输入程序中的每个元素提取出并送入相应子函数进行扫描
   
   void print(ofstream &file, vector<content> &store);//打印得到的输出结果
   
   void error();//错误程序，为之后的错误处理留出空间
   ```

3. 关键解析

   ##### filterText函数

   ```c++
   void filterText(string &ft) {
       int i;
       for (i = 0; i < text.length(); i++) {
           //注释内的所有内容应该全部保留
           if (text[i] == '"') {
               ft += text[i++];
               while (text[i] != '"') {
                   ft += text[i++];
               }
               ft += text[i++];
           }
           //单行注释，清除同一行内注释后的所有内容
           if (text[i] == '/' && text[i + 1] == '/') {
               while (text[i] != '\n') {
                   i++;
               }
           }
           //双行注释
           if (text[i] == '/' && text[i + 1] == '*') {
               i += 2;
               while (text[i] != '*' || text[i + 1] != '/') {
                   i++;
                   if (i == text.length()) {
                       error();
                       return;
                   }
               }
               i += 2;
               ft += ' ';
           }
           //预防两个注释连在一起的情况
           if (text[i] == '/' && (text[i+1] == '/' || text[i+1] == '*')) {
               i--;
               continue;
           } else {
               ft += text[i];
           }
       }
   }
   ```


### 语法分析

1. 设计思路：

   考虑到之后的开发会逐层递增，因此将关键定义、跨文件全局变量以及函数声明都放入头文件中。将词法分析修改为`lexical.cpp`和`lexical.h`，并新建`grammar.cpp`和`grammar.h`。例如，将原词法分析的结果存储在vector容器`storage`中，并通过在头文件中声明其为extern类型变量保证可以在语法分析程序中使用。

   为每个非终结符新建一个函数，作为递归下降将要调用的子函数。之后，开始从`storage`中`pop`出词法分析的结果（*注：词法分析的结果存储在`content`类型的变量中，由于`storage`是普通的`vector`结构，此处的`pop`应该从`storage[0]`开始。*）对于每个`content`，判断其所属的语法结构，再传入语法分析的函数中。例如将`pop`出的第一个`content`传入`compUnit()`中。

2. 主要操作

   为每个非终结符分别建立一个分析子函数，并实现`push`和`pop`的相关操作。

   ```C++
   //
   // Created by lemon on 2021/10/9.
   //
   
   #ifndef PROJECT3_GRAMMAR_H
   #define PROJECT3_GRAMMAR_H
   extern std::vector<content> storagePlus;
   
   void initGrammarCategory();
   
   void push(int op);//将语法分析得到的结果传入新容器中
   
   bool getNextSym();//pop词法分析的结果
   
   bool stepBack();//push词法分析的结果
   
   bool match(int op);//判断词法分析的类别是否匹配
   
   bool compUnit();//以下均为每个非终结符的分析子函数
   bool decl();
   bool constDecl();
   bool bType();
   bool constDef();
   bool constInitVal();
   bool varDecl();
   bool varDef();
   bool initVal();
   bool funcDef();
   bool mainFuncDef();
   bool funcType();
   bool funcFParams();
   bool funcFParam();
   bool block();
   bool blockItem();
   bool stmt();
   bool exp();
   bool cond();
   bool lVal();
   bool primaryExp();
   bool number();
   bool unaryExp();
   bool unaryOp();
   bool funcRParams();
   bool mulExp();
   bool addExp();
   bool relExp();
   bool eqExp();
   bool lAndExp();
   bool lOrExp();
   bool constExp();
   
   void print(std::ofstream &file, std::vector<content> &store);//打印内容
   
   void error(int op);//预留的错误处理接口
   
   #endif //PROJECT3_GRAMMAR_H
   ```

3. 关键解析

   语法分析最应该考虑的是预读问题。对于不同的语法定义，怎么预读、预读多少个，都是问题。下面以`compUnit`为例体现预读的方法。

   CompUnit的语法定义：`CompUnit → {Decl} {FuncDef} MainFuncDef `

   ```C++
   bool compUnit() {
       while (match(4) || match(5)) {//预读：如果读到int或者const，判断是不是decl
           if (match(4)) {//const
               decl();
           } else {//int
               getNextSym();
               if (match(0)) {//ident
                   getNextSym();
                   if (match(32)) {//main
                       stepBack();//表明此处应该是MainFuncDef，回退
                       stepBack();
                       break;
                   } else {//反之，则是decl，回退后进入子函数
                       stepBack();
                       stepBack();
                       decl();
                   }
               } else {
                   stepBack();
                   break;
               }
           }
       }
       while (match(5) || match(14)) {//预读判断是不是funcDef，同理
           if (match(5)) {
               getNextSym();
               if (match(0)) {
                   stepBack();
                   funcDef();
               } else {
                   stepBack();
                   break;
               }
           } else {
               funcDef();
           }
       }
       if (mainFuncDef()) {//处理完Decl和FuncDef，开始读取MainFuncDef
           stepBack();
           push(38);
           return true;
       } else {
           error();
           return false;
       }
   }
   ```

   对于其他语法，需要判断预读的方法。由于语法较多且较冗杂，用大量测试数据进行检验可以较为有效地发现问题。

### 错误处理

在建立ast失败后选择直接在语法分析的基础上进行错误处理，即边递归下降边进行错误处理。

观察了错误处理给出的错误定义后，可以将对应的错误简单分为x大类。第一类是针对符号，包括名字重定义和未定义的名字，需要借助符号表进行查看；第二类是针对函数，包括对函数参数个数、类型以及return语句的检查；第三类主要是较为基础的语法错误，主要检查是否有非法符号、常量的值是否被改变、分号、右小括号、右中括号等是否匹配，以及printf和break，continue等语句是否正确使用。

以下就三类错误分别举例说明。

**第一类错误**主要借助建立、查看符号表进行判断，因此符号表的建立是关键。此处我们采取栈式符号表，每到新的层次执行入栈出栈操作。

其中，符号表元素的结构定义为：

```c++
class Symbol {

public:
    symbolType type = null;     //符号类型
    string str;                 //内容
    int index;                  //层次
    int dim = 0;                //数组维数
    vector<Symbol> params;      //函数/数组参数
    bType btype = unknown;      //变量类型
    bType returnType = unknown; //函数返回值
}
```

符号和返回值的类型分别有：

```c++
enum symbolType {
    var, constVar, func, returnParam, null
};
enum bType {
    intType, voidType, unknown
};
```

当识别到ident时，调用函数进行判断即可。

```c++
bool checkMultiIdent(Symbol symbol, int line, int index) {
    int size = symbolTable.size();
    for (int i = size - 1; i >= 0; i--) {
        Symbol item = symbolTable[i];
        if (item.index < index) {
            break;
        }
        if (item.str == symbol.str && item.index >= index) {
            if (item.type == func && symbol.type != func) {
                continue;
            } else {
                errorList.emplace_back('b', line);
                return false;
            }
        }
    }
    return true;
}

bool checkNoIdent(Symbol symbol) {
    for (int i = symbolTable.size() - 1; i >= 0; i--) {
        Symbol item = symbolTable[i];
        if (item.str == symbol.str && item.index <= symbol.index) {
            return true;
        }
    }
    errorList.emplace_back('c', token.line);
    return false;
}
```

**第二类错误**则主要在函数定义和函数调用时进行判断：

以检查函数参数为例，需要判断实参和形参的类型、维数是否一致，如果不一致，则报错。

```c++
bool checkParam(Symbol s, Content c) {
    //形参     实参
    for (int i = symbolTable.size() - 1; i >= 0; i--) {
        Symbol item = symbolTable[i];
        if (item.str == c.str) {
            if (item.type != s.type || item.dim - globalDim != s.dim) {
                errorList.emplace_back('e', c.line);
                return false;
            } else {
                return true;
            }
        }
    }
    return false;
}
```

**第三类错误**是基本的语法错误，在调用词法分析程序进行语法分析时单独判断即可：

以判断`')'`为例：

```c++
if (match(33)) {
    getNextToken();
} else {
    stepBack();
    errorList.emplace_back('j', token.line);
    getNextToken();
}
```

`match（33）`对应的即是判断当前输入的词法符号是不是`')'`。

### ast

考虑到生成中间代码需要较为方便的结构，因此决定重新生成ast，以便于中间代码及mips代码的生成。

其中，ast节点的结构定义为：

```c++
class AstNode {
public:
    NodeType nodeType = Unknown;//ast节点类型
    string str;                 //名称
    int val = 0;                //值
    int dim = 0;                //数组维数，可取0,1,2
    bool used = false;          //判断是否已经用于生成中间代码
    bool isFuncParam = false;   //判断是否是函数参数
    vector<AstNode> params;     //函数、数组参数
    vector<AstNode> children;   //存储子节点
    AstNode *father;            //存储父节点（好像没用）
}
```

nodeType有以下类型：

```C++
enum NodeType {
    ConstDeclList, VarDeclList, FuncDefList, CompUnit, ConstDecl, ConstInitVal, VarDecl, InitVal,
    FuncDef, MainFuncDef, FuncFParam, Block, Stmt, Exp, IfCond, IfBody, WhileCond, WhileBody, ConstExp,
    MulExp, AddExp, RelExp, EqExp, LAndExp, LOrExp,
    Number, FuncCall, Ident, FormatString, ConstTK, IntTK, VoidTK, BreakTK, ContinueTK,
    IfTK, ElseBody, GetintTK, PrintfTK, ReturnTK, WhileTK,
    Not, Plus, Minus, Multi, Div, Mod, And, Or, LSS, LEQ, GRE, GEQ, EQL, NEQ, Assign,
    Unknown
};
```

其余操作和语法分析类似，但是存储结构变成了上述所示的树结构。

### 生成中间代码

首先，给出每个中间代码的定义：

此处的中间代码采用四元组的格式，所以理论上,`op1`,`op2`,`opRand`以及`result`四个子对象就可以包含每个四元式需要存储的所有内容。其他的一些子对象主要用于辅助存储，记录上述四个对象的一些信息。例如，`opNum`即可以记录`exp`的类型（可以是形如`a=b`，也可以是形如`a=b+c`）等，也可以记录数组的维数。`resultType`主要用于记录某个变量是数字还是ident，前者记为0，后者记为1。

```c++
class MidCode {
public:
    MidCodeType type;
    int opNum = 2;
    string op1;
    string op2;
    string opRand = "";
    string result;
    int paraNumber = 0;
    int resultType = 0;//0:number 1:exp
    //0:left number 1:right number 2:both exp

    int dim1 = 0, dim2 = 0;
}
```

规定中间代码分为以下类型，并针对每个类型给出解释和说明，以及其`to_string`函数，便于后期调试。

| 名称         | 格式                    | 说明       |
| ------------ | ----------------------- | ---------- |
| ConstDeclMid | const int ident         | 常量说明   |
| VarDeclMid   | var int ident           | 变量说明   |
| ArrayDeclMid | array[n]                | 数组定义   |
| ArrayVarMid  | array[n] = x            | 数组赋值   |
| ArrayGetMid  | x= array[n]             | 数组取值   |
| FuncDefMid   | void/int fun()          | 函数定义   |
| FuncParamMid | param ident             | 函数参数   |
| PushVar      | push ident              | 函数传参   |
| FuncCallMid  | call fun                | 函数调用   |
| FuncEndMid   | funcEnd                 | 函数结束   |
| MainFuncMid  | Mainfun                 | 主函数调用 |
| BlockBegin   | blockBegin              | 块开始     |
| BlockEnd     | blockEnd                | 块结束     |
| ExpMid       | result = op1 opRand op2 | 表达式     |
| PrintfMid    | printf str/int          | 输出       |
| GetIntMid    | getint ident            | 输入       |
| Label        | label:                  | 标签       |
| Goto         | goto label              | 跳转       |
| Cmp          | cmp identa,identb       | 比较并跳转 |
| Return       | return label            | 返回       |
| Compare      | cmp identa,identb       | 比较       |

```c++
enum MidCodeType {
    ConstDeclMid, VarDeclMid, ArrayDeclMid, ArrayVarMid, ArrayGetMid,
    FuncDefMid, FuncParamMid, PushVar, FuncCallMid, FuncEndMid, MainFuncMid, BlockBegin, BlockEnd,
    ExpMid, PrintfMid, GetIntMid, Label, Goto, Cmp, Return, Compare,
};
string MidCode::toString() const {
    switch (type) {
        case ConstDeclMid:
            return "const int " + this->op1 + " = " + this->op2;
        case VarDeclMid:
            if (this->opNum == 2) {
                return "var int " + this->op1 + " = " + this->op2;
            } else {
                return "var int " + this->op1;
            }
        case ArrayDeclMid:
            if (this->opNum == 2) {
                return "arr int " + this->result + "[" + this->op1 + "][" + this->op2 + "]";
            } else {
                return "arr int " + this->result + "[" + this->op1 + "]";
            }
        case ArrayVarMid:
            if (this->opNum == 2) {
                return this->opRand + "[" + this->op1 + "][" + this->op2 + "] = " + this->result;
            } else {
                return this->opRand + "[" + this->op1 + "] = " + this->result;
            }
        case ArrayGetMid:
            return this->result + " = " + this->opRand + "[" + this->op1 + "]";
        case FuncDefMid:
            if (this->resultType == 0) {
                return "int " + this->op1 + "()";
            } else {
                return "void " + this->op1 + "()";
            }
        case FuncParamMid:
            if (this->opNum == 0) {
                return "para int " + this->opRand;
            } else if (this->opNum == 1) {
                return "para int " + this->opRand + "[" + this->op1 + "]";
            } else {
                return "para int " + this->opRand + "[][" + this->op2 + "]";
            }
        case MainFuncMid:
            return this->op1;
        case ExpMid:
            if (this->opNum == 0) {
                return this->result + "=" + this->op1;
            } else if (this->opNum == 1) {
                return this->result + " = " + this->opRand + " " + this->op1;
            } else {
                return this->result + " = " + this->op1 + " " + this->opRand + " " + this->op2;
            }
        case GetIntMid:
            return "getint " + this->result;
        case PushVar:
            return "push " + this->op1;
        case FuncCallMid:
            return "call " + this->op1;
        case Label:
            return this->op1 + ":";
        case Goto:
            return "goto " + this->op1;
        case Cmp:
            return this->opRand + " " + this->op1 + "," + this->op2 + "," + this->result;
        case Compare:
            return this->opRand + " " + this->result + "," + this->op1 + "," + this->op2;
        case Return:
            if (this->opNum == 0) {
                return "return";
            } else {
                return this->op1 + " " + this->op2;
            }
        case PrintfMid:
            if (this->opNum == 1) {
                return "print \"" + this->op1 + "\"";
            } else {
                return "print " + this->op2;
            }
        case BlockBegin:
            return "------------------------BlockBegin------------------------";
        case BlockEnd:
            return "-------------------------BlockEnd-------------------------";
    }
    return "";
}
```

##### 编码前的符号表设计

自己构想了一个三级的符号表架构：首先建立`symbleTable`，存储所有的`symbolLayer`，每个`symbolLayer`又存储所有的`symbolItem`。`symbolLayer`的`index`属性记录了当前处于哪一层（遇到block则进入下一层，反之跳出block则返回上一层）。但是这样的设计只考虑了不同layer之间符号的前后关系，没有考虑同一层不同的block的符号之间的前后顺序，所以会出错。

考虑到上述问题后，我为`symbolItem`添加了`blockNum`属性，记录当前的`block`序号。判断、检查符号表时，需要同时检查layer和blockNum，当两者均符合条件时才能得到正确的符号。

```c++
class SymbolItem {

public:
    symbolItemType type;        //符号类型
    string str;                 //内容
    int value = 0;              //常量的值
    int dim = 0;                //数组维数/函数参数个数
    int dim1 = 0, dim2 = 0;      //数组两个维数的值
    int blockNum = 0;
    vector<SymbolItem> params;  //函数/数组参数
}

class SymbolLayer {
public:
    int index = 0;
    int blockNum = 0;
    vector<SymbolItem> symbolLayer;
}

class SymbolTable {
public:
    int currentIndex = -1;
    int globalIndex = 0;
    vector<SymbolLayer> symbolTable;
}
```

但是！我还忽略了一个问题，那就是函数参数。对于函数参数而言，它的层次应该和函数内部变量相等，但这个架构下符号表遇到block才会进入下一层；也就是说，这一操作导致了函数参数和函数定义在同一层，因此后续又出了很多问题。

基于这个问题，我设计了一个基本的检验数据：

```c
const int a;
void f(int a) {
    int a = 10;
    printf("%d\n",a);
}
int main(){
    int a = a*10;
    f(a);
}
```

如果上述所有符号都能判断正确，那么符号表就算成功。在这一前提下，我认为原有的架构无法实现正确识别。因此，进行了重构。

##### 编码后的符号表实现

俗话说，返璞归真。我观察发现，在生成mips代码时，几乎没有用到midcode中的符号表，所以决定建立两个符号表分别使用。最后，我也选择在生成midcode时建立栈式符号表。

临时变量也需要存入符号表，由于保证生成的临时变量均为“#”开头，所以不会与实际变量重名。

```c++
vector<SymbolItem> symTable;
void blockMid(AstNode *node) {
    //新建一层符号表
    currentLayer++;
    MidCode begin = MidCode(BlockBegin);
    pushMidCode(begin);
    node->setAllUsed();
    for (int i = 0; i < node->children.size(); i++) {
        toMidCode(&node->children[i]);
    }
    //回退符号表
    for (auto iter = symTable.begin(); iter != symTable.end();) {
        if (iter->layer == currentLayer) {
            iter = symTable.erase(iter);
        } else {
            iter++;
        }
    }
    currentLayer--;
    MidCode end = MidCode(BlockEnd);
    pushMidCode(end);
}
```

在block中的操作如上所示。

中间代码生成部分整体的思路是语法制导翻译。基于上述类型符号表，可以开始进行中间代码的生成。对于不同类型的ast节点，我们可以对应生成不同的中间代码。下面以`assignMid`为例简要介绍。

```c++
void assignMid(AstNode *node) {
    node->setAllUsed();
    //a = getint()
    if (node->children[1].nodeType == GetintTK) {
        AstNode ident = node->children[0];
        if (ident.dim == 0) {
            MidCode getInt = MidCode(GetIntMid);
            getInt.setResult(ident.str);
            getInt.setOp1("GetInt");
            getInt.setOpNum(1);
            pushMidCode(getInt);
        } else if (ident.dim == 1) {
            MidCode getInt = MidCode(GetIntMid);
            getInt.setResult(getTmpVar());
            getInt.setOp1("GetInt");
            getInt.setOpNum(1);
            pushMidCode(getInt);

            SymbolItem item = SymbolItem(Ident);
            item.setIsTmp();
            item.setDim(0);
            item.setString(getInt.result);
            addSymbol(item);

            MidCode getArr = MidCode(ArrayVarMid);
            AstNode x = expMid(&ident.params[0]);
            AstNode ans = getAddress(&x);
            getArr.setResult(getInt.result);
            getArr.setOpRand(ident.str);
            getArr.setOp1(ans.str);
            if (ans.nodeType != Number) {
                getArr.dim1 = 1;
            }
            getArr.setOpNum(1);
            getArr.setResultType(1);
            pushMidCode(getArr);
        } else {
            MidCode getInt = MidCode(GetIntMid);
            getInt.setResult(getTmpVar());
            getInt.setOp1("GetInt");
            getInt.setOpNum(1);
            pushMidCode(getInt);

            SymbolItem item = SymbolItem(Ident);
            item.setIsTmp();
            item.setDim(0);
            item.setString(getInt.result);
            addSymbol(item);

            int dim2 = 0;
            if (checkExistedSymbol(ident.str)) {
                SymbolItem *arr = getExistedSymbol(ident.str);
                dim2 = arr->dim2;
            }
            MidCode getArr = MidCode(ArrayVarMid);
            AstNode x = expMid(&ident.params[0]);
            AstNode y = expMid(&ident.params[1]);
            AstNode ans = getAddress(&x, &y, dim2);
            getArr.setResult(getInt.result);
            getArr.setOpRand(ident.str);
            getArr.setOp1(ans.str);
            getArr.setOpNum(1);
            if (ans.nodeType != Number) {
                getArr.dim1 = 1;
            }
            getArr.setResultType(1);
            pushMidCode(getArr);
        }
    }
    //ident = exp
    else {
        AstNode ident = node->children[0];
        AstNode expNode = expMid(&node->children[1]);
        bool flag = true;
        if (!checkExistedSymbol(ident.str)) {
            return;
        }
        SymbolItem *item = getExistedSymbol(ident.str);
        if (item->isFParam) {
            flag = false;
        }
        if (ident.dim == 0) {
            //符号表赋值
            if (flag) {
                item->setValue(expNode.val);
            }
            //中间代码
            MidCode exp = MidCode(ExpMid);
            exp.setResult(ident.str);
            exp.setOp1(expNode.str);
            exp.setOpNum(0);
            if (expNode.nodeType != Number) {
                exp.setResultType(1);
            }
            pushMidCode(exp);
        } else if (ident.dim == 1) {
            //赋值
            AstNode tmp = getAddress(&ident.params[0]);
            int index = tmp.val;
            //中间代码
            MidCode arr = MidCode(ArrayVarMid);
            arr.setOpRand(ident.str);
            arr.setOp1(to_string(index));
            arr.setResult(expNode.str);
            arr.setOpNum(1);
            if (tmp.nodeType != Number) {
                arr.dim1 = 1;
                arr.setOp1(tmp.str);
            }
            if (expNode.nodeType != Number) {
                arr.setResultType(1);
            }
            pushMidCode(arr);
        } else {
            //赋值
            AstNode x = expMid(&ident.params[0]);
            AstNode y = expMid(&ident.params[1]);
            AstNode ans = getAddress(&x, &y, item->dim2);
            //中间代码
            MidCode arr = MidCode(ArrayVarMid);
            arr.setOpRand(ident.str);
            arr.setOp1(ans.str);
            arr.setResult(expNode.str);
            arr.setOpNum(1);
            if (ans.nodeType != Number) {
                arr.dim1 = 1;
            }
            if (expNode.nodeType != Number) {
                arr.setResultType(1);
            }
            pushMidCode(arr);
        }
    }
}
```

如上所示，先判断是否是“getint”类，如果不是，则是赋值操作。判断`Astnode`的维数后即可知道是变量、一维数组还是二维数组。如果是变量，直接生成`exp`型中间代码即可；反之，需要先计算数组对应的存储地址，并讲该地址的值存储在临时变量中，最后用临时变量生成`exp`型中间代码。

针对条件表达式，我的做法是：先引入`condExp`，判断属于哪种类型的表达式，并传入。`zeroMid`对应表达式，需要和0进行比较。

```c++
bool condMid(AstNode *node, const string &ifBody, const string &elseBody, bool flag) {
    node->setAllUsed();
    switch (node->nodeType) {
        case LOrExp:
            isSingleCond = false;
            return lOrMid(node, ifBody, elseBody);
        case LAndExp:
            isSingleCond = false;
            return lAndMid(node, ifBody, elseBody);
        case EqExp:
            return eqMid(node, ifBody, elseBody, flag);
        case RelExp:
            return relMid(node, ifBody, elseBody, flag);
        default:
            return zeroMid(node, ifBody, elseBody, flag);
    }
}
```

考虑到存在短路求值的情况，引入变量`flag`,`flag==true`对应`lOr`，反之对应`lAnd`。在或表达式中，如果满足第一个条件，则直接跳到body，反之，跳到第二个条件，如此往复。而对于与表达式，如果不满足第一个条件，则直接跳到else，反之，跳到第二个条件，如此往复。对于`eqExp`和`relExp`，先判断是单个表达式还是多个，如果是单个，直接用`bge`等跳转即可；如果是多个，需要先用临时变量进行`sge`等运算，最后剩下单个比较再跳转。

### 生成MIPS代码

##### 编码前的mips设计

在完成代码生成一作业时，感觉直接用寄存器操作较为便捷，同时考虑到作业一的数据量较小，测试数据也较弱，所以决定直接分配寄存器进行处理。这样做的结果就是，一个简单的代码一被我实现的非常复杂。具体来说，对于每个变量，从s0-s7七个寄存器中选择一个进行分配，而对于临时变量，从t0-t9中选择一个进行分配。如果发现寄存器用完了，则从头重新开始分配。其实分配原则类似于FIFO，但由于我~~仗着代码一的数据量小~~，甚至没有记录进入和使用的顺序，而是直接覆盖。现在想想，幸好数据简单。。不然满腔热血去构造怎么都过不了可能还是会很难受。

以Exp为例，从midcode生产mips的过程为：

```c++
void expMips(MidCode *midCode) {
    if (midCode->opNum == 0) {
        if (symTable.checkExistedSymbol(midCode->result)) {
            SymbolItem *item = symTable.getExistedSymbol(midCode->result);
            string sw;
            if (item->isGlobal) {
                item->address = getRegisterT();
                sw = "\tsw " + item->address + "," + midCode->result;
            }
            if (midCode->resultType == 0) {
                pushMips("\tli " + item->address + "," + midCode->op1);
            } else {
                pushMips("\tmove " + item->address + "," + midCode->op1);
            }
            if (!sw.empty()) {
                pushMips(sw);
            }
        } else {
            if (midCode->resultType == 0) {
                pushMips("\tli " + midCode->result + "," + midCode->op1);
            } else {
                pushMips("\tmove " + midCode->result + "," + midCode->op1);
            }
        }
    } else if (midCode->opNum == 1) {
        string op = midCode->opRand;
        string result;
        string x;
        string sw;
        string ans;
        if (symTable.checkExistedSymbol(midCode->result)) {
            SymbolItem *item = symTable.getExistedSymbol(midCode->result);
            if (item->isGlobal) {
                item->address = getRegisterT();
                sw = "\tsw " + item->address + "," + midCode->result;
            }
            result = item->address;
        } else {
            result = midCode->result;
        }
        if (symTable.checkExistedSymbol(midCode->op1)) {
            SymbolItem *item = symTable.getExistedSymbol(midCode->op1);
            if (item->isGlobal) {
                item->address = getRegisterT();
                pushMips("\tlw " + item->address + "," + midCode->op1);
            }
            x = item->address;
        } else {
            x = midCode->op1;
        }
        if (op == "!") {
            ans = "\tseq " + result + ",$0," + x;
        } else if (op == "+") {
            ans = "\tadd " + result + ",$0," + x;
        } else if (op == "-") {
            ans = "\tsub " + result + ",$0," + x;
        }
        pushMips(ans);
        if (!sw.empty()) {
            pushMips(sw);
        }
    } else {
        string op = midCode->opRand;
        string ans;
        string x;
        string y;
        string result;
        string sw;
//        cout << midCode->op1 <<" "<< midCode->op2 <<endl;
        if (symTable.checkExistedSymbol(midCode->result)) {
            SymbolItem *item = symTable.getExistedSymbol(midCode->result);
            if (item->isGlobal) {
                item->address = getRegisterT();
                sw = "\tsw " + item->address + "," + midCode->result;
            }
            result = item->address;
        } else {
            result = midCode->result;
        }
        if (symTable.checkExistedSymbol(midCode->op1)) {
            SymbolItem *item = symTable.getExistedSymbol(midCode->op1);
            if (item->isGlobal) {
                item->address = getRegisterT();
                pushMips("\tlw " + item->address + "," + midCode->op1);
            }
//            cout << item->str << " " << item->address<< endl;
            x = item->address;
        } else {
            x = midCode->op1;
        }
        if (symTable.checkExistedSymbol(midCode->op2)) {
            SymbolItem *item = symTable.getExistedSymbol(midCode->op2);
            if (item->isGlobal) {
                item->address = getRegisterT();
                pushMips("\tlw " + item->address + "," + midCode->op2);
            }
            y = item->address;
        } else {
            y = midCode->op2;
        }
        if (op == "+") {
            ans = "\taddu " + result + "," + x + "," + y;
        } else if (op == "-") {
            ans = "\tsubu " + result + "," + x + "," + y;
        } else if (op == "*") {
            string tmp = "\tmult " + x + "," + y;
            pushMips(tmp);
            ans = "\tmflo " + result;
        } else {
            string tmp = "\tdiv " + x + "," + y;
            pushMips(tmp);
            if (op == "/") {
                ans = "\tmflo " + result;
            } else {
                ans = "\tmfhi " + result;
            };
        }
        pushMips(ans);
        if (!sw.empty()) {
            pushMips(sw);
        }
    }
}
```

可以看到，每次判断符号的存储位置都非常麻烦，同时非常容易因为考虑不周就出现错误。所以在代码生成一的作业中我个人debug非常痛苦，也认识到代码二作业一定需要更改架构重新写一遍。

##### 编码后的mips设计

有过代码一的惨痛教训后，我深切认识到不能全靠寄存器，因此重新进行构造——由于每个中间代码最多设计三个变量，所以理论上三个寄存器可以完成所有操作。取值，直接从栈中对应的位置取出对应的值即可；而通过三个寄存器计算出值后再压栈，即可把相应的值存入栈中。这样的操作从头到尾都只会用到三个寄存器，所以非常便捷，并且没有寄存器分配的烦恼。

具体介绍如下。同样以Exp为例，从midcode生产mips的过程为：

```c++
void expMips(MidCode *midCode) {
    if (midCode->opNum == 0) {
        //a=b
        VarControl *item = searchVar(midCode->result);
        if (midCode->resultType == 0) {
            pushMips("\tli $s0," + midCode->op1);
        } else {
            if (checkVar(midCode->op1)) {
                VarControl *var = searchVar(midCode->op1);
                getVar(*var, "$s0");
            } else {
                pushMips("\tmove $s0," + midCode->op1);
            }
        }
        if (item->isTmpVar) {
            storeVar(item, "$s0");
        } else {
            putVar(*item, "$s0");
        }
    } else if (midCode->opNum == 1) {
        //a = !b
        VarControl *item = searchVar(midCode->result);
        if (midCode->resultType == 0) {
            pushMips("\tli $s0," + midCode->op1);
        } else {
            VarControl *var = searchVar(midCode->op1);
            getVar(*var, "$s0");
        }
        string op = midCode->opRand;
        if (op == "!") {
            pushMips("\tseq $s1,$0,$s0");
        } else if (op == "+") {
            pushMips("\tadd $s1,$0,$s0");
        } else if (op == "-") {
            pushMips("\tsub $s1,$0,$s0");
        }
        if (item->isTmpVar) {
            storeVar(item, "$s1");
        } else {
            putVar(*item, "$s1");
        }
    } else {
        //a = b + c
        string op = midCode->opRand;
        VarControl *item = searchVar(midCode->result);
        if (midCode->resultType == 0) {
            //left number
            pushMips("\tli $s0," + midCode->op1);
            VarControl *right = searchVar(midCode->op2);
            getVar(*right, "$s1");
        } else if (midCode->resultType == 1) {
            //right number
            VarControl *left = searchVar(midCode->op1);
            getVar(*left, "$s0");
            pushMips("\tli $s1," + midCode->op2);
        } else {
            //both exp
            VarControl *left = searchVar(midCode->op1);
            getVar(*left, "$s0");
            VarControl *right = searchVar(midCode->op2);
            getVar(*right, "$s1");
        }
        if (op == "+") {
            pushMips("\taddu $s2,$s0,$s1");
        } else if (op == "-") {
            pushMips("\tsubu $s2,$s0,$s1");
        } else if (op == "*") {
            pushMips("\tmul $s2,$s0,$s1");
        } else {
            pushMips("\tdiv $s0,$s1");
            if (op == "/") {
                pushMips("\tmflo $s2");
            } else {
                pushMips("\tmfhi $s2");
            }
        }
        if (item->isTmpVar) {
            storeVar(item, "$s2");
        } else {
            putVar(*item, "$s2");
        }
    }
}
```

可以看出需要判断的量大大减少。

此外，对于数组，我的做法是先压栈（数组内元素个数）个单位，并在此存入数组的初地址。此后，数组内元素的赋值均按照和首地址的偏移进行赋值即可。这样做的好处是保证局部数组和全局数组的存取方式一致，可以避免函数参数传参的麻烦。

针对函数，由于我采取了所有变量用完即压栈的做法，所以理论上不需要将寄存器压栈（除了`$ra`）。

### 优化

没做。做不动了。

## 难点分析

1. 词法分析过程中对注释部分的去除：
   1. 最初选择按行扫描程序并存储，但发现需要判断的情况太复杂，遂放弃，选择直接存储输入文件的全部内容并消除注释。
   2. 按照gcc的标准，扫描到了注释后应该将注释替换为`" "`，但考虑到后续的错误处理需要输出出现错误的行号，因此考虑将注释替换为`"\r"`。
2. 语法分析中“如何将词法分析的结果作为基础”的问题。之前考虑到需要连续输出词法分析和语法分析的结果，想过在语法分析过程中调用词法分析的函数以实现同时分析。但后续遇到了无法预读的问题，遂放弃。选择将词法分析的结果存储在一个容器中，新建另一个容器同时存储词法和语法分析的结果。由于已经实现了对每个词法的分析，因此用该种方法较容易实现预读。
3. 错误处理。。坑点真的很多，可以使用大量数据进行测试。
4. 由于C++的特性，建议在递归下降传参时都利用指针而不是引用。以及着重考虑深浅拷贝的问题。
4. 符号表的架构问题。符号表的实现可以有多种方式，但一定要选择一种经过理论、实践验证的，不能凭空想象。最后自己在各个模块选择的符号表都是栈式，虽然基础，但是好用。

## 总结反思

在进行代码编写前一定要想好架构，同时考虑架构的可拓展性、可复用性。例如，在写词法分析和语法分析时我没有过多考虑错误处理需要的输出，直接利用pair结构存储元素。在错误处理时需要考虑的属性多起来后就发现这个结构不够使，进行了部分重构。

在考虑好架构后，代码的编写应该是行云流水的。如果意识到编写过程很阻塞，还是应该重新考虑是不是哪里没有设计好。此外，在写错误处理及之前的部分时重温了C++语言的诸多特性，也认识到了编译器的工作过程，可谓收获良多。

在生成中间代码和mips代码时，因为想偷懒只生成一个符号表两个部分一起用，所以自己构思了一种符号表架构。但是写着写着发现自己的符号表存在漏洞，就继续添加元素使得现在的符号表勉强能跑。但是写着写着又发现，自己想的架构终究是自己想的，经不起验证，再次出现问题，而且不是靠添加一点对象就可以解决的。所以纠结再三，还是踏上了重构的不归路。

不得不说，重构虽苦，但是重构后的代码看起来就好多了，没有曾经x山的样子。在周末两天的时间里，也一直用大量数据去检验自己的架构。不得不说编译器确实还是一个大工程，大量数据检验出了很多细节的纰漏。最后AC的时候，也真的可以说是开心的手舞足蹈了。

过程虽苦，结果是美丽的。感谢这一学期的付出，感谢助教和老师的帮助教导。
