# BLC

A simple calculator constructed by Bison and Flex.

## Abstract

This report is mainly talking about how a simple calculator, which is built by using flex and bison, works and user instructions to use its functions in details. The explanation of the code, in order to tell the how the calcultor is designed, is included in this report as well.

## Environments

Building BLC requires folloing components:

- Bison / Yacc: Generate syntax parser in C.
- Flex / Lex: Generate lexical analyzer in C.
- GCC / Clang / MSVC: Compiling C/C++ programs. (C++17 support required)
- LLVM: IR generation and executable compilation. (LLVM 9.0+ required)

Since Yacc / Lex were developed by AT&T (under Plan 9 from Bell Labs) and target for Unix system, which is not fully compatible with Linux, we use Bison and Flex instead. Bison and Flex are developed by GNU so that they are fully compatible with Linux and can run in Windows easily through mingw or Windows Subsystem for Linux (WSL). And of course Bison and Flex are also compatible with original Yacc and Lex.

### Windows

In windows paltform, there are two ways to run Linux programs.

One is using Windows Subsystem for Linux (WSL) which launched by Mircosoft to support running a Linux virtual environment under Windows. In this way, we can follow the [guidance of installation under Linux](#Linux) directly.

The other way is using mingw / cygwin, which provide a POSIX interface in Windows. For an example, by using [msys2](https://www.msys2.org/) environment, we can use pacman (a package manager) to install them just like in Linux.

```bash
pacman -S flex bison mingw-w64-x86_64-gcc mingw-w64-x86_64-llvm
```

Additionally, we can download pre-compiled binaries of [Win-Flex-Bison](https://sourceforge.net/projects/winflexbison/) and install directly.

### Linux

For linux distros, normally we can use package manager to install pre-compiled binaries directly. Simply run the commands below according to the distros name.

#### Ubuntu / Debian

```bash
apt install --yes flex bison gcc
# Use official script to install LLVM 9.
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 9
```

#### Fedora

```bash
dnf install flex-devel bison-devel gcc llvm
```

#### Cent OS

```bash
yum install yum-utils
yum-config-manager --enable extras
yum install flex bison gcc llvm
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

Also, the project needs the LLVM library, which can be installed using homebrew as well. Enter the command,

```bash
brew install llvm
```

Since the installation process involves the compilation of LLVM source code, it may take a long time. After installation is finished, LLVM is now installed at /usr/local/Cellar/llvm. Since llvm-config utility is needed when compiling our program, you may want to add `/usr/local/Cellar/llvm/9.0.0_1/bin` to the PATH variable as well (the path may vary).

At last, users need to install `make` to compile the code of the project.

```bash
brew install make
```


## Code Explanation

### Abstract Syntax Tree

To represent the syntax tree,  a set of classes are created. The first class created is `AST`, the body of this class is as follows,

```c++
class AST {
 public:
  AST() {}
  virtual ~AST() {}
  virtual nlohmann::json JsonTree() = 0;
  virtual void Run(Context* context) = 0;
  virtual llvm::Value* GenIR(Context* context) = 0;
};
```

This class is the parent of all kinds of tree nodes used in our program. It is an abstract class with three pure virtual methods, which are `JsonTree`, `Run` and `GenIR`. These three methods are three tree traverses, namely the syntax tree printer, interpreter and compiler. `JsonTree` is used to generate Json output for the constructed syntax tree, `Run` is used to interpret the user input and `GenIR` is used to generate LLVM Intermediate Representation for the constructed syntax tree.

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
  virtual ~BinaryOperationAST();
  virtual double Evaluate(Context* context) override;
  virtual nlohmann::json JsonTree() override;
};
```

The private member `type_` is for identifying the type of binary operation the node is representing. The two operands for a binary operation are seen as left and right children for the node. According to the grammar definition, they both should be expressions. Therefore, two private members `lhs_` and `rhs_` are used to access the two operands; they are two pointers of type `ExpressionAST`, which point to the left and right child of this node respectively. Like the previous class, the `evaluate` method evaluates the binary operation with both operands and returns the result. The `JsonTree` returns the representation of the node's tree structure.

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
  virtual ~VariableAssignmentAST();
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
  virtual ~ExpressionAssignmentAST();
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

The function `WithChildren` is used to set the children list of the block object, it is called once the subtrees of this node are constructed. `Execute` method executes the block object in a certain way, which acts as interpreter. The other two overridden methods have similar functionalities as the previous classes.

`WhileAST` is the node type for while statements. The body of the class is as follows.

```c++
class WhileAST : public StatementAST {
 private:
  ExpressionAST* condition_;
  StatementAST* statement_;

 public:
  WhileAST(ExpressionAST* condition, StatementAST* statement)
      : condition_(condition), statement_(statement) {}
  virtual ~WhileAST();

  virtual void Execute(Context* context) override {
    while (condition_->Evaluate(context)) statement_->Execute(context);
  }
  virtual nlohmann::json JsonTree() override;
};
```

This class has two private members, `condition_` and `statement_`. `condition_` points to an `ExpressionAST` which is used as the condition of while loop. `statement_` points to a `StatementAST` which contains the instructions the loop body does. The class also contains three overridden methods with similar functionalities as previous classes.

`IFAST` is the node type for if statements. The body of the class is as follows.

```c++
class IfAST : public StatementAST {
 private:
  ExpressionAST* condition_;
  AST* then_;
  AST* else_;

 public:
  IfAST(ExpressionAST* condition, AST* then_statement,
        AST* else_statement = nullptr)
      : condition_(condition), then_(then_statement), else_(else_statement) {}
  virtual ~IfAST();
  virtual void Execute(Context* context) override;
  virtual nlohmann::json JsonTree() override;
  virtual llvm::Value* GenIR(Context* context) override;
};
```

The member `condition_` stores a pointer to `ExpressionAST`, whose result of evaluation will be used as the condition of the if statement. The members `then_` and `else_` are two pointers to `AST`, representing the instructions needed to be run when the condition passes and fails respectively.


### Syntax Analyzer

```c++
%{
#include <string>
#include <list>
#include "ast.h"

extern int yylex();
extern void yyerror(std::string);
extern void OnParsed();

extern AST* ast;
%}

```

Firstly, the code includes the head file `ast.h` , and defines the extern functions `yylex()` , `yyerror()` , `OnParsed()`  and the extern pointer `ast`  at the beginning.

```c++
%union {
  std::string* value;

  ExpressionAST* expression;
  StatementAST* statement;
  IdentifierAST* identifier;

  std::list<AST*>* statements;
}

%token <value> IDENTIFIER DOUBLE_NUM
%token EXPR IF WHILE
%right '='
%left GEQ LEQ EQ NE
%left '+' '-'
%left '*' '/' '%'

%type <statement> statement
%type <statements> statements
%type <identifier> identifier
%type <expression> expression

%start program

%%
```

Secondly, the types that might be used are defined in `union` . Following the union, `IDENTIFIER` , `DOUBLE_NUM` , `EXPR` , `IF` , and `WHILE`  are defined as token, meanwhile, `IDENTIFIER`  and `DOUBLE_NUM` are defined to have type of `value` . The associativity of the operands are also defined as followed. Lastly, the types of the non-terminals are defined separately to be in different types.

```c++
program:
program statement { ast = $2; OnParsed(); }
|
;

statement:
';' { $$ = new StatementAST(); }
| expression ';' { $<expression>$ = $1; }
| WHILE '(' expression ')' statement { $$ = new WhileAST($3, $5); }
| IF '(' expression ')' statement { $$ = new IfAST($3, $5); }
| '{' statements '}' { $$ = (new BlockAST())->WithChildren($2); }
;

statements:
statement { $$ = new std::list<AST*>(); $$->push_back($1); }
| statements statement { $1->push_back($2); }
;
```

The parsing starts with `program`  and  `statement` first define that any `expression`  in this grammar should be end with the semicolon. And then the productions define the grammar of the IF statement, While statement and the block statement. Meanwhile, the productions of `statements`  define that `statements`  are either consists of a set of `statement`  or a single `statement` .

```c++
expression:
DOUBLE_NUM { $$ = new DoubleAST($1); }
| identifier { $$ = $1; }
| identifier '=' expression { $$ = new VariableAssignmentAST($1, $3); }
| EXPR identifier '=' expression { $$ = new ExpressionAssignmentAST($2, $4); }
| '-' expression %prec ';' { $$ = $2; }
| expression '+' expression { $$ = new BinaryOperationAST('+', $1, $3); }
| expression '-' expression { $$ = new BinaryOperationAST('-', $1, $3); }
| expression '*' expression { $$ = new BinaryOperationAST('*', $1, $3); }
| expression '/' expression { $$ = new BinaryOperationAST('/', $1, $3); }
| expression '<' expression { $$ = new BinaryOperationAST('<', $1, $3); }
| expression '>' expression { $$ = new BinaryOperationAST('>', $1, $3); }
| expression GEQ expression { $$ = new BinaryOperationAST(GEQ, $1, $3); }
| expression LEQ expression { $$ = new BinaryOperationAST(LEQ, $1, $3); }
| expression NE expression { $$ = new BinaryOperationAST(NE, $1, $3); }
| expression EQ expression { $$ = new BinaryOperationAST(EQ, $1, $3); }
| '(' expression ')' { $$ = $2; }
;
```

The productions of `expression` define that `expression`  can be a single double-type number or a single identifier. Meanwhile, it could also state the value assignment, arithmetic operations, including negation, addition, subtraction, multiplication and division, and comparison between two double-type numbers.

```c++
identifier:
IDENTIFIER { $$ = new IdentifierAST($1); }
;

%%
```

The production of `identifier`  states to create a new `identifier`  .

```c++
void yyerror(std::string s) {
  fprintf(stdout, "%s\n", s.c_str());
}
```

The `yyerror()` function states to detect and report the error that might appear in the program.

### Lexer

The blc.l file is regarded as lexer in the process doing lexical analysis job.

```c++
%{
#include <string>
#include <vector>
#include "ast.h"
#include "blc.tab.hpp"

extern void OnEnd();

void yyerror(std::string);
%}
```

This part starting from `%{` and ended up with `%}` is a block whose contents will be put into the output `.c` file automatically.

```C++
%%

"expr" return EXPR;
"if" return IF;
"else" return ELSE;
"while" return WHILE;

[0-9]*\.[0-9]+|[0-9]+ { yylval.value = new std::string(yytext); return DOUBLE_NUM; }
[_a-zA-Z][_a-zA-Z0-9]* { yylval.value = new std::string(yytext); return IDENTIFIER; }

[-(){}<>=+*/%;] return *yytext;

">=" return GEQ;
"<=" return LEQ;
"==" return EQ;
"!=" return NE;

[ \t\n]+ ;
. yyerror("Lexical Error: Unknown character.");

%%
```

In this part, two `%%`s mean that contents inside them are rules of how lexer would translate input codes to tokens.

This lexical analyzer would give `EXPR`, `IF`, `ELSE`, `WHILE`, `GEQ`, `LEQ`, `EQ` and `NE` token names to syntax analyzer if it detects "expr", “if”, “else”, “while”, “>=”, “<=”, “==” and “!=” respectively.

Besides, there are rules about regular expression. When the input string can be expressed by `[0-9]*\.[0-9]+|[0-9]+`, its token name would be `DOUBLE_NUM`. Similarly, expression `[_a-zA-Z][_a-zA-Z0-9]*` would have `IDENTIFIER` as its token name. Both of expressions will change their contents to string type and then store them as their token value.

Additionally, expression `[-(){}<>=+*/%;]` would directly return contents itself as token. When detects `[ \t\n]+` , it would do nothing. When detects all the other inputs, it would report error information.

```c++
int yywrap() {
  OnEnd();
  return 1;
}
```

When input is exhausted, the lexer will call function yywrap and return 1.

### Abstract Syntax Tree Traverser

After an AST is constructed, by traversing through AST, three kind of outputs can be generated.

- Evaluated result.
- JSON format abstract syntax tree.
- LLVM intermediate representation.

While traversing through AST, we usually have some neccessary inforamtion to pass into child node or get from parent, the `Context` class is designed to store the information that passing through nodes.

```c++
class Context {
 public:
  llvm::LLVMContext llvm_context_;
  llvm::Module llvm_module_;
  llvm::IRBuilder<> builder_;

  std::list<BlockAST*> blocks_;

  Context();
  ~Context();
};
```

The `llvm_context_`, `llvm_module_` and `builder_` store context of constructing LLVM IR. Since BLC has interactive mode which interpret each line of code immediately, all generated IR codes are contained in a singl function (main) in a single module (blc). `blocks_` is the block stack that used to isolate symbol table for nested blcoks.

The following of this part will explain three traversers one by one.

#### Interpreter

In the `ast_expression.cpp` file, the method `Evaluate()` shows how the interpreter works. Meanwhile, in the `ast_statement.cpp` , the method `Execute()` has the same function.

```c++
double DoubleAST::Evaluate(Context* context) { return value_; }

double BinaryOperationAST::Evaluate(Context* context) {
  auto lhs = lhs_->Evaluate(context);
  auto rhs = rhs_->Evaluate(context);

  switch (type_) {
    case '+':
      return lhs + rhs;
    case '-':
      return lhs - rhs;
    ...
    default:
      return 0;
  }
}
```

In the `ast_expression.cpp` file, it shows that how a interpreter returns a double number or an identifier, presents the binary operations, variable assignment and expression assignment. Take binary operation as an example, the method `Evaluate()` evaluate the result while reading the input, and finally return the result.

```c++
void WhileAST::Execute(Context* context) {
  while (condition_->Evaluate(context)) statement_->Run(context);
}
```

The `Execute()` method, which is objected to `statement` , also has the similiar features as `Evaluate()` method. Take `Execute()` in `WhileAST`  as an example, it first uses `Evaluate()` to check the condition, then uses `Run()` to execute the statement, and eventually return the result.

#### JSON Tree

This traverser creates a JSON tree and prints it. JSON is a data format consisting of key-value pairs and array types. Take `DoubleAST` as an example, the code of JSON tree traverser is as follows.

```c++
nlohmann::json DoubleAST::JsonTree() {
  nlohmann::json json;
  json["type"] = "double";
  json["value"] = value_;
  return json;
}
```

The code shows how JSON representation for a leaf node of the syntax tree is constructed. First an object of `nlohmann::json` is constructed.  `nlohmann::json` is the library used to generate JSON objects in C++. `json["type"] = "double";` means adding the key-value pair `"type": "double"` to the json object. This method adds the type and value of the node to the JSON object and returns it as the JSON representation of this node.

For non-leaf nodes of the syntax tree, the JSON representations of children of the nodes are added as a value of the children key-pair. Take `BinaryOperationAST` as an example.

```c++
nlohmann::json BinaryOperationAST::JsonTree() {
  nlohmann::json json;
  json["type"] = "BinaryOperation";
  json["lhs"] = lhs_->JsonTree();
  json["rhs"] = rhs_->JsonTree();
  switch (type_) {
    case GEQ:
      json["operationType"] = ">=";
      break;
    ......
  }
  return json;
}
```

The "lhs" and "rhs" key-value pairs are the one that contains children nodes. The `JsonTree` method of children are called to generate their JSON representation, and their returned JSON object are directly assigned to the corresponding key. In this way, a hierachy of nodes are created.

After a complete syntax tree is created, the `OnParsed` function will be called.

```c++
void OnParsed() {
  if (!ast) return;
  ...
  // JsonTree
  std::cout << "Parsed Syntax Tree:" << std::endl
            << ast->JsonTree().dump(4) << std::endl;
  ...
}
```

In the function, the `JsonTree` method of the root node will be called, which will treverse the whole tree and return the JSON object of it. The JSON object is then dumped to string with indentation of 4 spaces, which will be output to the screen.

#### Intermediate Representation

LLVM IR is a famous intermediate representation around the world. It can be compiled into assembly code and binary and target for different platforms easily by using (LLVM Static Compiler)[http://llvm.org/]. The functions can also be JIT compiled and run through LLVM ORC Engine, which will be much faster than interpreter.

The program create a global LLVM module named `blc` to contain all IR code. Since BLC respond to user input interactively, all instructions was generated in `main` function under `blc` module. All functions are generated as a LLVM function independently under `blc` module.

For each node of abstract syntax tree, method `GenIR` defined the procedure of generating intermediate representation of it and its children. The `builder_` in `context` store the insertion point of IR code and provide a set of interface to insert IR easily.

```c++
Value* DoubleAST::GenIR(Context* context) {
  return ConstantFP::get(Type::getFloatTy(context->llvm_context_), value_);
}
```

Take the easiest AST, `DoubleAST`, as an example. It simply create a constant float in LLVM IR that represent the value it contains.

```c++
llvm::Value* FunctionAST::GenIR(Context* context) {
  // Backup previous insertion point and block stack.
  auto previous_block = context->builder_.GetInsertBlock();
  auto previous_point = context->builder_.GetInsertPoint();
  auto previous_block_stack = context->blocks_;
  context->blocks_.clear();
  context->blocks_.push_back(new BlockAST());

  ...// Create function and set up arguments

  // Generate function body.
  auto ret = block_->GenIR(context);
  context->builder_.CreateRet(ret);

  // Resotre previous insertion point and block stack.
  delete context->blocks_.front();
  context->builder_.SetInsertPoint(previous_block, previous_point);
  context->blocks_ = previous_block_stack;
  return nullptr;
}
```

For other ASTs like `FunctionAST`, the generation logic may be very complex. The previous insertion point and block stack have to be backuped before creating function and restored after generation of function body, which ensures the function will have a isolated symbol table.

## The integration of code and compilation guide

The file structure of our project is as follows.

```bash
.
├── LICENSE.md
├── README.md
├── blc.sln
├── blc.vcxproj
├── blc.vcxproj.filters
├── blc.vcxproj.user
├── src
│   ├── Makefile
│   ├── ast.h
│   ├── ast_expression.cpp
│   ├── ast_statement.cpp
│   ├── blc.l
│   ├── blc.y
│   ├── context.hpp
│   └── main.cpp
└── third_party
    └── nlohmann
        └── json.hpp

3 directories, 15 files
```

Source code of the calculator project is stored in `src` directory. The file `ast.h` contains all the declarations of the abstract syntax tree classes, and the implementation of class methods are in `ast_expression.cpp` and `ast_statement.cpp`. `blc.l` and `blc.y` contains the code of lexer and syntax analyzer respectively. `Context` class is implemented in `context.hpp`, and `main.cpp` contains the `main` function and `OnParsed`. `Makefile` is the input file of `make`, which is used to compile the code.

The root directory of the project contains the Visual Studio project file as well as readme and lincense documents. `third_party` directory contains code of third_party libraries the project used.

### Compile for macOS & Linux

If users followed the installation guide previously in the report, they should be able to compile the code now. First go to the `src` directory, then enter the command below.

```bash
make all
```

After the command is finished, a file named `main` will be generated, which is the resulting program after compilation. Users can now run the `main` program and try out the functions of the calculator.

```bash
./main
[IN]<-
```

## Recommandation of Better Generators

We recommand ANTLR as a better code generator. It is a BSD lincenced open source software and is actively being developed, which means the bugs are being fixed and new features are being added. It supports a large variety of languages, including Java, C#, C++, JavaScript and Python; and is used by many big-name projects, such as twitter and NetBeans IDE. Therefore, its reliability is tested by thousands of experienced developers.

Since it is widely used, the community of this generator is very active, which means users can get help more easily from the developers and other users. It also has detailed documentation as well as plugins for many popular IDEs, making the developing process more convenient and efficient.