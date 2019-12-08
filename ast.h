#pragma once
#include "context.hpp"
#include "llvm/IR/Value.h"

class AST {
 public:
  AST() {}
  ~AST() {}

  virtual llvm::Value* GenIR(Context* context) = 0;
};

class StatementAST : public AST {
 public:
  StatementAST() {}
  ~StatementAST() {}

  virtual llvm::Value* GenIR(Context* context) { return nullptr; }
};

class ExpressionAST : public AST {
 public:
  ExpressionAST() {}
  ~ExpressionAST() {}

  virtual double eval(Context* context) const = 0;
  virtual llvm::Value* GenIR(Context* context) { return nullptr; }
};

class BlockAST {
 private:
  std::map<std::string, const ExpressionAST*> symbols_;

 public:
  BlockAST() {}
  ~BlockAST() {}

  const ExpressionAST* GetSymbol(std::string name) { return symbols_[name]; }
  void SetSymbol(std::string name, const ExpressionAST* value) {
    symbols_[name] = value;
  }
};

class DoubleAST : public ExpressionAST {
 private:
  const double value_;

 public:
  DoubleAST(std::string* value) : value_(std::stod(*value)) {}
  ~DoubleAST() {}

  virtual double eval(Context* context) const override { return value_; }
};

class BinaryOperationAST : public ExpressionAST {
 private:
  const int type_;
  const ExpressionAST* lhs_;
  const ExpressionAST* rhs_;

 public:
  BinaryOperationAST(int type, ExpressionAST* lhs, ExpressionAST* rhs)
      : type_(type), lhs_(lhs), rhs_(rhs) {}
  ~BinaryOperationAST() {}

  virtual double eval(Context* context) const override;
};

class IdentifierAST : public ExpressionAST {
 public:
  const std::string name_;

  IdentifierAST(std::string* name) : name_(*name) { delete name; }
  ~IdentifierAST() {}

  virtual double eval(Context* context) const override {
    auto exp = context->blocks_.top()->GetSymbol(name_);
    return exp->eval(context);
  }
};

class VariableAssignAST : public ExpressionAST {
 public:
  const IdentifierAST* name_;
  const ExpressionAST* value_;

  VariableAssignAST(IdentifierAST* name, ExpressionAST* value)
      : name_(name), value_(value) {}
  ~VariableAssignAST() {}

  virtual double eval(Context* context) const override {
    context->blocks_.top()->SetSymbol(name_->name_, value_);
    return value_->eval(context);
  }
};