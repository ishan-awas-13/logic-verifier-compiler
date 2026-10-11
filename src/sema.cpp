#include "sema.h"

namespace cmm {

SemanticAnalyzer::SemanticAnalyzer() {}

bool SemanticAnalyzer::analyze(const Program& program) {
    check_block_statements(program.statements);

    // After analyzing all statements, check for unused variables if no errors
    if (error_count_ == 0) {
        for (const auto& sym : symbols_.get_all_symbols()) {
            if (!sym->used) {
                warning(sym->line, sym->col, "variable '" + sym->name + "' is declared but never read");
            }
        }
    }

    std::sort(diagnostics_.begin(), diagnostics_.end());
    return error_count_ == 0;
}

void SemanticAnalyzer::error(int line, int col, const std::string& msg) {
    diagnostics_.push_back({DiagnosticSeverity::Error, line, col, "line " + std::to_string(line) + ", col " + std::to_string(col) + ": error: " + msg});
    error_count_++;
}

void SemanticAnalyzer::warning(int line, int col, const std::string& msg) {
    diagnostics_.push_back({DiagnosticSeverity::Warning, line, col, "line " + std::to_string(line) + ", col " + std::to_string(col) + ": warning: " + msg});
}

void SemanticAnalyzer::check_block_statements(const std::vector<std::unique_ptr<Stmt>>& statements) {
    bool unreachable = false;
    for (const auto& stmt : statements) {
        if (unreachable) {
            warning(stmt->line, stmt->col, "unreachable statement");
        }
        stmt->accept(*this);
        if (dynamic_cast<const GiveStmt*>(stmt.get()) ||
            dynamic_cast<const StopStmt*>(stmt.get()) ||
            dynamic_cast<const SkipStmt*>(stmt.get())) {
            unreachable = true;
        }
    }
}

void SemanticAnalyzer::visit_var_decl(const VarDecl& stmt) {
    Type decl_type = type_from_string(stmt.type_name);
    if (decl_type == Type::Error) {
        error(stmt.line, stmt.col, "unknown type '" + stmt.type_name + "'");
        return;
    }

    bool success = symbols_.declare(stmt.name, decl_type, stmt.is_fixed, stmt.line, stmt.col);
    if (!success) {
        error(stmt.line, stmt.col, "redeclaration of variable '" + stmt.name + "' in the same scope");
    }

    if (stmt.is_fixed && !stmt.initializer) {
        error(stmt.line, stmt.col, "fixed variable '" + stmt.name + "' must have an initializer");
    }

    if (stmt.initializer) {
        if (dynamic_cast<const InputExpr*>(stmt.initializer.get())) {
            if (decl_type == Type::List) {
                error(stmt.line, stmt.col, "cannot assign input() to list");
            }
            input_allowed_ = true;
            input_target_type_ = decl_type;
            stmt.initializer->accept(*this);
            input_allowed_ = false;
        } else {
            stmt.initializer->accept(*this);
            Type init_type = stmt.initializer->type;
            if (init_type != Type::Error && !is_assignable(decl_type, init_type)) {
                error(stmt.line, stmt.col, "type mismatch: cannot assign " + to_string(init_type) + " to variable '" + stmt.name + "' of type " + to_string(decl_type));
            }
        }
    }
}

void SemanticAnalyzer::visit_assign(const AssignStmt& stmt) {
    Symbol* sym = symbols_.lookup(stmt.name);
    if (!sym) {
        error(stmt.line, stmt.col, "undeclared variable '" + stmt.name + "'");
        if (stmt.value) stmt.value->accept(*this);
        return;
    }

    if (stmt.kind == AssignKind::Increment || stmt.kind == AssignKind::Decrement) {
        sym->used = true;
        std::string op_str = (stmt.kind == AssignKind::Increment ? "++" : "--");
        if (sym->is_fixed) {
            error(stmt.line, stmt.col, "cannot use " + op_str + " on fixed variable '" + stmt.name + "'");
        }
        if (!is_numeric(sym->type)) {
            error(stmt.line, stmt.col, op_str + " requires a variable of type num or decimal, found '" + stmt.name + "' of type " + to_string(sym->type));
        }
        return;
    }

    if (sym->is_fixed) {
        error(stmt.line, stmt.col, "cannot assign to fixed variable '" + stmt.name + "'");
    }

    if (dynamic_cast<const InputExpr*>(stmt.value.get())) {
        if (sym->type == Type::List) {
            error(stmt.line, stmt.col, "cannot assign input() to list");
        }
        input_allowed_ = true;
        input_target_type_ = sym->type;
        stmt.value->accept(*this);
        input_allowed_ = false;
    } else {
        stmt.value->accept(*this);
        Type val_type = stmt.value->type;
        if (val_type != Type::Error && !is_assignable(sym->type, val_type)) {
            error(stmt.line, stmt.col, "type mismatch: cannot assign " + to_string(val_type) + " to variable '" + stmt.name + "' of type " + to_string(sym->type));
        }
    }
}

void SemanticAnalyzer::visit_emit(const EmitStmt& stmt) {
    stmt.expr->accept(*this);
    if (stmt.expr->type != Type::Error) {
        if (stmt.expr->type == Type::List || stmt.expr->type == Type::Void) {
            error(stmt.line, stmt.col, "cannot show value of type " + to_string(stmt.expr->type));
        }
    }
}

void SemanticAnalyzer::visit_if(const IfStmt& stmt) {
    stmt.condition->accept(*this);
    if (stmt.condition->type != Type::Error && stmt.condition->type != Type::Boolean) {
        error(stmt.line, stmt.col, "condition must be boolean, found " + to_string(stmt.condition->type));
    }
    if (stmt.then_branch) stmt.then_branch->accept(*this);
    if (stmt.else_branch) stmt.else_branch->accept(*this);
}

void SemanticAnalyzer::visit_while(const WhileStmt& stmt) {
    stmt.condition->accept(*this);
    if (stmt.condition->type != Type::Error && stmt.condition->type != Type::Boolean) {
        error(stmt.line, stmt.col, "condition must be boolean, found " + to_string(stmt.condition->type));
    }
    loop_depth_++;
    if (stmt.body) stmt.body->accept(*this);
    loop_depth_--;
}

void SemanticAnalyzer::visit_block(const BlockStmt& stmt) {
    symbols_.enter_scope();
    check_block_statements(stmt.statements);
    symbols_.exit_scope();
}

void SemanticAnalyzer::visit_give(const GiveStmt& stmt) {
    if (!stmt.value) {
        error(stmt.line, stmt.col, "'give' statement requires num exit code");
    } else {
        stmt.value->accept(*this);
        if (stmt.value->type != Type::Error && stmt.value->type != Type::Num) {
            error(stmt.line, stmt.col, "'give' statement requires num exit code, found " + to_string(stmt.value->type));
        }
    }
}

void SemanticAnalyzer::visit_stop(const StopStmt& stmt) {
    if (loop_depth_ == 0) {
        error(stmt.line, stmt.col, "'stop' is only allowed inside an until loop");
    }
}

void SemanticAnalyzer::visit_skip(const SkipStmt& stmt) {
    if (loop_depth_ == 0) {
        error(stmt.line, stmt.col, "'skip' is only allowed inside an until loop");
    }
}

void SemanticAnalyzer::visit_number(const NumberLiteral& expr) {
    expr.type = expr.is_decimal ? Type::Decimal : Type::Num;
    expr.is_const = true;
}

void SemanticAnalyzer::visit_string(const StringLiteral& expr) {
    expr.type = Type::Word;
    expr.is_const = true;
}

void SemanticAnalyzer::visit_char(const CharLiteral& expr) {
    expr.type = Type::Letter;
    expr.is_const = true;
}

void SemanticAnalyzer::visit_bool(const BoolLiteral& expr) {
    expr.type = Type::Boolean;
    expr.is_const = true;
}

void SemanticAnalyzer::visit_variable(const VariableExpr& expr) {
    Symbol* sym = symbols_.lookup(expr.name);
    if (!sym) {
        error(expr.line, expr.col, "undeclared variable '" + expr.name + "'");
        expr.type = Type::Error;
        return;
    }
    sym->used = true;
    expr.type = sym->type;
    expr.is_const = sym->is_fixed;
}

void SemanticAnalyzer::visit_unary(const UnaryExpr& expr) {
    expr.right->accept(*this);
    Type rt = expr.right->type;
    if (rt == Type::Error) {
        expr.type = Type::Error;
        return;
    }

    if (expr.op.type == TokenType::Not) {
        if (rt != Type::Boolean) {
            error(expr.line, expr.col, "operator 'not' requires boolean operand, found " + to_string(rt));
            expr.type = Type::Error;
            return;
        }
        expr.type = Type::Boolean;
    } else if (expr.op.type == TokenType::Minus) {
        if (!is_numeric(rt)) {
            error(expr.line, expr.col, "unary '-' requires numeric operand, found " + to_string(rt));
            expr.type = Type::Error;
            return;
        }
        expr.type = rt;
    } else {
        error(expr.line, expr.col, "unexpected unary operator '" + std::string(expr.op.lexeme) + "'");
        expr.type = Type::Error;
    }
}

void SemanticAnalyzer::visit_binary(const BinaryExpr& expr) {
    expr.left->accept(*this);
    expr.right->accept(*this);

    Type lt = expr.left->type;
    Type rt = expr.right->type;

    if (lt == Type::Error || rt == Type::Error) {
        expr.type = Type::Error;
        return;
    }

    std::string op_lexeme(expr.op.lexeme);

    // Arithmetic: +, -, *, /
    if (expr.op.type == TokenType::Plus || expr.op.type == TokenType::Minus ||
        expr.op.type == TokenType::Star || expr.op.type == TokenType::Slash) {
        if (!is_numeric(lt) || !is_numeric(rt)) {
            error(expr.line, expr.col, "operator '" + op_lexeme + "' requires numeric operands, found " + to_string(lt) + " and " + to_string(rt));
            expr.type = Type::Error;
            return;
        }
        if (lt == Type::Decimal || rt == Type::Decimal) {
            expr.type = Type::Decimal;
        } else {
            expr.type = Type::Num;
        }
        return;
    }

    // Modulus: %
    if (expr.op.type == TokenType::Percent) {
        if (lt != Type::Num || rt != Type::Num) {
            error(expr.line, expr.col, "operator '%' requires num operands, found " + to_string(lt) + " and " + to_string(rt));
            expr.type = Type::Error;
            return;
        }
        expr.type = Type::Num;
        return;
    }

    // Comparison: <, >, <=, >=
    if (expr.op.type == TokenType::Less || expr.op.type == TokenType::Greater ||
        expr.op.type == TokenType::LessEqual || expr.op.type == TokenType::GreaterEqual) {
        if (!is_numeric(lt) || !is_numeric(rt)) {
            error(expr.line, expr.col, "comparison operator '" + op_lexeme + "' requires numeric operands, found " + to_string(lt) + " and " + to_string(rt));
            expr.type = Type::Error;
            return;
        }
        expr.type = Type::Boolean;
        return;
    }

    // Equality: equals / ==, notequals / !=
    if (expr.op.type == TokenType::Equals || expr.op.type == TokenType::Notequals) {
        if (lt == Type::List || rt == Type::List) {
            error(expr.line, expr.col, "cannot compare lists");
            expr.type = Type::Error;
            return;
        }
        bool compatible = (lt == rt) || (is_numeric(lt) && is_numeric(rt));
        if (!compatible) {
            error(expr.line, expr.col, "cannot compare incompatible types " + to_string(lt) + " and " + to_string(rt));
            expr.type = Type::Error;
            return;
        }
        expr.type = Type::Boolean;
        return;
    }

    // Logical: and, or
    if (expr.op.type == TokenType::And || expr.op.type == TokenType::Or) {
        if (lt != Type::Boolean || rt != Type::Boolean) {
            error(expr.line, expr.col, "logical operator '" + op_lexeme + "' requires boolean operands, found " + to_string(lt) + " and " + to_string(rt));
            expr.type = Type::Error;
            return;
        }
        expr.type = Type::Boolean;
        return;
    }

    error(expr.line, expr.col, "unsupported binary operator '" + op_lexeme + "'");
    expr.type = Type::Error;
}

void SemanticAnalyzer::visit_grouping(const GroupingExpr& expr) {
    expr.expr->accept(*this);
    expr.type = expr.expr->type;
    expr.is_const = expr.expr->is_const;
}

void SemanticAnalyzer::visit_input(const InputExpr& expr) {
    if (!input_allowed_) {
        error(expr.line, expr.col, "input() must be assigned directly to a variable");
        expr.type = Type::Error;
    } else {
        expr.type = input_target_type_;
    }
}

} // namespace cmm
