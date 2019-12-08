#pragma once
#include "llvm/IR/Value.h"

class AST {
 public:
  AST() {}
  ~AST() {}

  virtual llvm::Value* GenIR() { return nullptr; }
};

class StatementAST : public AST {
 public:
  StatementAST() {}
  ~StatementAST() {}

  virtual llvm::Value* GenIR() { return nullptr; }
};

class ExpressionAST : public AST {
 public:
  ExpressionAST() {}
  ~ExpressionAST() {}

  virtual llvm::Value* GenIR() { return nullptr; }
};

class DoubleAST : public ExpressionAST {
 private:
  double value_;

 public:
  DoubleAST(std::string* value) : value_(std::stod(*value)) {}
  ~DoubleAST() {}

  virtual llvm::Value* GenIR() { return nullptr; }
};

class BinaryOperationAST : public ExpressionAST {
 private:
  int type_;
  AST* lhs_;
  AST* rhs_;

 public:
  BinaryOperationAST(int type, AST* lhs, AST* rhs)
      : type_(type), lhs_(lhs), rhs_(rhs) {}
  ~BinaryOperationAST() {}

  virtual llvm::Value* GenIR() { return nullptr; }
};