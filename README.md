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

### macOS

macOS with Xcode installed comes with flex and bison by default. However, their versions are too outdated that the code generated cannot satisfy the ISO C++ 17 standard. Therefore, we need to install newer versions of flex and bison. 

Firstly, if the machine doesn’t have homebrew installed, we need to install it with a script from the internet. Open a terminal and enter the following command, 

```bash
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```

After the command is finished, the machine should have homebrew installed.  

Then, we need to use homebrew to install newer version of flex and bison. Enter the two commands, 

```bash
brew install flex 
brew install bison 
```

Bison and flex are now installed to `/usr/local/Cellar/bison/3.4.2/bin` and `/usr/local/Cellar/flex/2.6.4_1/bin` respectively (the paths may vary depending on the installed version). 

You can add the paths to PATH environment variable for convenience, 

```bash
export PATH=/usr/local/Cellar/bison/3.4.2/bin:/usr/local/Cellar/flex/2.6.4_1/bin:$PATH
```

Note that the new added paths should come before the old PATH variable, so that the shell will find the newer version instead of the old one.  

At last, LLVM library needs to be installed by homebrew. Enter the command, 

```bash
brew install llvm 
```

Since the installation process involves the compilation of LLVM source code, it may take a long time. After installation is finished, LLVM is now installed at /usr/local/Cellar/llvm. Since llvm-config utility is needed when compiling our program, you may want to add `/usr/local/Cellar/llvm/9.0.0_1/bin` to the PATH variable as well (the path may vary). 


## Code Explanation

### Abstract syntax tree

To represent the syntax tree,  a set of classes are created. The first class created is `AST`, the body of this class is as follows, 

```c++
class AST {
 public:
  AST() {}
  virtual ~AST() {}
  virtual nlohmann::json JsonTree() = 0;
  virtual llvm::Value* GenIR(Context* context) = 0;
};
```

This class is the parent of all kinds of tree nodes used in our program. It is an abstract class with two pure virtual methods, which are `JsonTree` and `GenIR` respectively. These two methods are two tree traverses, namely the syntax tree printer and compiler. JsonTree is used to generate Json output for the constructed syntax tree, and GenIR is used to generate LLVM Intermediate Representation for the constructed syntax tree. 

There are two types of tree nodes in the program, which are called statement nodes and expression nodes. Statement nodes are inherited from `StatementAST`, whose body is as follows. 

```c++
class StatementAST : public AST {
 public:
  StatementAST() {}
  virtual ~StatementAST() {}
  virtual void Execute(Context* context) {}
  virtual nlohmann::json JsonTree() override;
  virtual llvm::Value* GenIR(Context* context) override;
};
```

It overrides the two tree-traverser methods and has a new `Execute` method. Execute acts as interpreter, which is the third tree traverser. It takes the current context, which will be explained later, and perform certain instructions depending on the statement type. 

The other type of tree nodes is expression node. Expression nodes are inherited from `ExpressionAST`, whose body is as follows. 

```c++
class ExpressionAST : public AST {
 public:
  ExpressionAST() {}
  virtual ~ExpressionAST() {}
  virtual double Evaluate(Context* context) = 0;
  virtual nlohmann::json JsonTree() = 0;
  virtual llvm::Value* GenIR(Context* context) = 0;
};
```

Like `StatementAST`, it overrides the `JsonTree` and `GenIR` methods, but defines them as pure virtual methods. Since the expression must have a type, `ExpressionAST` is defined as an abstract class and is not instantiable. However, the `StatementAST` can be instantiated, becouse the program allows empty statement inputs. The major difference between expression nodes and statement nodes is that expression nodes can be evaluatd and a double type result will be returned, while statement nodes can be executed but no value will be returned. 

Expression nodes can have the type of  `DoubleAST`, `BinaryOperationAST`, `IdentifierAST`, `VariableAssignmentAST` or `ExpressionAssignmentAST`. This report will explain all these classes in order. 

