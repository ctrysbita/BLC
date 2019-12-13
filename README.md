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

It overrides the two tree-traverser methods and has a new `Execute` method. `Execute` acts as interpreter, which is the third tree traverser. It takes the current context, which will be explained later, and perform certain instructions depending on the statement type. 

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

Like `StatementAST`, it overrides the `JsonTree` and `GenIR` methods, but defines them as pure virtual methods. Since the expression must have a type, `ExpressionAST` is defined as an abstract class and is not instantiable. However, the `StatementAST` can be instantiated, because the program allows empty statement inputs. The major difference between expression nodes and statement nodes is that expression nodes can be evaluated and a double type result will be returned, while statement nodes can be executed but no value will be returned. 

Expression nodes can have the type of  `DoubleAST`, `BinaryOperationAST`, `IdentifierAST`, `VariableAssignmentAST` or `ExpressionAssignmentAST`. The following of this report will explain all these classes in order. 

`DoubleAST` is the node type for a single double number. Since all the numbers in the program are of type double, all number nodes are represented by the `DoubleAST`. Its body is as follows.

```c++
class DoubleAST : public ExpressionAST {
 private:
  double value_;

 public:
  DoubleAST(std::string* value) : value_(std::stod(*value)) {}
  virtual ~DoubleAST() {}

  virtual double Evaluate(Context* context) override;
  virtual nlohmann::json JsonTree() override;
};
```

It has a constructor which takes a string as parameter and translates it into a double value. The double value is stored as a private member `value_`. `DoubleAST` overrides the method evaluate, which returns its member `value_` directly as a result. This class also overrides the `JsonTree` method, which returns its tree structure as a `nlohmann::json` object. 

`BinaryOperationAST` is the node type for binary operations, for instance, plus minus, multiplication and division between two numbers. The body of this class is as follows. 

```c++
class BinaryOperationAST : public ExpressionAST {
 private:
  int type_;
  ExpressionAST* lhs_;
  ExpressionAST* rhs_;

 public:
  BinaryOperationAST(int type, ExpressionAST* lhs, ExpressionAST* rhs)
      : type_(type), lhs_(lhs), rhs_(rhs) {}
  virtual ~BinaryOperationAST() {
    delete lhs_;
    delete rhs_;
  }

  virtual double Evaluate(Context* context) override;
  virtual nlohmann::json JsonTree() override;
};
```

The private member `type_` is for identifying the type of binary operation the node is representing. The two operands for a binary operation are seen as left and right children for the node. According to the grammar definition, they both should be expressions. Therefore, two private members `lhs_` and `rhs_` are used to access the two operands; they are two pointers of type `ExpressionAST`, which point to the left and right child of this node respectively. The constructor of this class initializes the members, while the destructor deletes both children in case of memory overflow. Like the previous class, the `evaluate` method evaluates the binary operation with both operands and returns the result. The `JsonTree` returns the representation of the node's tree structure. 

`IdentifierAST` is the node type for variable identifiers. In the program, users can use variables with names of unlimited lengh, and the identifier is represented using this class. The body of this class is as follows. 

```c++
class IdentifierAST : public ExpressionAST {
 private:
  std::string name_;

 public:
  IdentifierAST(std::string* name) : name_(*name) { delete name; }
  virtual ~IdentifierAST() {}

  inline const std::string& get_name() { return name_; }

  virtual double Evaluate(Context* context) override;
  virtual nlohmann::json JsonTree() override;
};
```

The private member `name_` is used to store the name of the identifier. There is also a `get_name` method to retrieve the name of this identifier. Like previous classes, the class overrides  the`Evaluate` method, which returns the value of this identifier. It also overrides the `JsonTree` method that returns the tree structure for this node. 

`VariableAssignmentAST` is the node type for variable assignments. In the program, variable assignment is the operation that assigns an expression to a variable. The body of this class is as follows. 

```c++
class VariableAssignmentAST : public ExpressionAST {
 private:
  IdentifierAST* name_;
  ExpressionAST* value_;

 public:
  VariableAssignmentAST(IdentifierAST* name, ExpressionAST* value)
      : name_(name), value_(value) {}
  virtual ~VariableAssignmentAST() {
    delete name_;
    delete value_;
  }

  virtual double Evaluate(Context* context) override;
  virtual nlohmann::json JsonTree() override;
};
```

The private member `name_` is the pointer to the identifier object of the target variable, and `value_` is the pointer to the node of the source expression. Other methods have the same meaning as those in previous class. 

`ExpressionAssignmentAST` is the node type for expression assignments. Expression assignment is logically similar to variable assignment, but with different behaviors. The body of this class is as follows. 

```c++
class ExpressionAssignmentAST : public ExpressionAST {
 private:
  IdentifierAST* name_;
  ExpressionAST* value_;

 public:
  ExpressionAssignmentAST(IdentifierAST* name, ExpressionAST* value)
      : name_(name), value_(value) {}
  virtual ~ExpressionAssignmentAST() {
    delete name_;
  }

  virtual double Evaluate(Context* context) override;
  virtual nlohmann::json JsonTree() override;
};
```

As seen from the code, the `ExpressionAssignmentAST` is very similar to `VariableAssignmentAST`. However, in method `Evaluate` it assigns the source expression to the destination identifier while in the same method of `VariableAssignmentAST` the resulting value of the source expression is assigned to the destination identifier. For this reason, in the destructor it does not delete the pointer of `ExpressionAST`. After the expression assignment, the destination identifier refers to an expression, which will be evaluated every time the program uses the identifier. 

Similar to expression nodes, statement nodes can also be of different types, which are `BlockAST`, `IFAST` and `WhileAST`. 

`BlockAST` is the node type for code blocks, which are statements wrapped with curly brackets. The body of this class is as follows. 

```c++
class BlockAST : public StatementAST {
 public:
  typedef std::variant<double, ExpressionAST*> SymbolType;

 private:
  std::map<std::string, SymbolType> symbols_;
  std::map<std::string, llvm::Value*> llvm_symbols_;
  std::list<AST*> children_;

 public:
  BlockAST() {}
  virtual ~BlockAST() {}
    
  inline std::optional<SymbolType> get_symbol(const std::string& name);
  inline void set_symbol(const std::string& name, SymbolType&& value);

  inline llvm::Value* get_llvm_symbol(const std::string& name);
  inline void set_llvm_symbol(const std::string& name, llvm::Value* value);
    
  BlockAST* WithChildren(std::list<AST*>* asts);

  virtual void Execute(Context* context) override;
  virtual nlohmann::json JsonTree() override;
  virtual llvm::Value* GenIR(Context* context) override;
};
```

This class defined a public type named `SymbolType`, which is of type `std::variant`. This is a new class in C++17 standard, which represents a type-safe C-like union. `SymbolType` is the type of values of variables in the program, which can either be a double or a pointer of an expression object. The class also maintains two symbol tables to map the names of variables to their values. One of the table is a map from name string to `SymbolType`, which is used by the interpreter and JSON tree generator. The other table is a map from name string to a pointer to  `llvm::Value` objects; this table is used by the LLVM intermediate representation generator. The private `std::list` `children_` stores all the children of the block. Since the children of block can be statements and expressions, the list is of type pointer to `AST`. 

The functions `get_symbol`, `set_symbol` and `get_llvm_symbol`, `set_llvm_symbol` act as getter and setter for `symbols_` and `llvm_symbols_` respectively. Note that the return value of `get_symbol` is of type `std::optional`, because the name may not be found in the symbol table, which means the variable is used before definition and an empty value will be returned. Also, one of the parameters of `set_symbol` is `SymbolType&& value`, which is called a R-value reference; this allows the program to put the parameter object directly to the map without copying, which can improve operation efficiency. 

The function `WithChildren` is used to set the children list of the block object, it is called once the subtrees of this node are constructed. `Execute` method is used to execute the operations of the children of the block object, which acts as interpreter. The other two overridden methods have similar functionalities as the previous classes. 

`WhileAST` is the node type for while statements. The body of the class is as follows. 

```c++
class WhileAST : public StatementAST {
 private:
  ExpressionAST* condition_;
  StatementAST* statement_;

 public:
  WhileAST(ExpressionAST* condition, StatementAST* statement)
      : condition_(condition), statement_(statement) {}
  virtual ~WhileAST() {
    delete condition_;
    delete statement_;
  }

  virtual void Execute(Context* context) override {
    while (condition_->Evaluate(context)) statement_->Execute(context);
  }
  virtual nlohmann::json JsonTree() override;
};
```

