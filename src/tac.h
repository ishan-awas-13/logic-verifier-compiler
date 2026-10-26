#pragma once

#include "types.h"
#include <string>
#include <vector>
#include <iostream>
#include <cstdint>

namespace cmm {

struct TACOperand {
    enum class Kind {
        None,
        Variable,
        Temp,
        Constant
    };

    Kind kind = Kind::None;
    Type type = Type::Void;
    std::string name;

    int64_t num_val = 0;
    double dec_val = 0.0;
    std::string word_val;
    char letter_val = '\0';
    bool bool_val = false;

    static TACOperand make_var(const std::string& name, Type type);
    static TACOperand make_temp(const std::string& name, Type type);
    static TACOperand make_num(int64_t val);
    static TACOperand make_decimal(double val);
    static TACOperand make_word(const std::string& val);
    static TACOperand make_letter(char val);
    static TACOperand make_bool(bool val);
    static TACOperand none();

    std::string to_string() const;
    bool is_none() const { return kind == Kind::None; }
};

enum class TACOp {
    Copy,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Less,
    Greater,
    LessEqual,
    GreaterEqual,
    Equal,
    NotEqual,
    Neg,
    Not,
    I2D,
    Label,
    Goto,
    IfTrue,
    IfFalse,
    Show,
    Input,
    Give,
    Call,
    Halt
};

struct TACInstr {
    TACOp op = TACOp::Halt;
    Type operand_type = Type::Void;
    TACOperand result;
    TACOperand arg1;
    TACOperand arg2;
    std::string label;
    int line = 0;
    int col = 0;
    enum class Origin { None, Check, Until, And, Or, Stop, Skip };
    Origin origin = Origin::None;
    int origin_line = 0;
    int origin_col = 0;

    TACInstr() = default;
    TACInstr(TACOp op, Type operand_type, TACOperand result, TACOperand arg1, TACOperand arg2, std::string label, int line, int col)
        : op(op), operand_type(operand_type), result(std::move(result)), arg1(std::move(arg1)),
          arg2(std::move(arg2)), label(std::move(label)), line(line), col(col) {}

    static TACInstr make_copy(TACOperand dest, TACOperand src, int line, int col);
    static TACInstr make_binary(TACOp op, Type operand_type, TACOperand dest, TACOperand a, TACOperand b, int line, int col);
    static TACInstr make_neg(TACOperand dest, TACOperand src, int line, int col);
    static TACInstr make_not(TACOperand dest, TACOperand src, int line, int col);
    static TACInstr make_i2d(TACOperand dest, TACOperand src, int line, int col);
    static TACInstr make_label(const std::string& label, int line, int col);
    static TACInstr make_goto(const std::string& label, int line, int col);
    static TACInstr make_if_true(TACOperand cond, const std::string& label, int line, int col);
    static TACInstr make_if_false(TACOperand cond, const std::string& label, int line, int col);
    static TACInstr make_show(TACOperand src, int line, int col);
    static TACInstr make_input(TACOperand dest, Type type, int line, int col);
    static TACInstr make_give(TACOperand src, int line, int col);
    static TACInstr make_call(TACOperand dest, const std::string& name, TACOperand a1, TACOperand a2, int line, int col);
    static TACInstr make_halt();

    std::string to_string() const;
};

class TACProgram {
public:
    std::vector<TACInstr> instructions;
    int temp_counter = 0;
    int label_counter = 0;

    TACOperand new_temp(Type type) {
        temp_counter++;
        return TACOperand::make_temp("t" + std::to_string(temp_counter), type);
    }

    std::string new_label() {
        label_counter++;
        return "L" + std::to_string(label_counter);
    }

    void emit(TACInstr instr) {
        instructions.push_back(std::move(instr));
    }

    void print(std::ostream& os) const;
    void dump() const;
};

} // namespace cmm
