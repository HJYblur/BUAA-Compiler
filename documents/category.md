```c++
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

category.insert(sort(38, "<CompUnit>"));
category.insert(sort(39, "<ConstDecl>"));
category.insert(sort(40, "<ConstDef>"));
category.insert(sort(41, "<ConstInitVal>"));
category.insert(sort(42, "<VarDecl>"));
category.insert(sort(43, "<VarDef>"));
category.insert(sort(44, "<InitVal>"));
category.insert(sort(45, "<FuncDef>"));
category.insert(sort(46, "<MainFuncDef>"));
category.insert(sort(47, "<FuncType>"));
category.insert(sort(48, "<FuncFParams>"));
category.insert(sort(49, "<FuncFParam>"));
category.insert(sort(50, "<Block>"));
category.insert(sort(51, "<Stmt>"));
category.insert(sort(52, "<Exp>"));
category.insert(sort(53, "<Cond>"));
category.insert(sort(54, "<LVal>"));
category.insert(sort(55, "<PrimaryExp>"));
category.insert(sort(56, "<Number>"));
category.insert(sort(57, "<UnaryExp>"));
category.insert(sort(58, "<UnaryOp>"));
category.insert(sort(59, "<FuncRParams>"));
category.insert(sort(60, "<MulExp>"));
category.insert(sort(61, "<AddExp>"));
category.insert(sort(62, "<RelExp>"));
category.insert(sort(63, "<EqExp>"));
category.insert(sort(64, "<LAndExp>"));
category.insert(sort(65, "<LOrExp>"));
category.insert(sort(66, "<ConstExp>"));
```

##### 2.2 难点分析

（1）**变量定义与有返回值函数定义的冲突问题**；`int a;`，`int a,b;`，`int a[10];`与`int a();`

即处理到标识符之后，需要向后**预读**一个单词，判断如果是左括号，则认为是函数定义，这时应该退回到`int`之前，转入对函数定义的分析。

解决方法：设计了一个函数`void retractString(int oldIndex);`  用于**回退**到某一个位置。

```
void retractString(int old) {
	for (int i = old; i < indexs; i++) {
		if (filecontent[i] == '\n') {
			line--;
		}
	}
	indexs = old;
}
```

类似的情况：无返回值函数与主函数`void f();`与`void main();`；因子中标识符、标识符[]、有返回值函数调用语句三者`a`、`a[]`、`a()`；语句中函数调用语句与赋值语句`a();`与`a=10,a[1]=10;`我都采用了预读，然后判断属于哪一种，然后回退，然后进入到对应的递归子程序。

（2）**错误处理的行号问题**

当读到一个符号，不符合当前语法分析结果时，应该把当前符号退掉，输出错误信息，然后把`symbol`改成正确的以正确执行后边的分析。在词法中设置一个`oldindex`，表示每次`getsym()`之前的`index`，也就是读入这个符号之前的位置，然后发生错误回退到`oldindex`，这样输出的行号就跟要求是一致的了。例如：

```
if (symbol != SEMICN) {
	retractString(oldIndex);
	errorfile << line << " k\n";  //缺少分号
	symbol = SEMICN;  //修正symbol
}
```