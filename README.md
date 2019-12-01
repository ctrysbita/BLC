# BLC

A simple calculator constructed by Bison and Flex.

## Environments

Building BLC requires folloing components:

- Bison / Yacc: Generate syntax parser in C.
- Flex / Lex: Generate lexical analyzer in C.
- GCC / Clang / MSVC: Compiling C/C++ programs.

Since Yacc / Lex were developed by AT&T (under Plan 9 from Bell Labs) and target for Unix system, which is not fully compatible with Linux, we use Bison and Flex instead. Bison and Flex are developed by GNU so that they are fully compatible with Linux and can run in Windows easily through mingw or Windows Subsystem for Linux (WSL). And of course Bison and Flex are also compatible with original Yacc and Lex.

### Windows

In windows paltform, there are two ways to run Linux programs.

One is using Windows Subsystem for Linux (WSL) which launched by Mircosoft to support running a Linux virtual environment under Windows. In this way, we can follow the [guidance of installing Bison and Flex in Linux](#Linux) directly.

The other way is using mingw / cygwin, which provide a POSIX interface in Windows. For an example, by using [msys2](https://www.msys2.org/) environment, we can use pacman (a package manager) to install them just like in Linux.

```bash
pacman -S flex bison mingw-w64-x86_64-gcc
```

Additionally, we can download pre-compiled binaries of [Win-Flex-Bison](https://sourceforge.net/projects/winflexbison/).

### Linux

For linux distros, normally we can use package manager to install pre-compiled binaries directly. Simply run the commands below according to the distros name.

#### Ubuntu / Debian

```bash
apt install --yes flex bison gcc
```

#### Fedora

```bash
dnf install flex-devel bison-devel gcc
```

#### Cent OS

```bash
yum install flex bison gcc
```

### Mac OS

No need to install la.


###