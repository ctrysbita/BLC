# BLC

A simple calculator constructed by Bison and Flex.

## Environments

Building BLC requires folloing components:

- Bison / Yacc: Generate syntax parser in C.
- Flex / Lex: Generate lexical analyzer in C.
- GCC / Clang / MSVC: Compiling C/C++ programs.

Since Yacc / Lex were developed by AT&T (under Plan 9 from Bell Labs) and target for Unix system, which is not fully compatible with Linux, we use Bison and Flex instead. Bison and Flex are developed by GNU so that they are fully compatible with Linux and can run in Windows easily through mingw or WSL (Windows Subsystem for Linux). And of course Bison and Flex are also compatible with original Yacc and Lex.

### Windows



### Linux

For linux distros, normally we can use package manager to install pre-compiled binaries directly. Simply run the commands below according to the distros name.

#### Ubuntu / Debian

```bash
apt install --yes flex bison
```

#### Fedora

```bash
dnf install flex-devel bison-devel
```

#### Cent OS

```bash
yum install flex bison
```

### Mac OS

No need to install la.