#pragma once

#include "ast.h"
#include <string>

namespace cmm {

class AstPrinter : public ExprVisitor, public StmtVisitor {
public:
    void print(const Program& program);
    std::string print_expr(const Expr& expr);

    void visit_number(const NumberLiteral& expr) override;
    void visit_string(const StringLiteral& expr) override;
    void visit_char(const CharLiteral& expr) override;
    void visit_bool(const BoolLiteral& expr) override;
    void visit_variable(const VariableExpr& expr) override;
    void visit_unary(const UnaryExpr& expr) override;
    void visit_binary(const BinaryExpr& expr) override;
    void visit_grouping(const GroupingExpr& expr) override;
    void visit_input(const InputExpr& expr) override;
    void visit_call(const CallExpr& expr) override;

    void visit_var_decl(const VarDecl& stmt) override;
    void visit_assign(const AssignStmt& stmt) override;
    void visit_emit(const EmitStmt& stmt) override;
    void visit_if(const IfStmt& stmt) override;
    void visit_while(const WhileStmt& stmt) override;
    void visit_block(const BlockStmt& stmt) override;
    void visit_give(const GiveStmt& stmt) override;
    void visit_stop(const StopStmt& stmt) override;
    void visit_skip(const SkipStmt& stmt) override;

private:
    int indent_level = 0;
    std::string result_;
    void indent();
    void print_line(const std::string& line);
};

} // namespace cmm
