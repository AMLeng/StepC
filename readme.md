# Incremental C -- An LLVM frontend for C, built in stages

## Introduction
Inspired by Nora Sandler's adaptation of Abdulaziz Ghuloum's "An Incremental Approach to Compiler Construction", we implement a LLVM frontend for C, in incremental steps, using C++. 

In her project, Nora Sandler directly compiles to assembly initially, and refactors to first compile to 3AC in her [upcoming book](https://norasandler.com/2022/03/29/Write-a-C-Compiler-the-Book.html). In anticipation of IncrementalC becoming increasingly complicated, and in order to take advantage of the optimization pipeline of LLVM, we instead compile to LLVM's IR from the very start. As our approach does not strictly follow either Sandler or Ghuloum's approaches, we document the taks accomplished at each stage at the end of the readme for easy reference.

## Requirements
The main prgram "incremental_c.out" makes system calls to "llc" and "gcc" to compile the produced LLVM IR into an executable. Modifying the incremental_c.cpp file before building can remove these system calls (and thus those dependencies). IncrementalC is written in C++17.

## Usage
After building with CMake, the executable "incremental_c.out" will take in a single command line argument for a ".c" file, and compile it to an executable with no file extension and the same name. Removing the system calls to "llc" and "gcc" in the "incremental_c.cpp" file will make the resulting program generate a ".ll" file, so that
```
./incremental_c.out input_file.c
```
will generate an output in LLVM IR, as a replacement for
```
clang -S -emit-llvm input_file.c
```

In stage 3 and beyond, IncrementalC can be essentially tested with Nora Sandler's test suite/script (forked to [here](https://github.com/AMLeng/incremental_c_compiler_tests)).

## Compiler Stages
* Stage 1: Set up layout of lexer, parser, and code generation from an AST. Parse (signed, 32 bit) integer literals, and return them from main. At this stage, "incremental_c.out" does not take arguments, and instead must be rebuilt with a suitable std::stringstream as the input.

* Stage 2: Add unary operators and some basic context/value classes to keep track of SSA registers. This is the last stage where "incremental_c.out" cannot take a file argument.

* Stage 3: Add binary operators, parentheses for grouping, and operator precedence (essentially using [Pratt parsing](https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html)), added code to automatically call llc and g++ to create an executable. (We don't call opt beforehand, but one could do so to create more optimized code). 
