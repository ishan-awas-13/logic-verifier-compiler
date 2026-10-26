#pragma once

#include "ast.h"
#include "tac.h"
#include <unordered_map>
#include <vector>

namespace cmm {

class TACGenerator : public StmtVisitor, public ExprVisitor {
public:
    TACGenerator() = default;

    TACProgram generate(const Program& program);

    // StmtVisitor
    void visit_var_decl(const VarDecl& stmt) override;
    void visit_assign(const AssignStmt& stmt) override;
    void visit_emit(const EmitStmt& stmt) override;
    void visit_if(const IfStmt& stmt) override;
    void visit_while(const WhileStmt& stmt) override;
    void visit_block(const BlockStmt& stmt) override;
    void visit_give(const GiveStmt& stmt) override;
    void visit_stop(const StopStmt& stmt) override;
    void visit_skip(const SkipStmt& stmt) override;

    // ExprVisitor
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

private:
    struct SymbolInfo {
        std::string tac_name;
        Type type = Type::Void;
    };

    struct LoopLabels {
        std::string start_label;
        std::string end_label;
    };

    std::string declare_var(const std::string& name, Type type);
    SymbolInfo lookup_var(const std::string& name);

    TACProgram program_;
    TACOperand current_operand_;

    std::unordered_map<std::string, int> decl_counts_;
    std::vector<std::unordered_map<std::string, SymbolInfo>> scopes_;
    std::vector<LoopLabels> loop_stack_;
};

} // namespace cmm

