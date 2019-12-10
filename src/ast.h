#pragma once
#include <llvm/IR/Value.h>

#include <list>
#include <map>
#include <memory>
#include <string>
#include <variant>

#include "context.hpp"

class AST {
 public:
  AST() {}
  virtual ~AST() {}

  virtual llvm::Value* GenIR(Context* context) = 0;
};

class StatementAST : public AST {
 public:
  StatementAST() {}
  virtual ~StatementAST() {}

  virtual void Execute(Context* context) {}
  virtual llvm::Value* GenIR(Context* context) { return nullptr; }
};

class ExpressionAST : public AST {
 public:
  ExpressionAST() {}
  virtual ~ExpressionAST() {}

  virtual double Evalutate(Context* context) const = 0;
  virtual llvm::Value* GenIR(Context* context) { return nullptr; }
};

class BlockAST : public StatementAST {
 public:
  typedef std::variant<double, const ExpressionAST*> SymbolType;

 private:
  std::map<std::string, SymbolType*> symbols_;

  std::list<AST*> children_;

 public:
  BlockAST() {}
  virtual ~BlockAST() {}

  inline SymbolType get_symbol(std::string name) { return *(symbols_[name]); }
  inline void set_symbol(std::string name, SymbolType&& value) {
    // TODO: Unsafe.
    if (symbols_.find(name) != symbols_.end())
      *symbols_[name] = value;
    else
      symbols_.insert({name, new SymbolType(value)});
  }

  BlockAST* WithChildren(std::list<AST*>* asts) {
    children_.splice(children_.end(), *asts);
    delete asts;
    return this;
  }

  virtual void Execute(Context* context) override;
};

class IfAST : public StatementAST {
 private:
  ExpressionAST* condition_;
  StatementAST* statement_;

 public:
  IfAST(ExpressionAST* condition, StatementAST* statement)
      : condition_(condition), statement_(statement) {}
  ~IfAST() {}

  virtual void Execute(Context* context) override {
    if (condition_->Evalutate(context)) statement_->Execute(context);
  }
};

class WhileAST : public StatementAST {
 private:
  ExpressionAST* condition_;
  StatementAST* statement_;

 public:
  WhileAST(ExpressionAST* condition, StatementAST* statement)
      : condition_(condition), statement_(statement) {}
  ~WhileAST() {}

  virtual void Execute(Context* context) override {
    while (condition_->Evalutate(context)) statement_->Execute(context);
  }
};

class DoubleAST : public ExpressionAST {
 private:
  const double value_;

 public:
  DoubleAST(std::string* value) : value_(std::stod(*value)) {}
  virtual ~DoubleAST() {}

  virtual double Evalutate(Context* context) const override { return value_; }
};

class BinaryOperationAST : public ExpressionAST {
 private:
  const int type_;
  const ExpressionAST* lhs_;
  const ExpressionAST* rhs_;

 public:
  BinaryOperationAST(int type, ExpressionAST* lhs, ExpressionAST* rhs)
      : type_(type), lhs_(lhs), rhs_(rhs) {}
  virtual ~BinaryOperationAST() {}

  virtual double Evalutate(Context* context) const override;
};

class IdentifierAST : public ExpressionAST {
 public:
  const std::string name_;

  IdentifierAST(std::string* name) : name_(*name) { delete name; }
  virtual ~IdentifierAST() {}

  virtual double Evalutate(Context* context) const override;
};

class VariableAssignmentAST : public ExpressionAST {
 public:
  const IdentifierAST* name_;
  const ExpressionAST* value_;

  VariableAssignmentAST(IdentifierAST* name, ExpressionAST* value)
      : name_(name), value_(value) {}
  virtual ~VariableAssignmentAST() {}

  virtual double Evalutate(Context* context) const override;
};

class ExpressionAssignmentAST : public ExpressionAST {
 private:
  const IdentifierAST* name_;
  const ExpressionAST* value_;

 public:
  ExpressionAssignmentAST(IdentifierAST* name, ExpressionAST* value)
      : name_(name), value_(value) {}
  virtual ~ExpressionAssignmentAST() {}

  virtual double Evalutate(Context* context) const override;
};