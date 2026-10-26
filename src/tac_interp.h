#pragma once

#include "tac.h"
#include "runtime_semantics.h"
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <iostream>

namespace cmm {

struct InterpResult {
    std::string output;
    int64_t exit_code = 0;
    std::string status = "ok"; // "ok", "step limit exceeded", or "runtime error: division by zero"
    bool error = false;
};

class TACInterpreter {
public:
    explicit TACInterpreter(const TACProgram& program) : program_(program) {}

    InterpResult run(std::deque<std::string> inputs, int max_steps = 1000000);

private:
    const TACProgram& program_;
    std::unordered_map<std::string, RuntimeValue> env_;
    std::unordered_map<std::string, size_t> label_map_;

    RuntimeValue eval(const TACOperand& op);
};

} // namespace cmm

