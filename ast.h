#pragma once
#include <llvm/IR/Value.h>
#include <list>
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
  union SymbolValueType {
    double value;
    const ExpressionAST* expression;
  };

 private:
  std::map<std::string, SymbolValueType> symbols_;
  std::map<std::string, int> types_;

  std::list<AST*> children_;

 public:
  BlockAST() {}
  virtual ~BlockAST() {}

  inline const SymbolValueType get_symbol(std::string name) {
    return symbols_[name];
  }
  inline int get_type(std::string name) { return types_[name]; }
  inline void set_symbol(std::string name, int type, SymbolValueType value) {
    types_[name] = type;
    symbols_[name] = value;
  }

  BlockAST* WithChildren(std::list<AST*>* asts) {
    children_.splice(children_.end(), *asts);
    delete asts;
    return this;
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