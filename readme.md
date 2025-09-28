# BUAA-Compiler

## Introduction
This repository contains a C++ implementation of a simplified compiler, developed as part of the BUAA compiler course.  
The project follows the classical design of a compiler, covering all major phases from lexical analysis to target code generation.  
It compiles a small C-like language into MIPS assembly, demonstrating the theory and practice of compiler construction.

Main features:
- Lexical and syntax analysis to parse input programs.
- Semantic analysis and symbol table management.
- Intermediate code generation in three-address code (TAC).
- Translation of TAC to MIPS assembly.
- Error handling and reporting across multiple phases.

For full Chinese documentation, see [README_zh.md](README_zh.md).

---

## Usage

### Build
Compile the project with any modern C++ compiler (e.g. `g++` or `clang++`):

```bash
g++ -std=c++11 -O2 -o compiler main.cpp
````

### Run

Provide a source file as input:

```bash
./compiler input.txt
```

The compiler will:

* Print tokens, parse results, or errors to the console.
* Generate intermediate representation and MIPS assembly as output files (check project configuration for exact filenames).

---

## Project Structure

```
.
├── main.cpp              # Entry point, orchestrates compilation pipeline
├── lexer/                # Lexical analysis (tokenizer)
├── parser/               # Syntax analysis (grammar rules, AST)
├── semantic/             # Semantic checks, symbol table management
├── ir/                   # Intermediate representation (TAC)
├── codegen/              # MIPS assembly code generator
├── errors/               # Error detection and reporting
├── tests/                # Example programs and test cases
├── README.md             # English documentation
└── README_zh.md          # Chinese documentation
```

---

## Compilation Pipeline

1. **Lexical Analysis**
   Input source code is scanned and divided into tokens such as keywords, identifiers, literals, and operators.

2. **Syntax Analysis**
   Tokens are parsed according to grammar rules. A parse tree or abstract syntax tree (AST) is constructed.

3. **Semantic Analysis**
   Type checking, scope resolution, and symbol table management ensure program correctness.

4. **Intermediate Code Generation**
   Valid programs are lowered into three-address code, enabling optimizations and easier translation.

5. **Target Code Generation**
   The TAC is translated into MIPS assembly instructions suitable for execution on a MIPS simulator.

6. **Error Handling**
   The compiler reports lexical, syntactic, and semantic errors with location information.

---

## Example

Source program (`input.txt`):

```c
int main() {
    int a = 5;
    int b = 10;
    int c = a + b;
    return c;
}
```

Output:

* Tokens from lexical analysis
* AST structure
* TAC (e.g., `t1 = a + b`)
* MIPS assembly file `output.asm`

---

## License

All rights reserved.