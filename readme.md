# StepC -- An LLVM frontend for C, built in stages

## Introduction
Inspired by Nora Sandler's adaptation of Abdulaziz Ghuloum's "An Incremental Approach to Compiler Construction", we implement a LLVM frontend for C, in incremental steps, using C++. 

In her project, Nora Sandler directly compiles to assembly initially, and refactors to first compile to 3AC in her [upcoming book](https://norasandler.com/2022/03/29/Write-a-C-Compiler-the-Book.html). In anticipation of StepC becoming increasingly complicated, and in order to take advantage of the optimization pipeline of LLVM, we instead compile to LLVM's IR from the very start. As our approach does not strictly follow either Sandler or Ghuloum's approaches, we document the taks accomplished at each stage at the end of the readme for easy reference.

## Requirements
The main prgram "step_c.out" makes system calls to "llc" and "gcc" to compile the produced LLVM IR into an executable. Modifying the step_c.cpp file before building can remove these system calls (and thus those dependencies). StepC is written in C++17, compiled with g++ (and sometimes clang++).

## Usage
Build main executable "step_c.out", in addition to unit tests, with
```
cmake .
cmake --build .
```
After building with CMake, the executable "step_c.out" will take in a single command line argument for a ".c" file, and compile it to an executable with no file extension and the same name. Removing the system calls to "llc" and "gcc" in the "step_c.cpp" file will make the resulting program generate a ".ll" file, so that
```
./step_c.out input_file.c
```
will generate an output in LLVM IR, as a replacement for
```
clang -S -emit-llvm input_file.c
```

In stage 3 and beyond, StepC will generate an executable, and can be tested with the programs [here](https://github.com/AMLeng/incremental_c_compiler_tests).

## Compiler Stages
* Stage 1: Set up layout of lexer, parser, and code generation from an AST. Parse (signed, 32 bit) integer literals, and return them from main. At this stage, "step_c.out" does not take arguments, and instead must be rebuilt with a suitable std::stringstream as the input.

* Stage 2: Add unary operators and some basic context/value classes to keep track of SSA registers. This is the last stage where "step_c.out" cannot take a file argument.

* Stage 3: Add binary operators, parentheses for grouping, and operator precedence (essentially using [Pratt parsing](https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html)), added code to automatically call llc and g++ to create an executable. (We don't call opt beforehand, but one could do so to create more optimized code). 

* Stage 4: New stage (not one of Nora Sandler's stages). Add in floating point constants (treating doubles and long doubles the same--higher precision long doubles are all target dependent), and started keeping track of types. Implemented most integer and floating point type conversions, including the "integer promotions" and "usual arithmetic conversions", as well as type conversions in codegen. Implemented type checking for previously implemented operations.

* Stage 5: Added local variables. Refactored error reporting to include the full line of source code containing the error. Separated out semantic analysis step. Added symbol table.

* Stage 6: Implemented compound statements, if statements, and ternary conditionals. In particular, implemented basic blocks in code generation for flor control. Equivalent to Nora Sandler's stages 6 and 7.
