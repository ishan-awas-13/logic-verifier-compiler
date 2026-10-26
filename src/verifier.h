#pragma once

#include "cfg.h"
#include "abstract_state.h"
#include <string>
#include <vector>
#include <iostream>

namespace cmm {

enum class FindingSeverity {
    Warning,
    Error
};

struct Finding {
    std::string code; // LV01, LV02, etc.
    FindingSeverity severity = FindingSeverity::Warning;
    int line = 0;
    int col = 0;
    std::string message;

    bool operator<(const Finding& other) const {
        if (line != other.line) return line < other.line;
        if (col != other.col) return col < other.col;
        return code < other.code;
    }
};

class LogicVerifier {
public:
    explicit LogicVerifier(const CFG& cfg) : cfg_(cfg) {}

    bool verify(bool strict, bool trace, std::ostream& out, std::ostream& err, int stage1_errs = 0, int stage1_warns = 0);

    const std::vector<Finding>& get_findings() const { return findings_; }

private:
    const CFG& cfg_;
    std::vector<Finding> findings_;
    std::vector<AbstractState> block_in_states_;
    std::vector<AbstractState> block_out_states_;

    void run_dataflow(bool trace, std::ostream& out);
    AbstractState transfer(const BasicBlock& block, const AbstractState& in_state);
    AbstractState transfer_instr(const TACInstr& ins, const AbstractState& in_state);
    AbstractValue eval_operand(const TACOperand& op, const AbstractState& state);
    
    void check_findings(std::ostream& out);
    void report(const std::string& code, FindingSeverity sev, int line, int col, const std::string& msg);
    std::string format_proof(const std::string& cond_var, const BasicBlock& block, const AbstractState& cur_st, bool is_tautology);
};

} // namespace cmm
