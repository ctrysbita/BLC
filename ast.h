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
    // Get real value from symbol table.
    return 0;
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
    return value_->eval(context);
  }
};