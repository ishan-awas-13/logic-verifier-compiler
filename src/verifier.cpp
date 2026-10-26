#include "verifier.h"
#include <queue>
#include <algorithm>
#include <set>
#include <cmath>

namespace cmm {

AbstractValue LogicVerifier::eval_operand(const TACOperand& op, const AbstractState& state) {
    if (op.kind == TACOperand::Kind::Constant) {
        if (op.type == Type::Num) {
            return AbstractValue::make_num(op.num_val, false);
        } else if (op.type == Type::Decimal) {
            return AbstractValue::make_decimal(op.dec_val, false);
        } else if (op.type == Type::Boolean) {
            return AbstractValue::make_bool(op.bool_val ? TriBool::True : TriBool::False, false);
        } else if (op.type == Type::Letter) {
            return AbstractValue::make_letter(op.letter_val, false);
        } else {
            return AbstractValue::make_unknown(op.type, false);
        }
    } else if (op.kind == TACOperand::Kind::Variable || op.kind == TACOperand::Kind::Temp) {
        return state.get(op.name);
    }
    return AbstractValue::make_unknown(op.type, false);
}

void LogicVerifier::report(const std::string& code, FindingSeverity sev, int line, int col, const std::string& msg) {
    findings_.push_back({code, sev, line, col, msg});
}

static std::string op_to_str(TACOp op) {
    switch (op) {
        case TACOp::Equal: return "==";
        case TACOp::NotEqual: return "!=";
        case TACOp::Less: return "<";
        case TACOp::LessEqual: return "<=";
        case TACOp::Greater: return ">";
        case TACOp::GreaterEqual: return ">=";
        case TACOp::Add: return "+";
        case TACOp::Sub: return "-";
        case TACOp::Mul: return "*";
        case TACOp::Div: return "/";
        case TACOp::Mod: return "%";
        default: return "?";
    }
}

static const TACInstr* find_def_in_block(const BasicBlock& block, const std::string& name) {
    for (int i = (int)block.instructions.size() - 1; i >= 0; --i) {
        if (block.instructions[i]->result.name == name) {
            return block.instructions[i];
        }
    }
    return nullptr;
}

static const TACInstr* find_def_global(const CFG& cfg, const std::string& name) {
    for (const auto& b : cfg.blocks) {
        for (const auto* ins : b.instructions) {
            if (ins->result.name == name) {
                return ins;
            }
        }
    }
    return nullptr;
}

static std::string format_expr_instr(const TACInstr& ins, const CFG& cfg) {
    if (ins.op == TACOp::Not) {
        return "not " + ins.arg1.to_string();
    }
    if (ins.op == TACOp::Equal || ins.op == TACOp::NotEqual ||
        ins.op == TACOp::Less || ins.op == TACOp::LessEqual ||
        ins.op == TACOp::Greater || ins.op == TACOp::GreaterEqual) {
        return ins.arg1.to_string() + " " + op_to_str(ins.op) + " " + ins.arg2.to_string();
    }
    if (ins.op == TACOp::Copy) {
        if (ins.arg1.kind == TACOperand::Kind::Temp) {
            const TACInstr* prev = find_def_global(cfg, ins.arg1.name);
            if (prev) return format_expr_instr(*prev, cfg);
        }
        return ins.arg1.to_string();
    }
    return ins.to_string();
}

std::string LogicVerifier::format_proof(
    const std::string& cond_var,
    const BasicBlock& block,
    const AbstractState& cur_st,
    bool is_tautology) {

    // Trace defining instruction for cond_var in the current block or globally
    const TACInstr* def = find_def_in_block(block, cond_var);
    if (!def) def = find_def_global(cfg_, cond_var);

    bool inverted = false;
    while (def && def->op == TACOp::Copy && def->arg1.kind == TACOperand::Kind::Temp) {
        def = find_def_global(cfg_, def->arg1.name);
    }
    if (def && def->op == TACOp::Not) {
        inverted = true;
        if (def->arg1.kind == TACOperand::Kind::Temp) {
            def = find_def_global(cfg_, def->arg1.name);
        }
    }

    // Check if cond_var is directly a boolean variable or expression
    std::string cond_expr_str = "";
    if (def) {
        cond_expr_str = format_expr_instr(*def, cfg_);
    } else {
        cond_expr_str = cond_var;
    }
    if (inverted && !cond_expr_str.empty()) {
        cond_expr_str = "not " + cond_expr_str;
    }

    if (!is_tautology) {
        // [LV01] Condition is always false

        // Check if predecessor was a short-circuit branch (like in 'x > 10 and x < 5')
        for (int p : block.preds) {
            const auto& pb = cfg_.blocks[p];
            const TACInstr* p_last = pb.instructions.empty() ? nullptr : pb.instructions.back();
            if (p_last && p_last->origin == TACInstr::Origin::And) {
                // p_last is 'if_false t2 goto L1' in pb
                const TACInstr* p_cmp = nullptr;
                for (int i = (int)pb.instructions.size() - 1; i >= 0; --i) {
                    if (pb.instructions[i]->op == TACOp::Equal || pb.instructions[i]->op == TACOp::NotEqual ||
                        pb.instructions[i]->op == TACOp::Less || pb.instructions[i]->op == TACOp::LessEqual ||
                        pb.instructions[i]->op == TACOp::Greater || pb.instructions[i]->op == TACOp::GreaterEqual) {
                        p_cmp = pb.instructions[i];
                        break;
                    }
                }
                // Right comparison is in block's other predecessor or in the chain
                const TACInstr* r_cmp = nullptr;
                for (int p2 : block.preds) {
                    if (p2 != p) {
                        const auto& p2b = cfg_.blocks[p2];
                        for (int i = (int)p2b.instructions.size() - 1; i >= 0; --i) {
                            if (p2b.instructions[i]->op == TACOp::Equal || p2b.instructions[i]->op == TACOp::NotEqual ||
                                p2b.instructions[i]->op == TACOp::Less || p2b.instructions[i]->op == TACOp::LessEqual ||
                                p2b.instructions[i]->op == TACOp::Greater || p2b.instructions[i]->op == TACOp::GreaterEqual) {
                                r_cmp = p2b.instructions[i];
                                break;
                            }
                        }
                    }
                }
                if (p_cmp && r_cmp) {
                    std::string c1 = format_expr_instr(*p_cmp, cfg_);
                    std::string c2 = format_expr_instr(*r_cmp, cfg_);
                    return c1 + " contradicts " + c2;
                }
            }
        }

        // Check if an operand in def is a known singleton variable
        if (def) {
            if (def->op == TACOp::Not) {
                std::string var = def->arg1.name;
                AbstractValue v = cur_st.get(var);
                if (v.tri_bool != TriBool::Unknown) {
                    return "'" + var + "' is always " + to_string(v.tri_bool) + " here, so not " + var + " is false";
                }
            }
            if (def->arg1.kind == TACOperand::Kind::Variable) {
                std::string var = def->arg1.name;
                AbstractValue v = cur_st.get(var);
                if (v.interval.is_singleton()) {
                    std::string val_str = v.interval.to_string();
                    return "'" + var + "' is always " + val_str + " here, so " + cond_expr_str + " is false";
                }
            }
        }

        // If cond_var itself is a boolean variable
        AbstractValue cv = cur_st.get(cond_var);
        if (cv.type == Type::Boolean && cv.tri_bool != TriBool::Unknown) {
            return "'" + cond_var + "' is always " + to_string(cv.tri_bool) + " here, so condition is false";
        }

        return cond_expr_str;
    } else {
        // [LV02] Condition is always true

        // Check if this tautology is from an 'or' short-circuit covering every value (e.g. x > 5 or x <= 5)
        for (int p : block.preds) {
            const auto& pb = cfg_.blocks[p];
            const TACInstr* p_last = pb.instructions.empty() ? nullptr : pb.instructions.back();
            if (p_last && p_last->origin == TACInstr::Origin::Or) {
                // Left side comparison in pb
                const TACInstr* p_cmp = nullptr;
                for (int i = (int)pb.instructions.size() - 1; i >= 0; --i) {
                    if (pb.instructions[i]->op == TACOp::Equal || pb.instructions[i]->op == TACOp::NotEqual ||
                        pb.instructions[i]->op == TACOp::Less || pb.instructions[i]->op == TACOp::LessEqual ||
                        pb.instructions[i]->op == TACOp::Greater || pb.instructions[i]->op == TACOp::GreaterEqual) {
                        p_cmp = pb.instructions[i];
                        break;
                    }
                }
                const TACInstr* r_cmp = nullptr;
                for (int p2 : block.preds) {
                    if (p2 != p) {
                        const auto& p2b = cfg_.blocks[p2];
                        for (int i = (int)p2b.instructions.size() - 1; i >= 0; --i) {
                            if (p2b.instructions[i]->op == TACOp::Equal || p2b.instructions[i]->op == TACOp::NotEqual ||
                                p2b.instructions[i]->op == TACOp::Less || p2b.instructions[i]->op == TACOp::LessEqual ||
                                p2b.instructions[i]->op == TACOp::Greater || p2b.instructions[i]->op == TACOp::GreaterEqual) {
                                r_cmp = p2b.instructions[i];
                                break;
                            }
                        }
                    }
                }
                if (p_cmp && r_cmp) {
                    std::string c1 = format_expr_instr(*p_cmp, cfg_);
                    std::string c2 = format_expr_instr(*r_cmp, cfg_);
                    return c1 + " or " + c2 + " covers every value";
                }
            }
        }

        // Check if an operand in def is a known singleton variable
        if (def) {
            if (def->arg1.kind == TACOperand::Kind::Variable) {
                std::string var = def->arg1.name;
                AbstractValue v = cur_st.get(var);
                if (v.interval.is_singleton()) {
                    return "'" + var + "' is always " + v.interval.to_string() + " here, so " + cond_expr_str + " is true";
                }
                if (v.type == Type::Boolean && v.tri_bool != TriBool::Unknown) {
                    return "'" + var + "' is always " + to_string(v.tri_bool) + " here, so " + cond_expr_str + " is true";
                }
            }
        }

        return cond_expr_str + " covers every value";
    }
}

// Branch refinement: returns {true_state, false_state}
static std::pair<AbstractState, AbstractState> refine_branch(
    const BasicBlock& block,
    const AbstractState& out_state,
    const TACInstr& br_instr) {

    AbstractState true_state = out_state;
    AbstractState false_state = out_state;

    // The condition operand is br_instr.arg1
    std::string cond_name = br_instr.arg1.name;
    AbstractValue cond_val = out_state.get(cond_name);

    if (cond_val.type == Type::Boolean) {
        if (cond_val.tri_bool == TriBool::True) {
            false_state = AbstractState::bottom();
        } else if (cond_val.tri_bool == TriBool::False) {
            true_state = AbstractState::bottom();
        }
    }

    // Try to find the defining instruction of cond_name in the same block (or follow copy chains)
    std::string cur_name = cond_name;
    bool inverted = false;

    const TACInstr* def_instr = nullptr;
    for (int i = (int)block.instructions.size() - 2; i >= 0; --i) {
        const auto* ins = block.instructions[i];
        if (ins->result.name == cur_name) {
            if (ins->op == TACOp::Copy && (ins->arg1.kind == TACOperand::Kind::Variable || ins->arg1.kind == TACOperand::Kind::Temp)) {
                cur_name = ins->arg1.name;
                continue;
            } else if (ins->op == TACOp::Not && (ins->arg1.kind == TACOperand::Kind::Variable || ins->arg1.kind == TACOperand::Kind::Temp)) {
                cur_name = ins->arg1.name;
                inverted = !inverted;
                continue;
            } else {
                def_instr = ins;
                break;
            }
        }
    }

    if (def_instr) {
        // If def_instr is a comparison: t = var OP const
        auto is_comp = [](TACOp op) {
            return op == TACOp::Equal || op == TACOp::NotEqual ||
                   op == TACOp::Less || op == TACOp::LessEqual ||
                   op == TACOp::Greater || op == TACOp::GreaterEqual;
        };

        if (is_comp(def_instr->op)) {
            std::string var_name;
            Interval const_iv;
            bool found_pattern = false;

            if (def_instr->arg1.kind == TACOperand::Kind::Variable && def_instr->arg2.kind == TACOperand::Kind::Constant) {
                var_name = def_instr->arg1.name;
                const_iv = (def_instr->arg2.type == Type::Num) 
                    ? Interval::point(def_instr->arg2.num_val, true)
                    : Interval::point(def_instr->arg2.dec_val, false);
                found_pattern = true;
            } else if (def_instr->arg1.kind == TACOperand::Kind::Variable && (def_instr->arg2.kind == TACOperand::Kind::Variable || def_instr->arg2.kind == TACOperand::Kind::Temp)) {
                AbstractValue v2 = out_state.get(def_instr->arg2.name);
                if (v2.interval.is_singleton()) {
                    var_name = def_instr->arg1.name;
                    const_iv = v2.interval;
                    found_pattern = true;
                }
            }

            if (found_pattern) {
                AbstractValue cur_var_val = out_state.get(var_name);
                Interval iv = cur_var_val.interval;
                bool is_num = iv.is_num;

                Interval t_iv = iv;
                Interval f_iv = iv;
                double k = const_iv.lo;

                TACOp eff_op = def_instr->op;
                if (inverted) {
                    if (eff_op == TACOp::Equal) eff_op = TACOp::NotEqual;
                    else if (eff_op == TACOp::NotEqual) eff_op = TACOp::Equal;
                    else if (eff_op == TACOp::Less) eff_op = TACOp::GreaterEqual;
                    else if (eff_op == TACOp::LessEqual) eff_op = TACOp::Greater;
                    else if (eff_op == TACOp::Greater) eff_op = TACOp::LessEqual;
                    else if (eff_op == TACOp::GreaterEqual) eff_op = TACOp::Less;
                }

                switch (eff_op) {
                    case TACOp::Equal:
                        t_iv = t_iv.meet(Interval::point(k, is_num));
                        break;
                    case TACOp::NotEqual:
                        f_iv = f_iv.meet(Interval::point(k, is_num));
                        break;
                    case TACOp::Greater:
                        t_iv = t_iv.meet(Interval(is_num ? (k + 1) : k, Interval::INF, is_num));
                        f_iv = f_iv.meet(Interval(-Interval::INF, k, is_num));
                        break;
                    case TACOp::GreaterEqual:
                        t_iv = t_iv.meet(Interval(k, Interval::INF, is_num));
                        f_iv = f_iv.meet(Interval(-Interval::INF, is_num ? (k - 1) : k, is_num));
                        break;
                    case TACOp::Less:
                        t_iv = t_iv.meet(Interval(-Interval::INF, is_num ? (k - 1) : k, is_num));
                        f_iv = f_iv.meet(Interval(k, Interval::INF, is_num));
                        break;
                    case TACOp::LessEqual:
                        t_iv = t_iv.meet(Interval(-Interval::INF, k, is_num));
                        f_iv = f_iv.meet(Interval(is_num ? (k + 1) : k, Interval::INF, is_num));
                        break;
                    default:
                        break;
                }

                if (t_iv.is_empty) {
                    true_state = AbstractState::bottom();
                } else {
                    cur_var_val.interval = t_iv;
                    true_state.set(var_name, cur_var_val);
                    true_state.set(cond_name, AbstractValue::make_bool(TriBool::True, false));
                }

                if (f_iv.is_empty) {
                    false_state = AbstractState::bottom();
                } else {
                    cur_var_val.interval = f_iv;
                    false_state.set(var_name, cur_var_val);
                    false_state.set(cond_name, AbstractValue::make_bool(TriBool::False, false));
                }
            }
        }
    } else {
        // If cond_name is a boolean variable, narrow it directly
        AbstractValue cur_val = out_state.get(cur_name);
        if (cur_val.type == Type::Boolean) {
            AbstractValue tv = cur_val;
            tv.tri_bool = inverted ? TriBool::False : TriBool::True;
            true_state.set(cur_name, tv);
            true_state.set(cond_name, AbstractValue::make_bool(TriBool::True, false));

            AbstractValue fv = cur_val;
            fv.tri_bool = inverted ? TriBool::True : TriBool::False;
            false_state.set(cur_name, fv);
            false_state.set(cond_name, AbstractValue::make_bool(TriBool::False, false));
        }
    }

    return {true_state, false_state};
}

AbstractState LogicVerifier::transfer(const BasicBlock& block, const AbstractState& in_state) {
    if (in_state.is_bottom) return AbstractState::bottom();
    AbstractState st = in_state;
    for (const auto* ins : block.instructions) {
        st = transfer_instr(*ins, st);
    }
    return st;
}

AbstractState LogicVerifier::transfer_instr(const TACInstr& instr, const AbstractState& in_state) {
    if (in_state.is_bottom) return AbstractState::bottom();
    AbstractState st = in_state;
    const TACInstr* ins = &instr;
    switch (ins->op) {
        case TACOp::Copy: {
            AbstractValue val = eval_operand(ins->arg1, st);
            st.set(ins->result.name, val);
            break;
        }
            case TACOp::Add: {
                AbstractValue a1 = eval_operand(ins->arg1, st);
                AbstractValue a2 = eval_operand(ins->arg2, st);
                Interval iv = a1.interval.add(a2.interval);
                bool tainted = a1.from_input || a2.from_input;
                if (ins->operand_type == Type::Decimal) {
                    st.set(ins->result.name, AbstractValue::make_decimal_interval(iv, tainted));
                } else {
                    st.set(ins->result.name, AbstractValue::make_num_interval(iv, tainted));
                }
                break;
            }
            case TACOp::Sub: {
                AbstractValue a1 = eval_operand(ins->arg1, st);
                AbstractValue a2 = eval_operand(ins->arg2, st);
                Interval iv = a1.interval.sub(a2.interval);
                bool tainted = a1.from_input || a2.from_input;
                if (ins->operand_type == Type::Decimal) {
                    st.set(ins->result.name, AbstractValue::make_decimal_interval(iv, tainted));
                } else {
                    st.set(ins->result.name, AbstractValue::make_num_interval(iv, tainted));
                }
                break;
            }
            case TACOp::Mul: {
                AbstractValue a1 = eval_operand(ins->arg1, st);
                AbstractValue a2 = eval_operand(ins->arg2, st);
                Interval iv = a1.interval.mul(a2.interval);
                bool tainted = a1.from_input || a2.from_input;
                if (ins->operand_type == Type::Decimal) {
                    st.set(ins->result.name, AbstractValue::make_decimal_interval(iv, tainted));
                } else {
                    st.set(ins->result.name, AbstractValue::make_num_interval(iv, tainted));
                }
                break;
            }
            case TACOp::Div: {
                AbstractValue a1 = eval_operand(ins->arg1, st);
                AbstractValue a2 = eval_operand(ins->arg2, st);
                Interval iv = a1.interval.div(a2.interval);
                bool tainted = a1.from_input || a2.from_input;
                if (ins->operand_type == Type::Decimal) {
                    st.set(ins->result.name, AbstractValue::make_decimal_interval(iv, tainted));
                } else {
                    st.set(ins->result.name, AbstractValue::make_num_interval(iv, tainted));
                }
                break;
            }
            case TACOp::Mod: {
                AbstractValue a1 = eval_operand(ins->arg1, st);
                AbstractValue a2 = eval_operand(ins->arg2, st);
                Interval iv = a1.interval.mod(a2.interval);
                bool tainted = a1.from_input || a2.from_input;
                st.set(ins->result.name, AbstractValue::make_num_interval(iv, tainted));
                break;
            }
            case TACOp::Neg: {
                AbstractValue a1 = eval_operand(ins->arg1, st);
                Interval iv = a1.interval.negate();
                if (ins->operand_type == Type::Decimal) {
                    st.set(ins->result.name, AbstractValue::make_decimal_interval(iv, a1.from_input));
                } else {
                    st.set(ins->result.name, AbstractValue::make_num_interval(iv, a1.from_input));
                }
                break;
            }
            case TACOp::Not: {
                AbstractValue a1 = eval_operand(ins->arg1, st);
                TriBool tb = tri_not(a1.tri_bool);
                st.set(ins->result.name, AbstractValue::make_bool(tb, a1.from_input));
                break;
            }
            case TACOp::I2D: {
                AbstractValue a1 = eval_operand(ins->arg1, st);
                Interval iv(a1.interval.lo, a1.interval.hi, false, a1.interval.is_empty);
                st.set(ins->result.name, AbstractValue::make_decimal_interval(iv, a1.from_input));
                break;
            }
            case TACOp::Input: {
                Type t = ins->operand_type;
                AbstractValue v = AbstractValue::make_unknown(t, true);
                st.set(ins->result.name, v);
                break;
            }
            case TACOp::Call: {
                AbstractValue a1 = eval_operand(ins->arg1, st);
                AbstractValue a2 = eval_operand(ins->arg2, st);
                bool tainted = a1.from_input || a2.from_input;
                const std::string& name = ins->label;

                if (name == "length") {
                    Interval iv(0.0, Interval::INF, true, false);
                    st.set(ins->result.name, AbstractValue::make_num_interval(iv, tainted));
                } else if (name == "absolute") {
                    double lo = 0.0;
                    double hi = Interval::INF;
                    if (!a1.interval.is_empty) {
                        if (a1.interval.lo >= 0) {
                            lo = a1.interval.lo;
                            hi = a1.interval.hi;
                        } else if (a1.interval.hi <= 0) {
                            lo = (a1.interval.hi == -Interval::INF) ? 0 : -a1.interval.hi;
                            hi = (a1.interval.lo == -Interval::INF) ? Interval::INF : -a1.interval.lo;
                        } else {
                            lo = 0.0;
                            double m1 = (a1.interval.lo == -Interval::INF) ? Interval::INF : -a1.interval.lo;
                            double m2 = a1.interval.hi;
                            hi = std::max(m1, m2);
                        }
                    }
                    Interval iv(lo, hi, true, false);
                    st.set(ins->result.name, AbstractValue::make_num_interval(iv, tainted));
                } else if (name == "maximum") {
                    double lo = std::max(a1.interval.lo, a2.interval.lo);
                    double hi = std::max(a1.interval.hi, a2.interval.hi);
                    Interval iv(lo, hi, true, false);
                    st.set(ins->result.name, AbstractValue::make_num_interval(iv, tainted));
                } else if (name == "minimum") {
                    double lo = std::min(a1.interval.lo, a2.interval.lo);
                    double hi = std::min(a1.interval.hi, a2.interval.hi);
                    Interval iv(lo, hi, true, false);
                    st.set(ins->result.name, AbstractValue::make_num_interval(iv, tainted));
                } else if (name == "isempty") {
                    st.set(ins->result.name, AbstractValue::make_bool(TriBool::Unknown, tainted));
                } else {
                    Interval iv(0.0, Interval::INF, true, false);
                    st.set(ins->result.name, AbstractValue::make_num_interval(iv, tainted));
                }
                break;
            }
            case TACOp::Equal: {
                AbstractValue a1 = eval_operand(ins->arg1, st);
                AbstractValue a2 = eval_operand(ins->arg2, st);
                bool tainted = a1.from_input || a2.from_input;
                TriBool tb = TriBool::Unknown;
                if (a1.interval.is_singleton() && a2.interval.is_singleton() && !a1.interval.is_empty && !a2.interval.is_empty) {
                    tb = (a1.interval.lo == a2.interval.lo) ? TriBool::True : TriBool::False;
                } else if (a1.interval.meet(a2.interval).is_empty) {
                    tb = TriBool::False;
                }
                st.set(ins->result.name, AbstractValue::make_bool(tb, tainted));
                break;
            }
            case TACOp::NotEqual: {
                AbstractValue a1 = eval_operand(ins->arg1, st);
                AbstractValue a2 = eval_operand(ins->arg2, st);
                bool tainted = a1.from_input || a2.from_input;
                TriBool tb = TriBool::Unknown;
                if (a1.interval.is_singleton() && a2.interval.is_singleton() && !a1.interval.is_empty && !a2.interval.is_empty) {
                    tb = (a1.interval.lo != a2.interval.lo) ? TriBool::True : TriBool::False;
                } else if (a1.interval.meet(a2.interval).is_empty) {
                    tb = TriBool::True;
                }
                st.set(ins->result.name, AbstractValue::make_bool(tb, tainted));
                break;
            }
            case TACOp::Less: {
                AbstractValue a1 = eval_operand(ins->arg1, st);
                AbstractValue a2 = eval_operand(ins->arg2, st);
                bool tainted = a1.from_input || a2.from_input;
                TriBool tb = TriBool::Unknown;
                if (!a1.interval.is_empty && !a2.interval.is_empty) {
                    if (a1.interval.hi < a2.interval.lo) tb = TriBool::True;
                    else if (a1.interval.lo >= a2.interval.hi) tb = TriBool::False;
                }
                st.set(ins->result.name, AbstractValue::make_bool(tb, tainted));
                break;
            }
            case TACOp::LessEqual: {
                AbstractValue a1 = eval_operand(ins->arg1, st);
                AbstractValue a2 = eval_operand(ins->arg2, st);
                bool tainted = a1.from_input || a2.from_input;
                TriBool tb = TriBool::Unknown;
                if (!a1.interval.is_empty && !a2.interval.is_empty) {
                    if (a1.interval.hi <= a2.interval.lo) tb = TriBool::True;
                    else if (a1.interval.lo > a2.interval.hi) tb = TriBool::False;
                }
                st.set(ins->result.name, AbstractValue::make_bool(tb, tainted));
                break;
            }
            case TACOp::Greater: {
                AbstractValue a1 = eval_operand(ins->arg1, st);
                AbstractValue a2 = eval_operand(ins->arg2, st);
                bool tainted = a1.from_input || a2.from_input;
                TriBool tb = TriBool::Unknown;
                if (!a1.interval.is_empty && !a2.interval.is_empty) {
                    if (a1.interval.lo > a2.interval.hi) tb = TriBool::True;
                    else if (a1.interval.hi <= a2.interval.lo) tb = TriBool::False;
                }
                st.set(ins->result.name, AbstractValue::make_bool(tb, tainted));
                break;
            }
            case TACOp::GreaterEqual: {
                AbstractValue a1 = eval_operand(ins->arg1, st);
                AbstractValue a2 = eval_operand(ins->arg2, st);
                bool tainted = a1.from_input || a2.from_input;
                TriBool tb = TriBool::Unknown;
                if (!a1.interval.is_empty && !a2.interval.is_empty) {
                    if (a1.interval.lo >= a2.interval.hi) tb = TriBool::True;
                    else if (a1.interval.hi < a2.interval.lo) tb = TriBool::False;
                }
                st.set(ins->result.name, AbstractValue::make_bool(tb, tainted));
                break;
            }
            default:
                break;
        }
    return st;
}

void LogicVerifier::run_dataflow(bool trace, std::ostream& out) {
    size_t n = cfg_.blocks.size();
    block_in_states_.assign(n, AbstractState::bottom());
    block_out_states_.assign(n, AbstractState::bottom());

    if (n == 0) return;

    // Entry block B0 in_state is initial empty state (non-bottom)
    block_in_states_[0] = AbstractState();

    // Map each edge (u -> v) to the propagated state
    std::unordered_map<int64_t, AbstractState> edge_states;
    auto edge_key = [](int u, int v) -> int64_t {
        return (((int64_t)u) << 32) | ((uint32_t)v);
    };

    // Count visits per block for loop widening
    std::vector<int> visit_counts(n, 0);

    // Identify loop headers: blocks with a backward edge
    std::vector<bool> is_loop_header(n, false);
    for (size_t u = 0; u < n; ++u) {
        for (int v : cfg_.blocks[u].succs) {
            if (v <= (int)u) {
                is_loop_header[v] = true;
            }
        }
    }

    std::queue<int> worklist;
    std::vector<bool> in_worklist(n, false);

    worklist.push(0);
    in_worklist[0] = true;

    while (!worklist.empty()) {
        int u = worklist.front();
        worklist.pop();
        in_worklist[u] = false;

        const auto& block = cfg_.blocks[u];
        if (block.is_unreachable) continue;

        visit_counts[u]++;

        // Compute out state
        AbstractState out_st = transfer(block, block_in_states_[u]);
        block_out_states_[u] = out_st;

        if (out_st.is_bottom) {
            // Block is infeasible; propagate bottom to all outgoing edges
            for (const auto& edge : block.out_edges) {
                int64_t ek = edge_key(u, edge.target);
                if (edge_states.find(ek) == edge_states.end() || !edge_states[ek].is_bottom) {
                    edge_states[ek] = AbstractState::bottom();
                    if (!in_worklist[edge.target]) {
                        worklist.push(edge.target);
                        in_worklist[edge.target] = true;
                    }
                }
            }
            continue;
        }

        // Branch refinement for outgoing edges
        const TACInstr* last = block.instructions.empty() ? nullptr : block.instructions.back();
        if (last && (last->op == TACOp::IfTrue || last->op == TACOp::IfFalse)) {
            auto [true_st, false_st] = refine_branch(block, out_st, *last);

            for (const auto& edge : block.out_edges) {
                AbstractState propagated = out_st;
                if (edge.type == BasicBlock::EdgeType::TrueBranch) {
                    propagated = true_st;
                } else if (edge.type == BasicBlock::EdgeType::FalseBranch) {
                    propagated = false_st;
                }

                int64_t ek = edge_key(u, edge.target);
                if (edge_states.find(ek) == edge_states.end() || edge_states[ek] != propagated) {
                    edge_states[ek] = propagated;
                    if (!in_worklist[edge.target]) {
                        worklist.push(edge.target);
                        in_worklist[edge.target] = true;
                    }
                }
            }
        } else {
            // Unconditional or fallthrough
            for (const auto& edge : block.out_edges) {
                int64_t ek = edge_key(u, edge.target);
                if (edge_states.find(ek) == edge_states.end() || edge_states[ek] != out_st) {
                    edge_states[ek] = out_st;
                    if (!in_worklist[edge.target]) {
                        worklist.push(edge.target);
                        in_worklist[edge.target] = true;
                    }
                }
            }
        }

        // Recompute in_states for all successors that might have changed
        for (int v : block.succs) {
            if (cfg_.blocks[v].is_unreachable) continue;

            AbstractState new_in = AbstractState::bottom();
            for (int p : cfg_.blocks[v].preds) {
                if (cfg_.blocks[p].is_unreachable) continue;
                int64_t ek = edge_key(p, v);
                auto it = edge_states.find(ek);
                if (it != edge_states.end() && !it->second.is_bottom) {
                    new_in = new_in.join(it->second);
                }
            }

            // Apply widening at loop headers after 3 visits
            if (is_loop_header[v] && visit_counts[v] >= 3) {
                new_in = block_in_states_[v].widen(new_in);
            }

            if (block_in_states_[v] != new_in) {
                block_in_states_[v] = new_in;
                if (!in_worklist[v]) {
                    worklist.push(v);
                    in_worklist[v] = true;
                }
            }
        }
    }

    if (trace) {
        out << "Dataflow Entry States:\n";
        for (size_t i = 0; i < n; ++i) {
            const auto& b = cfg_.blocks[i];
            if (b.is_unreachable) continue;
            out << b.id;
            if (!b.labels.empty()) {
                out << " [" << b.labels[0] << "]";
            }
            out << ":\n";

            // Print user variables (skip temps starting with 't')
            std::vector<std::string> var_names;
            for (const auto& [k, v] : block_in_states_[i].env) {
                if (k.empty() || k[0] == 't') continue;
                var_names.push_back(k);
            }
            std::sort(var_names.begin(), var_names.end());
            for (const auto& name : var_names) {
                out << "  " << name << " = " << block_in_states_[i].env.at(name).to_string() << "\n";
            }

            // Report any infeasible outgoing edge
            for (const auto& edge : b.out_edges) {
                int64_t ek = edge_key((int)i, edge.target);
                if (edge_states[ek].is_bottom) {
                    out << "  edge -> B" << edge.target << " (infeasible)\n";
                }
            }
        }
    }
}

void LogicVerifier::check_findings(std::ostream& /*out*/) {
    std::set<std::pair<int, int>> reported_conditions;

    // Helper: is instruction inside a block ending in an until loop exit test?
    for (size_t b_idx = 0; b_idx < cfg_.blocks.size(); ++b_idx) {
        const auto& block = cfg_.blocks[b_idx];
        if (block.is_unreachable) continue;

        AbstractState cur_st = block_in_states_[b_idx];
        if (cur_st.is_bottom) continue;

        for (const auto* ins : block.instructions) {
            // Check self comparison: LV03
            if (ins->op == TACOp::Equal || ins->op == TACOp::NotEqual ||
                ins->op == TACOp::Less || ins->op == TACOp::LessEqual ||
                ins->op == TACOp::Greater || ins->op == TACOp::GreaterEqual) {
                if (ins->arg1.kind == TACOperand::Kind::Variable &&
                    ins->arg2.kind == TACOperand::Kind::Variable &&
                    ins->arg1.name == ins->arg2.name) {
                    std::string op_str = (ins->op == TACOp::Equal) ? "equals" : (ins->op == TACOp::NotEqual ? "notequals" : "<");
                    std::string res_str = (ins->op == TACOp::Equal || ins->op == TACOp::LessEqual || ins->op == TACOp::GreaterEqual) ? "always true" : "always false";
                    report("LV03", FindingSeverity::Warning, ins->line, ins->col,
                           "comparison of variable '" + ins->arg1.name + "' with itself is " + res_str);
                    reported_conditions.insert({ins->line, ins->col});
                } else {
                    // Check if comparison evaluated under cur_st has a constant outcome
                    AbstractValue a1 = eval_operand(ins->arg1, cur_st);
                    AbstractValue a2 = eval_operand(ins->arg2, cur_st);
                    TriBool tb = TriBool::Unknown;
                    if (ins->op == TACOp::Equal) {
                        if (a1.interval.is_singleton() && a2.interval.is_singleton() && !a1.interval.is_empty && !a2.interval.is_empty) {
                            tb = (a1.interval.lo == a2.interval.lo) ? TriBool::True : TriBool::False;
                        } else if (a1.interval.meet(a2.interval).is_empty) {
                            tb = TriBool::False;
                        }
                    } else if (ins->op == TACOp::NotEqual) {
                        if (a1.interval.is_singleton() && a2.interval.is_singleton() && !a1.interval.is_empty && !a2.interval.is_empty) {
                            tb = (a1.interval.lo != a2.interval.lo) ? TriBool::True : TriBool::False;
                        } else if (a1.interval.meet(a2.interval).is_empty) {
                            tb = TriBool::True;
                        }
                    } else if (ins->op == TACOp::Greater) {
                        if (!a1.interval.is_empty && !a2.interval.is_empty) {
                            if (a1.interval.lo > a2.interval.hi) tb = TriBool::True;
                            else if (a1.interval.hi <= a2.interval.lo) tb = TriBool::False;
                        }
                    } else if (ins->op == TACOp::GreaterEqual) {
                        if (!a1.interval.is_empty && !a2.interval.is_empty) {
                            if (a1.interval.lo >= a2.interval.hi) tb = TriBool::True;
                            else if (a1.interval.hi < a2.interval.lo) tb = TriBool::False;
                        }
                    } else if (ins->op == TACOp::Less) {
                        if (!a1.interval.is_empty && !a2.interval.is_empty) {
                            if (a1.interval.hi < a2.interval.lo) tb = TriBool::True;
                            else if (a1.interval.lo >= a2.interval.hi) tb = TriBool::False;
                        }
                    } else if (ins->op == TACOp::LessEqual) {
                        if (!a1.interval.is_empty && !a2.interval.is_empty) {
                            if (a1.interval.hi <= a2.interval.lo) tb = TriBool::True;
                            else if (a1.interval.lo > a2.interval.hi) tb = TriBool::False;
                        }
                    }

                    if (tb == TriBool::True) {
                        // Check if this block is entered from a short-circuit branch (e.g. B0 had an And branch)
                        bool from_sc = false;
                        for (int p : block.preds) {
                            const auto& pb = cfg_.blocks[p];
                            const TACInstr* p_last = pb.instructions.empty() ? nullptr : pb.instructions.back();
                            if (p_last && (p_last->origin == TACInstr::Origin::And || p_last->origin == TACInstr::Origin::Or)) {
                                from_sc = true;
                                break;
                            }
                        }
                        const TACInstr* blk_last = block.instructions.empty() ? nullptr : block.instructions.back();
                        if (from_sc || (blk_last && (blk_last->origin == TACInstr::Origin::And || blk_last->origin == TACInstr::Origin::Or))) {
                            if (reported_conditions.find({ins->line, ins->col}) == reported_conditions.end()) {
                                report("LV04", FindingSeverity::Warning, ins->line, ins->col,
                                       "one operand of and/or is redundant: already implied");
                                reported_conditions.insert({ins->line, ins->col});
                            }
                        }
                    }
                }
            }

            // Value checks: LV10 and LV11 on / and %
            if (ins->op == TACOp::Div || ins->op == TACOp::Mod) {
                AbstractValue divisor = eval_operand(ins->arg2, cur_st);
                std::string op_name = (ins->op == TACOp::Div) ? "division" : "modulo";
                if (divisor.interval.is_singleton() && divisor.interval.lo == 0.0) {
                    report("LV10", FindingSeverity::Error, ins->line, ins->col,
                           op_name + " by zero: divisor is exactly 0");
                } else if (divisor.interval.contains_zero()) {
                    if (divisor.from_input) {
                        report("LV11", FindingSeverity::Warning, ins->line, ins->col,
                               "possible " + op_name + " by zero: divisor contains 0 and is tainted by input()");
                    } else if (divisor.interval.lo != -Interval::INF && divisor.interval.hi != Interval::INF) {
                        report("LV11", FindingSeverity::Warning, ins->line, ins->col,
                               "possible " + op_name + " by zero: divisor interval " + divisor.interval.to_string() + " contains 0");
                    }
                }
            }

            // Condition checks: LV01 and LV02 on branch instructions
            if (ins->op == TACOp::IfTrue || ins->op == TACOp::IfFalse) {
                int line = ins->origin_line ? ins->origin_line : ins->line;
                int col = ins->origin_col ? ins->origin_col : ins->col;

                AbstractValue cval = cur_st.get(ins->arg1.name);

                if (ins->origin == TACInstr::Origin::Check) {
                    if (reported_conditions.find({line, col}) == reported_conditions.end()) {
                        bool has_else = false;
                        for (const auto& edge : block.out_edges) {
                            if (edge.type == BasicBlock::EdgeType::FalseBranch) {
                                int tgt = edge.target;
                                if (tgt < (int)cfg_.blocks.size() && !cfg_.blocks[tgt].instructions.empty()) {
                                    if (cfg_.blocks[tgt].instructions[0]->op != TACOp::Halt) {
                                        has_else = true;
                                    }
                                }
                            }
                        }

                        // Check if an And branch in our predecessors was already proven always false
                        bool pred_and_false = false;
                        for (int p : block.preds) {
                            const auto& pb = cfg_.blocks[p];
                            const TACInstr* p_last = pb.instructions.empty() ? nullptr : pb.instructions.back();
                            if (p_last && p_last->origin == TACInstr::Origin::And) {
                                int p_line = p_last->origin_line ? p_last->origin_line : p_last->line;
                                int p_col = p_last->origin_col ? p_last->origin_col : p_last->col;
                                if (reported_conditions.count({p_line, p_col})) {
                                    pred_and_false = true;
                                    break;
                                }
                            }
                        }

                        if (cval.tri_bool == TriBool::False && !pred_and_false) {
                            std::string proof = format_proof(ins->arg1.name, block, cur_st, false);
                            std::string msg = "condition is always false: " + proof + " (branch never runs)";
                            report("LV01", FindingSeverity::Warning, line, col, msg);
                            reported_conditions.insert({line, col});
                        } else if (cval.tri_bool == TriBool::True) {
                            std::string proof = format_proof(ins->arg1.name, block, cur_st, true);
                            std::string msg = "condition is always true: " + proof;
                            if (has_else) {
                                msg += " (otherwise never runs)";
                            }
                            report("LV02", FindingSeverity::Warning, line, col, msg);
                            reported_conditions.insert({line, col});
                        }
                    }
                } else if (ins->origin == TACInstr::Origin::And) {
                    // Short-circuit jump for 'and': if_false t goto Lsc
                    // If t is always false: right branch never runs, left is false
                    // If t is always true: t = b, left side is redundant / always true!
                    if (reported_conditions.find({line, col}) == reported_conditions.end()) {
                        if (cval.tri_bool == TriBool::True) {
                            report("LV04", FindingSeverity::Warning, line, col,
                                   "operand of 'and' is redundant (left operand is always true or implied)");
                            reported_conditions.insert({line, col});
                        } else if (cval.tri_bool == TriBool::False) {
                            std::string proof = format_proof(ins->arg1.name, block, cur_st, false);
                            std::string msg = "condition is always false: " + proof;
                            report("LV01", FindingSeverity::Warning, line, col, msg);
                            reported_conditions.insert({line, col});
                        }
                    }
                } else if (ins->origin == TACInstr::Origin::Or) {
                    // Short-circuit jump for 'or': if_true t goto Lsc
                    // If t is always true: right branch never runs, left is true (or condition always true)
                    if (reported_conditions.find({line, col}) == reported_conditions.end()) {
                        if (cval.tri_bool == TriBool::True) {
                            std::string proof = format_proof(ins->arg1.name, block, cur_st, true);
                            std::string msg = "condition is always true: " + proof;
                            report("LV02", FindingSeverity::Warning, line, col, msg);
                            reported_conditions.insert({line, col});
                        }
                    }
                }
            }

            // Transfer within block
            BasicBlock single_instr_block;
            single_instr_block.instructions = {ins};
            cur_st = transfer(single_instr_block, cur_st);
        }
    }

    // Loop checks: LV05, LV06, LV07, LV08, LV09
    // Loop header has a back-edge from within loop
    for (size_t b_idx = 0; b_idx < cfg_.blocks.size(); ++b_idx) {
        const auto& block = cfg_.blocks[b_idx];
        if (block.is_unreachable) continue;

        // Check if block ends in an Until branch: if_true cond goto Lend
        const TACInstr* branch_ins = nullptr;
        for (const auto* ins : block.instructions) {
            if (ins->op == TACOp::IfTrue && ins->origin == TACInstr::Origin::Until) {
                branch_ins = ins;
                break;
            }
        }
        if (!branch_ins) continue;

        int line = branch_ins->origin_line ? branch_ins->origin_line : branch_ins->line;
        int col = branch_ins->origin_col ? branch_ins->origin_col : branch_ins->col;

        // Compute entry state from NON-back-edge predecessors (LV05 entry check)
        AbstractState entry_st = AbstractState::bottom();
        for (int p : block.preds) {
            if (p < (int)b_idx) { // forward predecessor entering the loop
                entry_st = entry_st.join(block_out_states_[p]);
            }
        }
        if (entry_st.is_bottom && !block.preds.empty()) {
            entry_st = block_out_states_[block.preds[0]];
        }

        // Evaluate condition under entry state to see if it is true on entry
        AbstractState eval_entry_st = entry_st;
        for (const auto* ins : block.instructions) {
            if (ins == branch_ins) break;
            eval_entry_st = transfer_instr(*ins, eval_entry_st);
        }
        AbstractValue entry_cval = eval_operand(branch_ins->arg1, eval_entry_st);

        // LV05: loop body never runs: until condition is already true on entry
        if (entry_cval.tri_bool == TriBool::True) {
            report("LV05", FindingSeverity::Warning, line, col,
                   "loop body never runs: condition is already true on entry");
            continue;
        }

        // Gather all blocks in the loop body:
        // Everything reachable from the loop header's fallthrough (body entry) that doesn't pass exit_label
        std::set<int> loop_blocks;
        std::string exit_label = branch_ins->label; // Lend
        int exit_b_idx = cfg_.label_to_block.count(exit_label) ? cfg_.label_to_block.at(exit_label) : -1;

        std::queue<int> lq;
        // Start from successors of b_idx that are NOT exit_b_idx
        for (int s : block.succs) {
            if (s != exit_b_idx && s != (int)b_idx) {
                loop_blocks.insert(s);
                lq.push(s);
            }
        }
        loop_blocks.insert((int)b_idx);

        while (!lq.empty()) {
            int curr = lq.front();
            lq.pop();
            for (int s : cfg_.blocks[curr].succs) {
                if (s != exit_b_idx && s != (int)b_idx && loop_blocks.find(s) == loop_blocks.end()) {
                    loop_blocks.insert(s);
                    lq.push(s);
                }
            }
        }

        // Check if loop has a reachable stop (suppresses LV06) or skip
        bool has_reachable_stop = false;
        bool has_skip = false;

        for (int lb : loop_blocks) {
            for (const auto* ins : cfg_.blocks[lb].instructions) {
                if (ins->origin == TACInstr::Origin::Stop) {
                    has_reachable_stop = true;
                } else if (ins->origin == TACInstr::Origin::Skip) {
                    has_skip = true;
                }
            }
        }

        // Condition variable(s)
        std::string tested_var;
        const TACInstr* def_comp = nullptr;
        for (int i = (int)block.instructions.size() - 2; i >= 0; --i) {
            const auto* ins = block.instructions[i];
            if (ins->result.name == branch_ins->arg1.name) {
                def_comp = ins;
                break;
            }
        }

        if (def_comp) {
            if (def_comp->arg1.kind == TACOperand::Kind::Variable) {
                tested_var = def_comp->arg1.name;
            }
        }

        // Check if tested_var is modified in the loop body
        int mod_count = 0;
        int64_t step_c = 0;
        bool has_step = false;

        std::unordered_map<std::string, const TACInstr*> temp_defs;
        for (int lb : loop_blocks) {
            for (const auto* ins : cfg_.blocks[lb].instructions) {
                if (ins->result.name.rfind("t", 0) == 0) {
                    temp_defs[ins->result.name] = ins;
                }
                if (!tested_var.empty() && ins->result.name == tested_var) {
                    mod_count++;
                    if (ins->op == TACOp::Add && ins->arg1.name == tested_var && ins->arg2.kind == TACOperand::Kind::Constant) {
                        step_c = (ins->arg2.type == Type::Num) ? ins->arg2.num_val : (int64_t)ins->arg2.dec_val;
                        has_step = true;
                    } else if (ins->op == TACOp::Sub && ins->arg1.name == tested_var && ins->arg2.kind == TACOperand::Kind::Constant) {
                        step_c = -((ins->arg2.type == Type::Num) ? ins->arg2.num_val : (int64_t)ins->arg2.dec_val);
                        has_step = true;
                    } else if (ins->op == TACOp::Copy && temp_defs.count(ins->arg1.name)) {
                        const auto* prev = temp_defs[ins->arg1.name];
                        if (prev->op == TACOp::Add && prev->arg1.name == tested_var && prev->arg2.kind == TACOperand::Kind::Constant) {
                            step_c = (prev->arg2.type == Type::Num) ? prev->arg2.num_val : (int64_t)prev->arg2.dec_val;
                            has_step = true;
                        } else if (prev->op == TACOp::Sub && prev->arg1.name == tested_var && prev->arg2.kind == TACOperand::Kind::Constant) {
                            step_c = -((prev->arg2.type == Type::Num) ? prev->arg2.num_val : (int64_t)prev->arg2.dec_val);
                            has_step = true;
                        }
                    }
                }
            }
        }

        // LV06: loop never terminates
        // Condition is constant false (TriBool::False) OR tested_var never modified
        bool lv06_fired = false;
        if (!has_reachable_stop) {
            if (entry_cval.tri_bool == TriBool::False && mod_count == 0) {
                report("LV06", FindingSeverity::Error, line, col,
                       "loop never terminates: condition is constant false and no exit exists");
                lv06_fired = true;
            } else if (!tested_var.empty() && mod_count == 0) {
                report("LV06", FindingSeverity::Error, line, col,
                       "loop never terminates: variable '" + tested_var + "' tested in condition is never modified");
                lv06_fired = true;
            }
        }

        if (lv06_fired) {
            // LV09: statements after this loop are unreachable
            int exit_block_idx = cfg_.label_to_block.at(exit_label);
            if (exit_block_idx < (int)cfg_.blocks.size()) {
                const auto& exit_b = cfg_.blocks[exit_block_idx];
                for (const auto* post_ins : exit_b.instructions) {
                    if (post_ins->op != TACOp::Halt && post_ins->line > 0) {
                        report("LV09", FindingSeverity::Warning, post_ins->line, post_ins->col,
                               "unreachable code after non-terminating loop");
                        break;
                    }
                }
            }
            continue;
        }

        // LV07 & LV08: step / direction checks for until (v equals K)
        if (!has_reachable_stop && !has_skip && mod_count == 1 && has_step && def_comp && def_comp->op == TACOp::Equal) {
            AbstractValue v0 = entry_st.get(tested_var);
            if (v0.interval.is_singleton() && def_comp->arg2.kind == TACOperand::Kind::Constant) {
                int64_t init_val = (int64_t)v0.interval.lo;
                int64_t target_val = (def_comp->arg2.type == Type::Num) ? def_comp->arg2.num_val : (int64_t)def_comp->arg2.dec_val;

                int64_t diff = target_val - init_val;

                // Wrong direction: LV08
                if ((diff > 0 && step_c < 0) || (diff < 0 && step_c > 0)) {
                    report("LV08", FindingSeverity::Error, line, col,
                           "wrong direction: step moves '" + tested_var + "' away from target value " + std::to_string(target_val));
                } else if (step_c != 0) {
                    // Overshoot / not divisible: LV07
                    if (diff % step_c != 0) {
                        std::string seq = std::to_string(init_val) + ", " +
                                          std::to_string(init_val + step_c) + ", " +
                                          std::to_string(init_val + 2 * step_c);
                        report("LV07", FindingSeverity::Error, line, col,
                               "step never reaches target value: '" + tested_var + "' takes " + seq +
                               " and never equals " + std::to_string(target_val));
                    }
                }
            }
        }
    }

    std::sort(findings_.begin(), findings_.end());
}

bool LogicVerifier::verify(bool strict, bool trace, std::ostream& out, std::ostream& err, int stage1_errs, int stage1_warns) {
    findings_.clear();
    run_dataflow(trace, out);
    check_findings(out);

    int error_count = 0;
    int warning_count = 0;

    for (const auto& f : findings_) {
        bool is_err = (f.severity == FindingSeverity::Error) || strict;
        if (is_err) error_count++;
        else warning_count++;

        std::string sev_str = is_err ? "error" : "warning";
        std::string line_msg = "line " + std::to_string(f.line) + ", col " + std::to_string(f.col) +
                               ": " + sev_str + ": [" + f.code + "] " + f.message;

        if (is_err) {
            err << line_msg << "\n";
        } else {
            out << line_msg << "\n";
        }
    }

    std::string summary = "Load and Link: " + std::to_string(stage1_errs) + " error(s), " +
                          std::to_string(stage1_warns) + " warning(s). Logic Verifier: " +
                          std::to_string(error_count) + " error(s), " +
                          std::to_string(warning_count) + " warning(s).";
    if (error_count > 0 || stage1_errs > 0) {
        summary += " Pipeline stopped.";
        err << summary << "\n";
    } else {
        out << summary << "\n";
    }

    return (error_count == 0 && stage1_errs == 0);
}

} // namespace cmm
