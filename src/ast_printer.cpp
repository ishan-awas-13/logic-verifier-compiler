#include "ast_printer.h"
#include <iostream>

namespace cmm {

void AstPrinter::print(const Program& program) {
    std::cout << "Program\n";
    indent_level++;
    for (const auto& imp : program.imports) {
        print_line("Import " + imp.name);
    }
    for (const auto& stmt : program.statements) {
        stmt->accept(*this);
    }
    indent_level--;
}

std::string AstPrinter::print_expr(const Expr& expr) {
    expr.accept(*this);
    return "";
}

void AstPrinter::indent() {
    for (int i = 0; i < indent_level * 2; ++i) {
        std::cout << " ";
    }
}

void AstPrinter::print_line(const std::string& line) {
    indent();
    std::cout << line << "\n";
}

void AstPrinter::visit_number(const NumberLiteral& expr) {
    if (expr.value == (int)expr.value) {
        print_line("Number " + std::to_string((int)expr.value));
    } else {
        print_line("Number " + std::to_string(expr.value));
    }
}

void AstPrinter::visit_string(const StringLiteral& expr) {
    print_line("String \"" + expr.value + "\"");
}

void AstPrinter::visit_char(const CharLiteral& expr) {
    std::string s = "Char '";
    s += expr.value;
    s += "'";
    print_line(s);
}

void AstPrinter::visit_bool(const BoolLiteral& expr) {
    print_line(std::string("Bool ") + (expr.value ? "true" : "false"));
}

void AstPrinter::visit_variable(const VariableExpr& expr) {
    print_line("Variable " + expr.name);
}

void AstPrinter::visit_unary(const UnaryExpr& expr) {
    print_line("Unary " + std::string(expr.op.lexeme));
    indent_level++;
    expr.right->accept(*this);
    indent_level--;
}

void AstPrinter::visit_binary(const BinaryExpr& expr) {
    print_line("Binary " + std::string(expr.op.lexeme));
    indent_level++;
    expr.left->accept(*this);
    expr.right->accept(*this);
    indent_level--;
}

void AstPrinter::visit_grouping(const GroupingExpr& expr) {
    print_line("Grouping");
    indent_level++;
    expr.expr->accept(*this);
    indent_level--;
}

void AstPrinter::visit_input(const InputExpr&) {
    print_line("Input");
}

void AstPrinter::visit_call(const CallExpr& expr) {
    print_line("Call " + expr.callee);
    indent_level++;
    for (const auto& arg : expr.args) {
        arg->accept(*this);
    }
    indent_level--;
}

void AstPrinter::visit_var_decl(const VarDecl& stmt) {
    std::string prefix = stmt.is_fixed ? "Fixed VarDecl " : "VarDecl ";
    print_line(prefix + stmt.type_name + " " + stmt.name);
    if (stmt.initializer) {
        indent_level++;
        stmt.initializer->accept(*this);
        indent_level--;
    }
}

void AstPrinter::visit_assign(const AssignStmt& stmt) {
    print_line("Assign " + stmt.name);
    indent_level++;
    stmt.value->accept(*this);
    indent_level--;
}

void AstPrinter::visit_emit(const EmitStmt& stmt) {
    print_line("Show");
    indent_level++;
    stmt.expr->accept(*this);
    indent_level--;
}

void AstPrinter::visit_if(const IfStmt& stmt) {
    print_line("Check");
    indent_level++;
    stmt.condition->accept(*this);
    stmt.then_branch->accept(*this);
    if (stmt.else_branch) {
        print_line("Otherwise");
        stmt.else_branch->accept(*this);
    }
    indent_level--;
}

void AstPrinter::visit_while(const WhileStmt& stmt) {
    print_line("Until");
    indent_level++;
    stmt.condition->accept(*this);
    stmt.body->accept(*this);
    indent_level--;
}

void AstPrinter::visit_block(const BlockStmt& stmt) {
    print_line("Block");
    indent_level++;
    for (const auto& s : stmt.statements) {
        s->accept(*this);
    }
    indent_level--;
}

void AstPrinter::visit_give(const GiveStmt& stmt) {
    print_line("Give");
    if (stmt.value) {
        indent_level++;
        stmt.value->accept(*this);
        indent_level--;
    }
}

void AstPrinter::visit_stop(const StopStmt&) {
    print_line("Stop");
}

void AstPrinter::visit_skip(const SkipStmt&) {
    print_line("Skip");
}

} // namespace cmm
