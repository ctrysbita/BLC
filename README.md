# BLC

A basic calculator constructed by Bison, Flex and LLVM. ([Wiki](https://github.com/ctrysbita/BLC/wiki))

## Environments

Building BLC requires folloing components:

- Bison / Yacc: Generate syntax parser in C.
- Flex / Lex: Generate lexical analyzer in C.
- GCC / Clang / MSVC: Compiling C/C++ programs. (C++17 support required)
- LLVM: IR generation and executable compilation. (LLVM 9.0+ required)

## Compilation Guide

Go to the `src` directory, make it~

```bash
cd src
make all
```

After the command is finished, a file named `main` will be generated, which is the executable after compilation.

```bash
./main
[IN]<-
```