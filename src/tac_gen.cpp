#include "tac_gen.h"

namespace cmm {

TACProgram TACGenerator::generate(const Program& program) {
    program_ = TACProgram();
    decl_counts_.clear();
    scopes_.clear();
    loop_stack_.clear();

    scopes_.push_back({}); // global scope

    for (const auto& stmt : program.statements) {
        if (stmt) {
            stmt->accept(*this);
        }
    }

    program_.emit(TACInstr::make_halt());

    scopes_.pop_back();
    return program_;
}

std::string TACGenerator::declare_var(const std::string& name, Type type) {
    int count = decl_counts_[name]++;
    std::string tac_name;
    if (count == 0) {
        tac_name = name;
    } else {
        tac_name = name + "." + std::to_string(count);
    }
    scopes_.back()[name] = {tac_name, type};
    return tac_name;
}

TACGenerator::SymbolInfo TACGenerator::lookup_var(const std::string& name) {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto f = it->find(name);
        if (f != it->end()) {
            return f->second;
        }
    }
    return {name, Type::Error};
}

void TACGenerator::visit_var_decl(const VarDecl& stmt) {
    Type decl_type = type_from_string(stmt.type_name);
    std::string tac_name = declare_var(stmt.name, decl_type);
    TACOperand dest = TACOperand::make_var(tac_name, decl_type);

    if (stmt.initializer) {
        if (dynamic_cast<const InputExpr*>(stmt.initializer.get())) {
            program_.emit(TACInstr::make_input(dest, decl_type, stmt.line, stmt.col));
        } else {
            stmt.initializer->accept(*this);
            TACOperand val = current_operand_;
            if (decl_type == Type::Decimal && val.type == Type::Num) {
                TACOperand t = program_.new_temp(Type::Decimal);
                program_.emit(TACInstr::make_i2d(t, val, stmt.initializer->line, stmt.initializer->col));
                val = t;
            }
            program_.emit(TACInstr::make_copy(dest, val, stmt.line, stmt.col));
        }
    } else {
        if (decl_type == Type::Num) {
            program_.emit(TACInstr::make_copy(dest, TACOperand::make_num(0), stmt.line, stmt.col));
        } else if (decl_type == Type::Decimal) {
            program_.emit(TACInstr::make_copy(dest, TACOperand::make_decimal(0.0), stmt.line, stmt.col));
        } else if (decl_type == Type::Word) {
            program_.emit(TACInstr::make_copy(dest, TACOperand::make_word(""), stmt.line, stmt.col));
        } else if (decl_type == Type::Letter) {
            program_.emit(TACInstr::make_copy(dest, TACOperand::make_letter('\0'), stmt.line, stmt.col));
        } else if (decl_type == Type::Boolean) {
            program_.emit(TACInstr::make_copy(dest, TACOperand::make_bool(false), stmt.line, stmt.col));
        }
    }
}

void TACGenerator::visit_assign(const AssignStmt& stmt) {
    SymbolInfo sym = lookup_var(stmt.name);
    TACOperand dest = TACOperand::make_var(sym.tac_name, sym.type);

    if (stmt.kind == AssignKind::Increment) {
        if (sym.type == Type::Num) {
            program_.emit(TACInstr::make_binary(TACOp::Add, Type::Num, dest, dest, TACOperand::make_num(1), stmt.line, stmt.col));
        } else {
            program_.emit(TACInstr::make_binary(TACOp::Add, Type::Decimal, dest, dest, TACOperand::make_decimal(1.0), stmt.line, stmt.col));
        }
    } else if (stmt.kind == AssignKind::Decrement) {
        if (sym.type == Type::Num) {
            program_.emit(TACInstr::make_binary(TACOp::Sub, Type::Num, dest, dest, TACOperand::make_num(1), stmt.line, stmt.col));
        } else {
            program_.emit(TACInstr::make_binary(TACOp::Sub, Type::Decimal, dest, dest, TACOperand::make_decimal(1.0), stmt.line, stmt.col));
        }
    } else {
        if (dynamic_cast<const InputExpr*>(stmt.value.get())) {
            program_.emit(TACInstr::make_input(dest, sym.type, stmt.line, stmt.col));
        } else {
            stmt.value->accept(*this);
            TACOperand val = current_operand_;
            if (sym.type == Type::Decimal && val.type == Type::Num) {
                TACOperand t = program_.new_temp(Type::Decimal);
                program_.emit(TACInstr::make_i2d(t, val, stmt.value->line, stmt.value->col));
                val = t;
            }
            program_.emit(TACInstr::make_copy(dest, val, stmt.line, stmt.col));
        }
    }
}

void TACGenerator::visit_emit(const EmitStmt& stmt) {
    stmt.expr->accept(*this);
    TACOperand val = current_operand_;
    program_.emit(TACInstr::make_show(val, stmt.line, stmt.col));
}

void TACGenerator::visit_if(const IfStmt& stmt) {
    stmt.condition->accept(*this);
    TACOperand cond = current_operand_;

    if (stmt.else_branch) {
        std::string label_else = program_.new_label();
        std::string label_end = program_.new_label();

        auto br = TACInstr::make_if_false(cond, label_else, stmt.line, stmt.col);
        br.origin = TACInstr::Origin::Check;
        br.origin_line = stmt.condition ? stmt.condition->line : stmt.line;
        br.origin_col = stmt.condition ? stmt.condition->col : stmt.col;
        program_.emit(br);
        if (stmt.then_branch) stmt.then_branch->accept(*this);
        program_.emit(TACInstr::make_goto(label_end, stmt.line, stmt.col));
        program_.emit(TACInstr::make_label(label_else, stmt.line, stmt.col));
        stmt.else_branch->accept(*this);
        program_.emit(TACInstr::make_label(label_end, stmt.line, stmt.col));
    } else {
        std::string label_end = program_.new_label();

        auto br = TACInstr::make_if_false(cond, label_end, stmt.line, stmt.col);
        br.origin = TACInstr::Origin::Check;
        br.origin_line = stmt.condition ? stmt.condition->line : stmt.line;
        br.origin_col = stmt.condition ? stmt.condition->col : stmt.col;
        program_.emit(br);
        if (stmt.then_branch) stmt.then_branch->accept(*this);
        program_.emit(TACInstr::make_label(label_end, stmt.line, stmt.col));
    }
}

void TACGenerator::visit_while(const WhileStmt& stmt) {
    std::string label_start = program_.new_label();
    std::string label_end = program_.new_label();

    loop_stack_.push_back({label_start, label_end});

    program_.emit(TACInstr::make_label(label_start, stmt.line, stmt.col));
    stmt.condition->accept(*this);
    TACOperand cond = current_operand_;
    auto br = TACInstr::make_if_true(cond, label_end, stmt.line, stmt.col);
    br.origin = TACInstr::Origin::Until;
    br.origin_line = stmt.condition ? stmt.condition->line : stmt.line;
    br.origin_col = stmt.condition ? stmt.condition->col : stmt.col;
    program_.emit(br);

    if (stmt.body) {
        stmt.body->accept(*this);
    }

    program_.emit(TACInstr::make_goto(label_start, stmt.line, stmt.col));
    program_.emit(TACInstr::make_label(label_end, stmt.line, stmt.col));

    loop_stack_.pop_back();
}

void TACGenerator::visit_block(const BlockStmt& stmt) {
    scopes_.push_back({});
    for (const auto& s : stmt.statements) {
        if (s) {
            s->accept(*this);
        }
    }
    scopes_.pop_back();
}

void TACGenerator::visit_give(const GiveStmt& stmt) {
    stmt.value->accept(*this);
    TACOperand val = current_operand_;
    program_.emit(TACInstr::make_give(val, stmt.line, stmt.col));
}

void TACGenerator::visit_stop(const StopStmt& stmt) {
    auto ins = TACInstr::make_goto(loop_stack_.back().end_label, stmt.line, stmt.col);
    ins.origin = TACInstr::Origin::Stop;
    program_.emit(ins);
}

void TACGenerator::visit_skip(const SkipStmt& stmt) {
    auto ins = TACInstr::make_goto(loop_stack_.back().start_label, stmt.line, stmt.col);
    ins.origin = TACInstr::Origin::Skip;
    program_.emit(ins);
}

void TACGenerator::visit_number(const NumberLiteral& expr) {
    if (expr.is_decimal) {
        current_operand_ = TACOperand::make_decimal(expr.value);
    } else {
        current_operand_ = TACOperand::make_num((int64_t)expr.value);
    }
}

void TACGenerator::visit_string(const StringLiteral& expr) {
    current_operand_ = TACOperand::make_word(expr.value);
}

void TACGenerator::visit_char(const CharLiteral& expr) {
    current_operand_ = TACOperand::make_letter(expr.value);
}

void TACGenerator::visit_bool(const BoolLiteral& expr) {
    current_operand_ = TACOperand::make_bool(expr.value);
}

void TACGenerator::visit_variable(const VariableExpr& expr) {
    SymbolInfo sym = lookup_var(expr.name);
    current_operand_ = TACOperand::make_var(sym.tac_name, sym.type);
}

void TACGenerator::visit_unary(const UnaryExpr& expr) {
    expr.right->accept(*this);
    TACOperand arg = current_operand_;
    if (expr.op.type == TokenType::Minus) {
        TACOperand t = program_.new_temp(arg.type);
        program_.emit(TACInstr::make_neg(t, arg, expr.op.line, expr.op.col));
        current_operand_ = t;
    } else if (expr.op.type == TokenType::Not) {
        TACOperand t = program_.new_temp(Type::Boolean);
        program_.emit(TACInstr::make_not(t, arg, expr.op.line, expr.op.col));
        current_operand_ = t;
    }
}

void TACGenerator::visit_binary(const BinaryExpr& expr) {
    if (expr.op.type == TokenType::And) {
        expr.left->accept(*this);
        TACOperand op_a = current_operand_;
        TACOperand t = program_.new_temp(Type::Boolean);
        program_.emit(TACInstr::make_copy(t, op_a, expr.left->line, expr.left->col));
        std::string l_sc = program_.new_label();
        auto br = TACInstr::make_if_false(t, l_sc, expr.op.line, expr.op.col);
        br.origin = TACInstr::Origin::And;
        br.origin_line = expr.op.line;
        br.origin_col = expr.op.col;
        program_.emit(br);
        expr.right->accept(*this);
        TACOperand op_b = current_operand_;
        program_.emit(TACInstr::make_copy(t, op_b, expr.right->line, expr.right->col));
        program_.emit(TACInstr::make_label(l_sc, expr.op.line, expr.op.col));
        current_operand_ = t;
        return;
    }

    if (expr.op.type == TokenType::Or) {
        expr.left->accept(*this);
        TACOperand op_a = current_operand_;
        TACOperand t = program_.new_temp(Type::Boolean);
        program_.emit(TACInstr::make_copy(t, op_a, expr.left->line, expr.left->col));
        std::string l_sc = program_.new_label();
        auto br = TACInstr::make_if_true(t, l_sc, expr.op.line, expr.op.col);
        br.origin = TACInstr::Origin::Or;
        br.origin_line = expr.op.line;
        br.origin_col = expr.op.col;
        program_.emit(br);
        expr.right->accept(*this);
        TACOperand op_b = current_operand_;
        program_.emit(TACInstr::make_copy(t, op_b, expr.right->line, expr.right->col));
        program_.emit(TACInstr::make_label(l_sc, expr.op.line, expr.op.col));
        current_operand_ = t;
        return;
    }

    expr.left->accept(*this);
    TACOperand op_a = current_operand_;
    expr.right->accept(*this);
    TACOperand op_b = current_operand_;

    if (op_a.type == Type::Num && op_b.type == Type::Decimal) {
        TACOperand t = program_.new_temp(Type::Decimal);
        program_.emit(TACInstr::make_i2d(t, op_a, expr.left->line, expr.left->col));
        op_a = t;
    } else if (op_a.type == Type::Decimal && op_b.type == Type::Num) {
        TACOperand t = program_.new_temp(Type::Decimal);
        program_.emit(TACInstr::make_i2d(t, op_b, expr.right->line, expr.right->col));
        op_b = t;
    }

    TACOp op;
    switch (expr.op.type) {
        case TokenType::Plus: op = TACOp::Add; break;
        case TokenType::Minus: op = TACOp::Sub; break;
        case TokenType::Star: op = TACOp::Mul; break;
        case TokenType::Slash: op = TACOp::Div; break;
        case TokenType::Percent: op = TACOp::Mod; break;
        case TokenType::Less: op = TACOp::Less; break;
        case TokenType::Greater: op = TACOp::Greater; break;
        case TokenType::LessEqual: op = TACOp::LessEqual; break;
        case TokenType::GreaterEqual: op = TACOp::GreaterEqual; break;
        case TokenType::Equals: op = TACOp::Equal; break;
        case TokenType::Notequals: op = TACOp::NotEqual; break;
        default: op = TACOp::Add; break;
    }

    Type res_type = expr.type;
    Type op_type = op_a.type;
    TACOperand t = program_.new_temp(res_type);
    program_.emit(TACInstr::make_binary(op, op_type, t, op_a, op_b, expr.op.line, expr.op.col));
    current_operand_ = t;
}

void TACGenerator::visit_grouping(const GroupingExpr& expr) {
    expr.expr->accept(*this);
}

void TACGenerator::visit_input(const InputExpr& expr) {
    TACOperand t = program_.new_temp(expr.type);
    program_.emit(TACInstr::make_input(t, expr.type, expr.line, expr.col));
    current_operand_ = t;
}

void TACGenerator::visit_call(const CallExpr& expr) {
    TACOperand a1 = TACOperand::none();
    TACOperand a2 = TACOperand::none();

    if (expr.args.size() >= 1) {
        expr.args[0]->accept(*this);
        a1 = current_operand_;
    }
    if (expr.args.size() >= 2) {
        expr.args[1]->accept(*this);
        a2 = current_operand_;
    }

    TACOperand dest = program_.new_temp(expr.type);
    program_.emit(TACInstr::make_call(dest, expr.callee, std::move(a1), std::move(a2), expr.line, expr.col));
    current_operand_ = dest;
}

} // namespace cmm
