#include "tac.h"
#include <sstream>

namespace cmm {

TACOperand TACOperand::make_var(const std::string& name, Type type) {
    TACOperand op;
    op.kind = Kind::Variable;
    op.name = name;
    op.type = type;
    return op;
}

TACOperand TACOperand::make_temp(const std::string& name, Type type) {
    TACOperand op;
    op.kind = Kind::Temp;
    op.name = name;
    op.type = type;
    return op;
}

TACOperand TACOperand::make_num(int64_t val) {
    TACOperand op;
    op.kind = Kind::Constant;
    op.type = Type::Num;
    op.num_val = val;
    return op;
}

TACOperand TACOperand::make_decimal(double val) {
    TACOperand op;
    op.kind = Kind::Constant;
    op.type = Type::Decimal;
    op.dec_val = val;
    return op;
}

TACOperand TACOperand::make_word(const std::string& val) {
    TACOperand op;
    op.kind = Kind::Constant;
    op.type = Type::Word;
    op.word_val = val;
    return op;
}

TACOperand TACOperand::make_letter(char val) {
    TACOperand op;
    op.kind = Kind::Constant;
    op.type = Type::Letter;
    op.letter_val = val;
    return op;
}

TACOperand TACOperand::make_bool(bool val) {
    TACOperand op;
    op.kind = Kind::Constant;
    op.type = Type::Boolean;
    op.bool_val = val;
    return op;
}

TACOperand TACOperand::none() {
    return TACOperand();
}

std::string TACOperand::to_string() const {
    switch (kind) {
        case Kind::None:
            return "";
        case Kind::Variable:
        case Kind::Temp:
            return name;
        case Kind::Constant: {
            switch (type) {
                case Type::Num:
                    return std::to_string(num_val);
                case Type::Decimal: {
                    std::ostringstream oss;
                    oss << dec_val;
                    std::string s = oss.str();
                    if (s.find('.') == std::string::npos && s.find('e') == std::string::npos && s.find('E') == std::string::npos) {
                        s += ".0";
                    }
                    return s;
                }
                case Type::Word: {
                    std::string s = "\"";
                    for (char c : word_val) {
                        if (c == '"') s += "\\\"";
                        else if (c == '\\') s += "\\\\";
                        else if (c == '\n') s += "\\n";
                        else if (c == '\t') s += "\\t";
                        else s += c;
                    }
                    s += "\"";
                    return s;
                }
                case Type::Letter: {
                    std::string s = "'";
                    if (letter_val == '\0') s += "\\0";
                    else if (letter_val == '\n') s += "\\n";
                    else if (letter_val == '\t') s += "\\t";
                    else if (letter_val == '\'') s += "\\'";
                    else if (letter_val == '\\') s += "\\\\";
                    else s += letter_val;
                    s += "'";
                    return s;
                }
                case Type::Boolean:
                    return bool_val ? "true" : "false";
                default:
                    return "<const>";
            }
        }
    }
    return "";
}

TACInstr TACInstr::make_copy(TACOperand dest, TACOperand src, int line, int col) {
    return TACInstr(TACOp::Copy, dest.type, std::move(dest), std::move(src), TACOperand::none(), "", line, col);
}

TACInstr TACInstr::make_binary(TACOp op, Type operand_type, TACOperand dest, TACOperand a, TACOperand b, int line, int col) {
    return TACInstr(op, operand_type, std::move(dest), std::move(a), std::move(b), "", line, col);
}

TACInstr TACInstr::make_neg(TACOperand dest, TACOperand src, int line, int col) {
    return TACInstr(TACOp::Neg, src.type, std::move(dest), std::move(src), TACOperand::none(), "", line, col);
}

TACInstr TACInstr::make_not(TACOperand dest, TACOperand src, int line, int col) {
    return TACInstr(TACOp::Not, Type::Boolean, std::move(dest), std::move(src), TACOperand::none(), "", line, col);
}

TACInstr TACInstr::make_i2d(TACOperand dest, TACOperand src, int line, int col) {
    return TACInstr(TACOp::I2D, Type::Num, std::move(dest), std::move(src), TACOperand::none(), "", line, col);
}

TACInstr TACInstr::make_label(const std::string& label, int line, int col) {
    return TACInstr(TACOp::Label, Type::Void, TACOperand::none(), TACOperand::none(), TACOperand::none(), label, line, col);
}

TACInstr TACInstr::make_goto(const std::string& label, int line, int col) {
    return TACInstr(TACOp::Goto, Type::Void, TACOperand::none(), TACOperand::none(), TACOperand::none(), label, line, col);
}

TACInstr TACInstr::make_if_true(TACOperand cond, const std::string& label, int line, int col) {
    return TACInstr(TACOp::IfTrue, Type::Boolean, TACOperand::none(), std::move(cond), TACOperand::none(), label, line, col);
}

TACInstr TACInstr::make_if_false(TACOperand cond, const std::string& label, int line, int col) {
    return TACInstr(TACOp::IfFalse, Type::Boolean, TACOperand::none(), std::move(cond), TACOperand::none(), label, line, col);
}

TACInstr TACInstr::make_show(TACOperand src, int line, int col) {
    Type t = src.type;
    return TACInstr(TACOp::Show, t, TACOperand::none(), std::move(src), TACOperand::none(), "", line, col);
}

TACInstr TACInstr::make_input(TACOperand dest, Type type, int line, int col) {
    return TACInstr(TACOp::Input, type, std::move(dest), TACOperand::none(), TACOperand::none(), "", line, col);
}

TACInstr TACInstr::make_give(TACOperand src, int line, int col) {
    return TACInstr(TACOp::Give, Type::Num, TACOperand::none(), std::move(src), TACOperand::none(), "", line, col);
}

TACInstr TACInstr::make_call(TACOperand dest, const std::string& name, TACOperand a1, TACOperand a2, int line, int col) {
    Type t = dest.type;
    return TACInstr(TACOp::Call, t, std::move(dest), std::move(a1), std::move(a2), name, line, col);
}

TACInstr TACInstr::make_halt() {
    return TACInstr(TACOp::Halt, Type::Void, TACOperand::none(), TACOperand::none(), TACOperand::none(), "", -1, -1);
}

std::string TACInstr::to_string() const {
    switch (op) {
        case TACOp::Copy:
            return result.to_string() + " = " + arg1.to_string();
        case TACOp::Add:
            return result.to_string() + " = " + arg1.to_string() + " + " + arg2.to_string();
        case TACOp::Sub:
            return result.to_string() + " = " + arg1.to_string() + " - " + arg2.to_string();
        case TACOp::Mul:
            return result.to_string() + " = " + arg1.to_string() + " * " + arg2.to_string();
        case TACOp::Div:
            return result.to_string() + " = " + arg1.to_string() + " / " + arg2.to_string();
        case TACOp::Mod:
            return result.to_string() + " = " + arg1.to_string() + " % " + arg2.to_string();
        case TACOp::Less:
            return result.to_string() + " = " + arg1.to_string() + " < " + arg2.to_string();
        case TACOp::Greater:
            return result.to_string() + " = " + arg1.to_string() + " > " + arg2.to_string();
        case TACOp::LessEqual:
            return result.to_string() + " = " + arg1.to_string() + " <= " + arg2.to_string();
        case TACOp::GreaterEqual:
            return result.to_string() + " = " + arg1.to_string() + " >= " + arg2.to_string();
        case TACOp::Equal:
            return result.to_string() + " = " + arg1.to_string() + " == " + arg2.to_string();
        case TACOp::NotEqual:
            return result.to_string() + " = " + arg1.to_string() + " != " + arg2.to_string();
        case TACOp::Neg:
            return result.to_string() + " = - " + arg1.to_string();
        case TACOp::Not:
            return result.to_string() + " = not " + arg1.to_string();
        case TACOp::I2D:
            return result.to_string() + " = i2d " + arg1.to_string();
        case TACOp::Label:
            return label + ":";
        case TACOp::Goto:
            return "goto " + label;
        case TACOp::IfTrue:
            return "if_true " + arg1.to_string() + " goto " + label;
        case TACOp::IfFalse:
            return "if_false " + arg1.to_string() + " goto " + label;
        case TACOp::Show:
            return "show " + arg1.to_string();
        case TACOp::Input:
            return result.to_string() + " = input." + cmm::to_string(operand_type);
        case TACOp::Give:
            return "give " + arg1.to_string();
        case TACOp::Call: {
            std::string s = result.to_string() + " = call " + label + "(";
            if (!arg1.is_none()) {
                s += arg1.to_string();
                if (!arg2.is_none()) {
                    s += ", " + arg2.to_string();
                }
            }
            s += ")";
            return s;
        }
        case TACOp::Halt:
            return "halt";
    }
    return "";
}

void TACProgram::print(std::ostream& os) const {
    for (const auto& instr : instructions) {
        if (instr.op == TACOp::Label) {
            os << instr.to_string() << "\n";
        } else {
            os << "  " << instr.to_string() << "\n";
        }
    }
}

void TACProgram::dump() const {
    print(std::cout);
}

} // namespace cmm

