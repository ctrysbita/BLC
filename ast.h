#pragma once
#include "context.hpp"
#include <llvm/IR/Value.h>

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

  virtual llvm::Value* GenIR(Context* context) { return nullptr; }
};

class ExpressionAST : public AST {
 public:
  ExpressionAST() {}
  virtual ~ExpressionAST() {}

  virtual double eval(Context* context) const = 0;
  virtual llvm::Value* GenIR(Context* context) { return nullptr; }
};

class BlockAST : public StatementAST {
 public:
  union SymbolValueType {
    double value;
    const ExpressionAST* expression;
  };

 private:
  std::map<std::string, SymbolValueType> symbols_;
  std::map<std::string, int> types_;

 public:
  BlockAST() {}
  virtual ~BlockAST() {}

  inline const SymbolValueType get_symbol(std::string name) {
    return symbols_[name];
  }
  inline int get_type(std::string name) { return types_[name]; }
  inline void SetSymbol(std::string name, int type, SymbolValueType value) {
    types_[name] = type;
    symbols_[name] = value;
  }
};

class DoubleAST : public ExpressionAST {
 private:
  const double value_;

 public:
  DoubleAST(std::string* value) : value_(std::stod(*value)) {}
  virtual ~DoubleAST() {}

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
  virtual ~BinaryOperationAST() {}

  virtual double eval(Context* context) const override;
};

class IdentifierAST : public ExpressionAST {
 public:
  const std::string name_;

  IdentifierAST(std::string* name) : name_(*name) { delete name; }
  virtual ~IdentifierAST() {}

  virtual double eval(Context* context) const override;
};

class VariableAssignmentAST : public ExpressionAST {
 public:
  const IdentifierAST* name_;
  const ExpressionAST* value_;

  VariableAssignmentAST(IdentifierAST* name, ExpressionAST* value)
      : name_(name), value_(value) {}
  virtual ~VariableAssignmentAST() {}

  virtual double eval(Context* context) const override;
};

class ExpressionAssignmentAST : public ExpressionAST {
 private:
  const IdentifierAST* name_;
  const ExpressionAST* value_;

 public:
  ExpressionAssignmentAST(IdentifierAST* name, ExpressionAST* value)
      : name_(name), value_(value) {}
  virtual ~ExpressionAssignmentAST() {}

  virtual double eval(Context* context) const override;
};