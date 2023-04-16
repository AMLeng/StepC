Following Nora Sandler's adaptation of Abdulaziz Ghuloum's "An Incremental Approach to Compiler Construction", we implement a LLVM frontend for C, in incremental steps, using C++. Note that Nora Sandler directly compiles to assembly, while we (in anticipation of the project becoming increasingly complicated, later on in the process, and in order to take advantage of the optimization pipeline of LLVM to generate code that has remotely reasonable runtimes), compile to LLVM's IR from the very start.

Stage 1: Set up layout of lexer, parser, and code generation from an AST. Parse (signed, 32 bit) integer literals, and return them from main.

Stage 2: Add unary operators and some basic context/value classes to keep track of SSA registers.
