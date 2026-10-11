#pragma once

#include <memory>
#include <string>
#include <vector>
#include "token.h"
#include "types.h"

// Note for Stage 4: until means "loop while the condition is FALSE" (run until the condition becomes true)

namespace cmm {

class NumberLiteral;
class StringLiteral;
class CharLiteral;
class BoolLiteral;
class VariableExpr;
class UnaryExpr;
class BinaryExpr;
class GroupingExpr;
class InputExpr;

class ExprVisitor {
public:
    virtual ~ExprVisitor() = default;
    virtual void visit_number(const NumberLiteral& expr) = 0;
    virtual void visit_string(const StringLiteral& expr) = 0;
    virtual void visit_char(const CharLiteral& expr) = 0;
    virtual void visit_bool(const BoolLiteral& expr) = 0;
    virtual void visit_variable(const VariableExpr& expr) = 0;
    virtual void visit_unary(const UnaryExpr& expr) = 0;
    virtual void visit_binary(const BinaryExpr& expr) = 0;
    virtual void visit_grouping(const GroupingExpr& expr) = 0;
    virtual void visit_input(const InputExpr& expr) = 0;
};

class Expr {
public:
    Expr(int line, int col) : line(line), col(col) {}
    virtual ~Expr() = default;
    virtual void accept(ExprVisitor& visitor) const = 0;

    int line;
    int col;
    mutable Type type = Type::Error;
    mutable bool is_const = false;
};

class NumberLiteral : public Expr {
public:
    double value;
    bool is_decimal;
    NumberLiteral(int line, int col, double value, bool is_decimal = false)
        : Expr(line, col), value(value), is_decimal(is_decimal) {}
    void accept(ExprVisitor& visitor) const override { visitor.visit_number(*this); }
};

class StringLiteral : public Expr {
public:
    std::string value;
    StringLiteral(int line, int col, std::string value) : Expr(line, col), value(std::move(value)) {}
    void accept(ExprVisitor& visitor) const override { visitor.visit_string(*this); }
};

class CharLiteral : public Expr {
public:
    char value;
    CharLiteral(int line, int col, char value) : Expr(line, col), value(value) {}
    void accept(ExprVisitor& visitor) const override { visitor.visit_char(*this); }
};

class BoolLiteral : public Expr {
public:
    bool value;
    BoolLiteral(int line, int col, bool value) : Expr(line, col), value(value) {}
    void accept(ExprVisitor& visitor) const override { visitor.visit_bool(*this); }
};

class VariableExpr : public Expr {
public:
    std::string name;
    VariableExpr(int line, int col, std::string name) : Expr(line, col), name(std::move(name)) {}
    void accept(ExprVisitor& visitor) const override { visitor.visit_variable(*this); }
};

class UnaryExpr : public Expr {
public:
    Token op;
    std::unique_ptr<Expr> right;
    UnaryExpr(int line, int col, Token op, std::unique_ptr<Expr> right) 
        : Expr(line, col), op(std::move(op)), right(std::move(right)) {}
    void accept(ExprVisitor& visitor) const override { visitor.visit_unary(*this); }
};

class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
    BinaryExpr(int line, int col, std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : Expr(line, col), left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
    void accept(ExprVisitor& visitor) const override { visitor.visit_binary(*this); }
};

class GroupingExpr : public Expr {
public:
    std::unique_ptr<Expr> expr;
    GroupingExpr(int line, int col, std::unique_ptr<Expr> expr)
        : Expr(line, col), expr(std::move(expr)) {}
    void accept(ExprVisitor& visitor) const override { visitor.visit_grouping(*this); }
};

class InputExpr : public Expr {
public:
    InputExpr(int line, int col) : Expr(line, col) {}
    void accept(ExprVisitor& visitor) const override { visitor.visit_input(*this); }
};

class VarDecl;
class AssignStmt;
class EmitStmt;
class IfStmt;
class WhileStmt;
class BlockStmt;
class GiveStmt;
class StopStmt;
class SkipStmt;

class StmtVisitor {
public:
    virtual ~StmtVisitor() = default;
    virtual void visit_var_decl(const VarDecl& stmt) = 0;
    virtual void visit_assign(const AssignStmt& stmt) = 0;
    virtual void visit_emit(const EmitStmt& stmt) = 0;
    virtual void visit_if(const IfStmt& stmt) = 0;
    virtual void visit_while(const WhileStmt& stmt) = 0;
    virtual void visit_block(const BlockStmt& stmt) = 0;
    virtual void visit_give(const GiveStmt& stmt) = 0;
    virtual void visit_stop(const StopStmt& stmt) = 0;
    virtual void visit_skip(const SkipStmt& stmt) = 0;
};

class Stmt {
public:
    Stmt(int line, int col) : line(line), col(col) {}
    virtual ~Stmt() = default;
    virtual void accept(StmtVisitor& visitor) const = 0;

    int line;
    int col;
};

class VarDecl : public Stmt {
public:
    std::string type_name;
    std::string name;
    std::unique_ptr<Expr> initializer;
    bool is_fixed;

    VarDecl(int line, int col, std::string type_name, std::string name, std::unique_ptr<Expr> initializer, bool is_fixed = false)
        : Stmt(line, col), type_name(std::move(type_name)), name(std::move(name)), initializer(std::move(initializer)), is_fixed(is_fixed) {}
    void accept(StmtVisitor& visitor) const override { visitor.visit_var_decl(*this); }
};

enum class AssignKind {
    Assign,
    Increment,
    Decrement
};

class AssignStmt : public Stmt {
public:
    std::string name;
    std::unique_ptr<Expr> value;
    AssignKind kind;

    AssignStmt(int line, int col, std::string name, std::unique_ptr<Expr> value, AssignKind kind = AssignKind::Assign)
        : Stmt(line, col), name(std::move(name)), value(std::move(value)), kind(kind) {}
    void accept(StmtVisitor& visitor) const override { visitor.visit_assign(*this); }
};

class EmitStmt : public Stmt {
public:
    std::unique_ptr<Expr> expr;
    EmitStmt(int line, int col, std::unique_ptr<Expr> expr)
        : Stmt(line, col), expr(std::move(expr)) {}
    void accept(StmtVisitor& visitor) const override { visitor.visit_emit(*this); }
};

class BlockStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> statements;
    BlockStmt(int line, int col, std::vector<std::unique_ptr<Stmt>> statements)
        : Stmt(line, col), statements(std::move(statements)) {}
    void accept(StmtVisitor& visitor) const override { visitor.visit_block(*this); }
};

class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<BlockStmt> then_branch;
    std::unique_ptr<BlockStmt> else_branch;
    IfStmt(int line, int col, std::unique_ptr<Expr> condition, std::unique_ptr<BlockStmt> then_branch, std::unique_ptr<BlockStmt> else_branch)
        : Stmt(line, col), condition(std::move(condition)), then_branch(std::move(then_branch)), else_branch(std::move(else_branch)) {}
    void accept(StmtVisitor& visitor) const override { visitor.visit_if(*this); }
};

class WhileStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<BlockStmt> body;
    WhileStmt(int line, int col, std::unique_ptr<Expr> condition, std::unique_ptr<BlockStmt> body)
        : Stmt(line, col), condition(std::move(condition)), body(std::move(body)) {}
    void accept(StmtVisitor& visitor) const override { visitor.visit_while(*this); }
};

class GiveStmt : public Stmt {
public:
    std::unique_ptr<Expr> value;
    GiveStmt(int line, int col, std::unique_ptr<Expr> value)
        : Stmt(line, col), value(std::move(value)) {}
    void accept(StmtVisitor& visitor) const override { visitor.visit_give(*this); }
};

class StopStmt : public Stmt {
public:
    StopStmt(int line, int col) : Stmt(line, col) {}
    void accept(StmtVisitor& visitor) const override { visitor.visit_stop(*this); }
};

class SkipStmt : public Stmt {
public:
    SkipStmt(int line, int col) : Stmt(line, col) {}
    void accept(StmtVisitor& visitor) const override { visitor.visit_skip(*this); }
};

struct Program {
    std::vector<std::unique_ptr<Stmt>> statements;
};

} // namespace cmm
