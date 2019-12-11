#pragma once
#include <llvm/IR/Value.h>
#include <nlohmann/json.hpp>

#include <list>
#include <map>
#include <memory>
#include <string>
#include <variant>

#include "context.hpp"

/**
 * @brief The base class for Abstract Syntax Tree.
 */
class AST {
 public:
  AST() {}
  virtual ~AST() {}

  /**
   * @brief Generate JSON format syntax tree.
   *
   * @param context Context that store associated information.
   * @return nlohmann::json JSON object of current AST.
   */
  virtual nlohmann::json JsonTree(Context* context) = 0;

  /**
   * @brief Generate LLVM IR for current AST.
   *
   * @param context Context that store associated information.
   * @return llvm::Value*
   */
  virtual llvm::Value* GenIR(Context* context) = 0;
};

/**
 * @brief The base class for all statement ASTs.
 * Statement can always be executed and normally doesn't have an exact return
 * value.
 */
class StatementAST : public AST {
 public:
  StatementAST() {}
  virtual ~StatementAST() {}

  /**
   * @brief Execute current statement.
   * The method is not virtual because ';' is an empty statement that do
   * nothing.
   *
   * @param context Context that store associated information.
   */
  virtual void Execute(Context* context) {}

  virtual nlohmann::json JsonTree(Context* context) { return {}; };
  virtual llvm::Value* GenIR(Context* context) { return nullptr; }
};

/**
 * @brief The base class for all expression ASTs.
 * Expression can always be evaluated and has an exact return value.
 */
class ExpressionAST : public AST {
 public:
  ExpressionAST() {}
  virtual ~ExpressionAST() {}

  /**
   * @brief Evaluate current expression.
   *
   * @param context Context that store associated information.
   * @return double The return value after evaluate.
   */
  virtual double Evalutate(Context* context) = 0;

  virtual nlohmann::json JsonTree(Context* context) = 0;
  virtual llvm::Value* GenIR(Context* context) { return nullptr; }
};

/**
 * @brief AST that represent a code block.
 * A code block has an isolated symbol table and the table may inherit from
 * parent block.
 */
class BlockAST : public StatementAST {
 public:
  typedef std::variant<double, ExpressionAST*> SymbolType;

 private:
  std::map<std::string, SymbolType*> symbols_;

  std::list<AST*> children_;

 public:
  BlockAST() {}
  virtual ~BlockAST() {}

  inline SymbolType get_symbol(std::string name) { return *symbols_[name]; }
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

/**
 * @brief AST that represent single double value, which is the only value type
 * in blc.
 */
class DoubleAST : public ExpressionAST {
 private:
  double value_;

 public:
  DoubleAST(std::string* value) : value_(std::stod(*value)) {}
  virtual ~DoubleAST() {}

  virtual double Evalutate(Context* context) override;
  virtual nlohmann::json JsonTree(Context* context) override;
};

/**
 * @brief AST that represent a binary operation. (like '+' '-' '*' '/')
 */
class BinaryOperationAST : public ExpressionAST {
 private:
  int type_;
  ExpressionAST* lhs_;
  ExpressionAST* rhs_;

 public:
  BinaryOperationAST(int type, ExpressionAST* lhs, ExpressionAST* rhs)
      : type_(type), lhs_(lhs), rhs_(rhs) {}
  virtual ~BinaryOperationAST() {}

  virtual double Evalutate(Context* context) override;
  virtual nlohmann::json JsonTree(Context* context) override;
};

/**
 * @brief AST that represent an identifier.
 */
class IdentifierAST : public ExpressionAST {
 private:
  std::string name_;

 public:
  IdentifierAST(std::string* name) : name_(*name) { delete name; }
  virtual ~IdentifierAST() {}

  inline const std::string& get_name() { return name_; }

  virtual double Evalutate(Context* context) override;
  virtual nlohmann::json JsonTree(Context* context) override;
};

/**
 * @brief AST that represent an variable assignment.
 * Assign a direct value or evaluated value to identifier.
 */
class VariableAssignmentAST : public ExpressionAST {
 private:
  IdentifierAST* name_;
  ExpressionAST* value_;

 public:
  VariableAssignmentAST(IdentifierAST* name, ExpressionAST* value)
      : name_(name), value_(value) {}
  virtual ~VariableAssignmentAST() {}

  virtual double Evalutate(Context* context) override;
  virtual nlohmann::json JsonTree(Context* context) override;
};

/**
 * @brief AST that represent an expression assignment.
 * Assign a expression to identifier. The expression will be evaluated when use.
 */
class ExpressionAssignmentAST : public ExpressionAST {
 private:
  IdentifierAST* name_;
  ExpressionAST* value_;

 public:
  ExpressionAssignmentAST(IdentifierAST* name, ExpressionAST* value)
      : name_(name), value_(value) {}
  virtual ~ExpressionAssignmentAST() {}

  virtual double Evalutate(Context* context) override;
  virtual nlohmann::json JsonTree(Context* context) override;
};