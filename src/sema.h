#pragma once

#include "ast.h"
#include "symbol_table.h"
#include "types.h"
#include <vector>
#include <string>
#include <algorithm>

namespace cmm {

enum class DiagnosticSeverity {
    Warning,
    Error
};

struct Diagnostic {
    DiagnosticSeverity severity;
    int line;
    int col;
    std::string message;

    bool operator<(const Diagnostic& other) const {
        if (line != other.line) return line < other.line;
        return col < other.col;
    }
};

class SemanticAnalyzer : public ExprVisitor, public StmtVisitor {
public:
    SemanticAnalyzer();

    bool analyze(const Program& program);

    const std::vector<Diagnostic>& get_diagnostics() const { return diagnostics_; }
    bool has_errors() const { return error_count_ > 0; }
    const SymbolTable& get_symbol_table() const { return symbols_; }

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

private:
    SymbolTable symbols_;
    std::vector<Diagnostic> diagnostics_;
    int error_count_ = 0;
    int loop_depth_ = 0;

    bool input_allowed_ = false;
    Type input_target_type_ = Type::Void;

    void error(int line, int col, const std::string& msg);
    void warning(int line, int col, const std::string& msg);
    void check_block_statements(const std::vector<std::unique_ptr<Stmt>>& statements);
};

} // namespace cmm

