cmake_minimum_required(VERSION 3.17)
project(project5)

set(CMAKE_CXX_STANDARD 14)

add_executable(project5 main.cpp lexical.cpp lexical.h grammar.cpp grammar.h
        error.cpp error.h symbol.cpp symbol.h ast.cpp ast.h globalVar.h
        test.cpp midCode.cpp midCode.h symbolTree.cpp symbolTree.h mips.cpp mips.h)