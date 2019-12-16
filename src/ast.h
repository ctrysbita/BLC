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
   * @return nlohmann::json JSON object of current AST.
   */
  virtual nlohmann::json JsonTree() = 0;

  /**
   * @brief A general interface for statements and expressions to run.
   *
   * @param context Context that store associated information.
   * @return double Return value. 0 for statements and evaluated value for
   * expressions.
   */
  virtual double Run(Context* context) = 0;

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
   * @brief Execute statement.
   *
   * @param context Context that store associated information.
   * @return double Return 0 if successful executed.
   */
  virtual double Run(Context* context) final override {
    Execute(context);
    return 0;
  };

  /**
   * @brief Execute current statement.
   * The method is not virtual because ';' is an empty statement that do
   * nothing.
   *
   * @param context Context that store associated information.
   */
  virtual void Execute(Context* context) {}

  virtual nlohmann::json JsonTree() override { return {}; };
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
   * @brief Evaluate expression and print result.
   *
   * @param context Context that store associated information.
   * @return double Evaluated value.
   */
  virtual double Run(Context* context) final override;

  /**
   * @brief Evaluate current expression.
   *
   * @param context Context that store associated information.
   * @return double The return value after evaluate.
   */
  virtual double Evaluate(Context* context) = 0;

  virtual nlohmann::json JsonTree() = 0;
  virtual llvm::Value* GenIR(Context* context) { return nullptr; }
};

/**
 * @brief AST that represent a code block.
 * A code block has an isolated symbol table and the table may inherit from
 * parent block.
 */
class BlockAST : public StatementAST {
 public:
  /**
   * @brief The type that can be bind to an identifier.
   * - double
   * - ExpressionAST*
   */
  typedef std::variant<double, ExpressionAST*> SymbolType;

 private:
  /**
   * @brief Symbol table for current code block.
   */
  std::map<std::string, SymbolType> symbols_;

  /**
   * @brief LLVM symbol table for current code block.
   */
  std::map<std::string, llvm::Value*> llvm_symbols_;

  /**
   * @brief Statements and expressions in current code block.
   */
  std::list<AST*> children_;

 public:
  BlockAST() {}
  virtual ~BlockAST() {}

  /**
   * @brief Get symbol from table if defined.
   *
   * @param name Symbol name.
   * @return std::optional<SymbolType> Symbol value.
   */
  inline std::optional<SymbolType> get_symbol(const std::string& name) {
    if (symbols_.find(name) != symbols_.end()) return symbols_[name];
    return {};
  }
  inline void set_symbol(const std::string& name, SymbolType&& value) {
    auto it = symbols_.find(name);
    if (it != symbols_.end() && it->second.index() == 1)
      delete std::get<ExpressionAST*>(it->second);
    symbols_[name] = value;
  }

  inline llvm::Value* get_llvm_symbol(const std::string& name) {
    return llvm_symbols_[name];
  }
  inline void set_llvm_symbol(const std::string& name, llvm::Value* value) {
    llvm_symbols_[name] = value;
  }

  /**
   * @brief To add other statements and expressions to children.
   *
   * @param asts List of statements and expressions to add.
   * @return BlockAST* Self.
   */
  BlockAST* WithChildren(std::list<AST*>* asts);

  virtual void Execute(Context* context) override;
  virtual nlohmann::json JsonTree() override;
  virtual llvm::Value* GenIR(Context* context) override;
};

class IfAST : public StatementAST {
 private:
  ExpressionAST* condition_;
  AST* then_;
  AST* else_;

 public:
  IfAST(ExpressionAST* condition, AST* then_statement,
        AST* else_statement = nullptr)
      : condition_(condition), then_(then_statement), else_(else_statement) {}
  virtual ~IfAST() {
    delete condition_;
    delete then_;
    delete else_;
  }

  virtual void Execute(Context* context) override;
  virtual nlohmann::json JsonTree() override;
  virtual llvm::Value* GenIR(Context* context) override;
};

class WhileAST : public StatementAST {
 private:
  ExpressionAST* condition_;
  AST* statement_;

 public:
  WhileAST(ExpressionAST* condition, AST* statement)
      : condition_(condition), statement_(statement) {}
  virtual ~WhileAST() {
    delete condition_;
    delete statement_;
  }

  virtual void Execute(Context* context) override;
  virtual nlohmann::json JsonTree() override;
  virtual llvm::Value* GenIR(Context* context) override;
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
  DoubleAST(double value) : value_(value) {}
  virtual ~DoubleAST() {}

  virtual double Evaluate(Context* context) override;
  virtual nlohmann::json JsonTree() override;
  virtual llvm::Value* GenIR(Context* context) override;
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
  virtual ~BinaryOperationAST() {
    delete lhs_;
    delete rhs_;
  }

  virtual double Evaluate(Context* context) override;
  virtual nlohmann::json JsonTree() override;
  virtual llvm::Value* GenIR(Context* context) override;
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

  virtual double Evaluate(Context* context) override;
  virtual nlohmann::json JsonTree() override;
  virtual llvm::Value* GenIR(Context* context) override;
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
  virtual ~VariableAssignmentAST() {
    delete name_;
    delete value_;
  }

  virtual double Evaluate(Context* context) override;
  virtual nlohmann::json JsonTree() override;
  virtual llvm::Value* GenIR(Context* context) override;
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
  virtual ~ExpressionAssignmentAST() {
    delete name_;
    // TODO: Unsafe. Handle value deletion.
  }

  virtual double Evaluate(Context* context) override;
  virtual nlohmann::json JsonTree() override;
  virtual llvm::Value* GenIR(Context* context) override;
};

class FunctionCallAST : public ExpressionAST {
 private:
  IdentifierAST* name_;
  std::vector<ExpressionAST*>* arguments_;

 public:
  FunctionCallAST(IdentifierAST* name, std::vector<ExpressionAST*>* arguments)
      : name_(name), arguments_(arguments) {}
  ~FunctionCallAST() {}

  virtual double Evaluate(Context* context) override;
  virtual nlohmann::json JsonTree() override;
  virtual llvm::Value* GenIR(Context* context) override;
};

class FunctionAST : public StatementAST {
 private:
  IdentifierAST* name_;
  std::vector<IdentifierAST*>* arguments_;
  BlockAST* block_;

 public:
  FunctionAST(IdentifierAST* name, std::vector<IdentifierAST*>* parameters,
              BlockAST* block)
      : name_(name), arguments_(parameters), block_(block) {}
  ~FunctionAST() {
    delete name_;
    for (auto arg : *arguments_) delete arg;
    delete arguments_;
    delete block_;
  }

  virtual void Execute(Context* context) override;
  virtual nlohmann::json JsonTree() override;
  virtual llvm::Value* GenIR(Context* context) override;

  friend class FunctionCallAST;
};
