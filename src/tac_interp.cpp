#include "tac_interp.h"
#include <sstream>

namespace cmm {

RuntimeValue TACInterpreter::eval(const TACOperand& op) {
    if (op.kind == TACOperand::Kind::Constant) {
        switch (op.type) {
            case Type::Num: return RuntimeValue::make_num(op.num_val);
            case Type::Decimal: return RuntimeValue::make_decimal(op.dec_val);
            case Type::Boolean: return RuntimeValue::make_bool(op.bool_val);
            case Type::Letter: return RuntimeValue::make_letter(op.letter_val);
            case Type::Word: return RuntimeValue::make_word(op.word_val);
            default: return {};
        }
    } else if (op.kind == TACOperand::Kind::Variable || op.kind == TACOperand::Kind::Temp) {
        auto it = env_.find(op.name);
        if (it != env_.end()) {
            return it->second;
        }
    }
    return {};
}

InterpResult TACInterpreter::run(std::deque<std::string> inputs, int max_steps) {
    InterpResult res;
    std::stringstream out;
    env_.clear();
    label_map_.clear();

    const auto& instrs = program_.instructions;
    for (size_t i = 0; i < instrs.size(); ++i) {
        if (instrs[i].op == TACOp::Label) {
            label_map_[instrs[i].label] = i;
        }
    }

    size_t pc = 0;
    int steps = 0;

    while (pc < instrs.size()) {
        if (++steps > max_steps) {
            res.output = out.str();
            res.status = "step limit exceeded";
            res.error = true;
            res.exit_code = 1;
            return res;
        }

        const auto& ins = instrs[pc];

        switch (ins.op) {
            case TACOp::Label: {
                pc++;
                break;
            }
            case TACOp::Copy: {
                env_[ins.result.name] = eval(ins.arg1);
                pc++;
                break;
            }
            case TACOp::Add: {
                RuntimeValue a = eval(ins.arg1);
                RuntimeValue b = eval(ins.arg2);
                if (ins.operand_type == Type::Decimal) {
                    env_[ins.result.name] = RuntimeValue::make_decimal(a.dec_val + b.dec_val);
                } else if (ins.operand_type == Type::Word) {
                    env_[ins.result.name] = RuntimeValue::make_word(a.word_val + b.word_val);
                } else {
                    int64_t r = 0;
                    safe_add_num(a.num_val, b.num_val, r);
                    env_[ins.result.name] = RuntimeValue::make_num(r);
                }
                pc++;
                break;
            }
            case TACOp::Sub: {
                RuntimeValue a = eval(ins.arg1);
                RuntimeValue b = eval(ins.arg2);
                if (ins.operand_type == Type::Decimal) {
                    env_[ins.result.name] = RuntimeValue::make_decimal(a.dec_val - b.dec_val);
                } else {
                    int64_t r = 0;
                    safe_sub_num(a.num_val, b.num_val, r);
                    env_[ins.result.name] = RuntimeValue::make_num(r);
                }
                pc++;
                break;
            }
            case TACOp::Mul: {
                RuntimeValue a = eval(ins.arg1);
                RuntimeValue b = eval(ins.arg2);
                if (ins.operand_type == Type::Decimal) {
                    env_[ins.result.name] = RuntimeValue::make_decimal(a.dec_val * b.dec_val);
                } else {
                    int64_t r = 0;
                    safe_mul_num(a.num_val, b.num_val, r);
                    env_[ins.result.name] = RuntimeValue::make_num(r);
                }
                pc++;
                break;
            }
            case TACOp::Div: {
                RuntimeValue a = eval(ins.arg1);
                RuntimeValue b = eval(ins.arg2);
                if (ins.operand_type == Type::Decimal) {
                    if (b.dec_val == 0.0) {
                        res.output = out.str();
                        res.status = "runtime error: division by zero (line " + std::to_string(ins.line) + ")";
                        res.error = true;
                        res.exit_code = 1;
                        return res;
                    }
                    env_[ins.result.name] = RuntimeValue::make_decimal(a.dec_val / b.dec_val);
                } else {
                    if (b.num_val == 0) {
                        res.output = out.str();
                        res.status = "runtime error: division by zero (line " + std::to_string(ins.line) + ")";
                        res.error = true;
                        res.exit_code = 1;
                        return res;
                    }
                    int64_t r = 0;
                    safe_div_num(a.num_val, b.num_val, r);
                    env_[ins.result.name] = RuntimeValue::make_num(r);
                }
                pc++;
                break;
            }
            case TACOp::Mod: {
                RuntimeValue a = eval(ins.arg1);
                RuntimeValue b = eval(ins.arg2);
                if (b.num_val == 0) {
                    res.output = out.str();
                    res.status = "runtime error: division by zero (line " + std::to_string(ins.line) + ")";
                    res.error = true;
                    res.exit_code = 1;
                    return res;
                }
                int64_t r = 0;
                safe_mod_num(a.num_val, b.num_val, r);
                env_[ins.result.name] = RuntimeValue::make_num(r);
                pc++;
                break;
            }
            case TACOp::Less: {
                RuntimeValue a = eval(ins.arg1);
                RuntimeValue b = eval(ins.arg2);
                bool r = (ins.operand_type == Type::Decimal) ? (a.dec_val < b.dec_val) : (a.num_val < b.num_val);
                env_[ins.result.name] = RuntimeValue::make_bool(r);
                pc++;
                break;
            }
            case TACOp::LessEqual: {
                RuntimeValue a = eval(ins.arg1);
                RuntimeValue b = eval(ins.arg2);
                bool r = (ins.operand_type == Type::Decimal) ? (a.dec_val <= b.dec_val) : (a.num_val <= b.num_val);
                env_[ins.result.name] = RuntimeValue::make_bool(r);
                pc++;
                break;
            }
            case TACOp::Greater: {
                RuntimeValue a = eval(ins.arg1);
                RuntimeValue b = eval(ins.arg2);
                bool r = (ins.operand_type == Type::Decimal) ? (a.dec_val > b.dec_val) : (a.num_val > b.num_val);
                env_[ins.result.name] = RuntimeValue::make_bool(r);
                pc++;
                break;
            }
            case TACOp::GreaterEqual: {
                RuntimeValue a = eval(ins.arg1);
                RuntimeValue b = eval(ins.arg2);
                bool r = (ins.operand_type == Type::Decimal) ? (a.dec_val >= b.dec_val) : (a.num_val >= b.num_val);
                env_[ins.result.name] = RuntimeValue::make_bool(r);
                pc++;
                break;
            }
            case TACOp::Equal: {
                RuntimeValue a = eval(ins.arg1);
                RuntimeValue b = eval(ins.arg2);
                bool r = false;
                switch (ins.operand_type) {
                    case Type::Num: r = (a.num_val == b.num_val); break;
                    case Type::Decimal: r = (a.dec_val == b.dec_val); break;
                    case Type::Boolean: r = (a.bool_val == b.bool_val); break;
                    case Type::Letter: r = (a.letter_val == b.letter_val); break;
                    case Type::Word: r = (a.word_val == b.word_val); break;
                    default: break;
                }
                env_[ins.result.name] = RuntimeValue::make_bool(r);
                pc++;
                break;
            }
            case TACOp::NotEqual: {
                RuntimeValue a = eval(ins.arg1);
                RuntimeValue b = eval(ins.arg2);
                bool r = false;
                switch (ins.operand_type) {
                    case Type::Num: r = (a.num_val != b.num_val); break;
                    case Type::Decimal: r = (a.dec_val != b.dec_val); break;
                    case Type::Boolean: r = (a.bool_val != b.bool_val); break;
                    case Type::Letter: r = (a.letter_val != b.letter_val); break;
                    case Type::Word: r = (a.word_val != b.word_val); break;
                    default: break;
                }
                env_[ins.result.name] = RuntimeValue::make_bool(r);
                pc++;
                break;
            }
            case TACOp::Neg: {
                RuntimeValue a = eval(ins.arg1);
                if (a.type == Type::Decimal) {
                    env_[ins.result.name] = RuntimeValue::make_decimal(-a.dec_val);
                } else {
                    int64_t r = 0;
                    safe_sub_num(0, a.num_val, r);
                    env_[ins.result.name] = RuntimeValue::make_num(r);
                }
                pc++;
                break;
            }
            case TACOp::Not: {
                RuntimeValue a = eval(ins.arg1);
                env_[ins.result.name] = RuntimeValue::make_bool(!a.bool_val);
                pc++;
                break;
            }
            case TACOp::I2D: {
                RuntimeValue a = eval(ins.arg1);
                env_[ins.result.name] = RuntimeValue::make_decimal(static_cast<double>(a.num_val));
                pc++;
                break;
            }
            case TACOp::Goto: {
                pc = label_map_.at(ins.label);
                break;
            }
            case TACOp::IfTrue: {
                RuntimeValue a = eval(ins.arg1);
                if (a.bool_val) {
                    pc = label_map_.at(ins.label);
                } else {
                    pc++;
                }
                break;
            }
            case TACOp::IfFalse: {
                RuntimeValue a = eval(ins.arg1);
                if (!a.bool_val) {
                    pc = label_map_.at(ins.label);
                } else {
                    pc++;
                }
                break;
            }
            case TACOp::Show: {
                RuntimeValue a = eval(ins.arg1);
                out << formatValue(a) << "\n";
                pc++;
                break;
            }
            case TACOp::Input: {
                std::string in_token = "";
                if (!inputs.empty()) {
                    in_token = inputs.front();
                    inputs.pop_front();
                } else {
                    std::cin >> in_token;
                }

                RuntimeValue val;
                val.type = ins.operand_type;
                switch (ins.operand_type) {
                    case Type::Num: {
                        try { val.num_val = std::stoll(in_token); } catch (...) { val.num_val = 0; }
                        break;
                    }
                    case Type::Decimal: {
                        try { val.dec_val = std::stod(in_token); } catch (...) { val.dec_val = 0.0; }
                        break;
                    }
                    case Type::Boolean: {
                        val.bool_val = (in_token == "true" || in_token == "1");
                        break;
                    }
                    case Type::Letter: {
                        val.letter_val = in_token.empty() ? '\0' : in_token[0];
                        break;
                    }
                    case Type::Word: {
                        val.word_val = in_token;
                        break;
                    }
                    default: break;
                }
                env_[ins.result.name] = val;
                pc++;
                break;
            }
            case TACOp::Call: {
                RuntimeValue a1 = eval(ins.arg1);
                RuntimeValue a2 = eval(ins.arg2);
                const std::string& name = ins.label;
                if (name == "length") {
                    int64_t len = static_cast<int64_t>(a1.word_val.length());
                    env_[ins.result.name] = RuntimeValue::make_num(len);
                } else if (name == "isempty") {
                    bool empty = a1.word_val.empty();
                    env_[ins.result.name] = RuntimeValue::make_bool(empty);
                } else if (name == "absolute") {
                    int64_t v = a1.num_val;
                    int64_t abs_v = (v < 0) ? -v : v;
                    env_[ins.result.name] = RuntimeValue::make_num(abs_v);
                } else if (name == "maximum") {
                    int64_t max_v = std::max(a1.num_val, a2.num_val);
                    env_[ins.result.name] = RuntimeValue::make_num(max_v);
                } else if (name == "minimum") {
                    int64_t min_v = std::min(a1.num_val, a2.num_val);
                    env_[ins.result.name] = RuntimeValue::make_num(min_v);
                }
                pc++;
                break;
            }
            case TACOp::Give: {
                RuntimeValue a = eval(ins.arg1);
                res.output = out.str();
                res.exit_code = a.num_val;
                res.status = "ok";
                return res;
            }
            case TACOp::Halt: {
                res.output = out.str();
                res.exit_code = 0;
                res.status = "ok";
                return res;
            }
        }
    }

    res.output = out.str();
    res.exit_code = 0;
    res.status = "ok";
    return res;
}

} // namespace cmm

