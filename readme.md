# StepC -- An LLVM frontend for C, built in stages

## Introduction
Inspired by Nora Sandler's adaptation of Abdulaziz Ghuloum's "An Incremental Approach to Compiler Construction", I implemented the majority of a LLVM frontend for C, in incremental steps, using C++. 

In her project, Nora Sandler directly compiles to assembly initially, and refactors to first compile to 3AC in her [upcoming book](https://norasandler.com/2022/03/29/Write-a-C-Compiler-the-Book.html). In anticipation of StepC becoming increasingly complicated, and in order to take advantage of the optimization pipeline of LLVM, I instead compile to LLVM's IR from the very start. As my approach does not strictly follow either Sandler or Ghuloum's approaches, I document the taks accomplished at each stage at the end of the readme for easy reference (e.g. for anyone else who decides to do a similar project).

This was a personal project I worked on over the span of four months, writing code on 64 days during that time. It originally had 4 goals, all of which have been accomplished:

* Learn about the C standard
* Practice writing a large project in modern C++
* Learn about compilers (I never took a compilers course in college)
* Get my first software job (I am almost completely self taught and had a lot of difficulty getting first round interviews)

Currently, this project passes 79/220 test cases in the [c-testsuite](https://github.com/c-testsuite/c-testsuite) (with a mix of unimplemented features and some bugs with existing features -- 76 of the failures are from unimplemented preproecssor directives).

## Requirements
The main prgram "step_c.out" makes a system call to clang to compile the produced LLVM IR into an executable. Modifying the step_c.cpp file before building can remove that system call (and thus those dependencies). StepC is written in C++17, compiled with g++ (and sometimes clang++).

## Usage
Build main executable "step_c.out", in addition to unit tests, with
```
cmake .
cmake --build .
```
After building with CMake, the executable "step_c.out" will take in a single command line argument for a ".c" file, and compile it to an executable with no file extension and the same name. Removing the system calls to "llc" and "gcc" (or to "clang", in later versions---llc ran into a bug with switch statements with too many options) in the "step_c.cpp" file will make the resulting program generate a ".ll" file, so that
```
./step_c.out input_file.c
```
will generate an output in LLVM IR, as a replacement for
```
clang -S -emit-llvm input_file.c
```

In stage 3 and beyond, StepC will generate an executable (by adding a system call to e.g. clang after generating the .ll LLVM IR file), and can be tested with the programs [here](https://github.com/AMLeng/incremental_c_compiler_tests).

## Compiler Stages
I roughly tried to follow Nora Sandler's ordering of stages since it seemed fairly reasonable, except that I moved implementation of the type system earlier in the process. In retrospect, the type system was by far one of the most taxing parts of the project (and it isn't even fully completed!); if you were to try to implement a compiler like this one as an exercise, I would highly recommend adding more stages to build out the type system a bit more incrementally---my stage 4 is definitely oversied, and took multiple weeks of work to implement.

* Stage 1: Set up layout of lexer, parser, and code generation from an AST. Parse (signed, 32 bit) integer literals, and return them from main. At this stage, "step_c.out" does not take arguments, and instead must be rebuilt with a suitable std::stringstream as the input.

* Stage 2: Add unary operators and some basic context/value classes to keep track of SSA registers. This is the last stage where "step_c.out" cannot take a file argument.

* Stage 3: Add binary operators, parentheses for grouping, and operator precedence (essentially using [Pratt parsing](https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html)), added code to automatically call llc and g++ to create an executable. (I don't call opt beforehand, but one could do so to create more optimized code). 

* Stage 4: New stage (not one of Nora Sandler's stages). Add in floating point constants (treating doubles and long doubles the same--higher precision long doubles are all target dependent), and started keeping track of types. Implemented most integer and floating point type conversions, including the "integer promotions" and "usual arithmetic conversions", as well as type conversions in codegen. Implemented type checking for previously implemented operations.

* Stage 5: Added local variables. Refactored error reporting to include the full line of source code containing the error. Separated out semantic analysis step. Added symbol table.

* Stage 6: Implemented compound statements, if statements, and ternary conditionals. In particular, implemented basic blocks in code generation for flor control. Equivalent to Nora Sandler's stages 6 and 7.

* Stage 7: Implemented remaining binary operators (including compound assignments), as well as prefix/postfix increment and decrement, as well as multiple comma separated variable declarations. Superset of Nora Sandler's stage 4.

* Stage 8: Implemented loops, gotos, switch statements, and labeled statements. Changed system calls from llc and gcc to a single call to clang (since llc seemed to have a bug when called on a switch statement with 4 or more non-default cases). Superset of Nora Sandler's stage 8.

* Stage 9: Implemented function calls, function definitions/declarations, and global variables. Upgraded type system to include derived and void types. Essentially Nora Sandler's stages 9 and 10

* Stage 10: Implemented pointers, and refactored function handling to correctly convert function designators to pointers in most circumstances.

* Stage 11: Implemented arrays and strings substantially fixed codegeneration for pointers, implemented constant expressions, sizeof. 

* Stage 12: Implemented structs and unions, typedefs, enums.

* Stage 13: Implemented preprocessor (unfinished).

* Stage 14: Type qualifiers and other features (unfinished).
