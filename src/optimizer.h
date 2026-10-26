#pragma once

#include "tac.h"
#include "cfg.h"
#include <string>
#include <vector>
#include <map>

namespace cmm {

struct OptStats {
    size_t instructions_before = 0;
    size_t instructions_after = 0;
    int rounds = 0;
    int const_fold_count = 0;
    int prop_count = 0;
    int algebra_count = 0;
    int branch_fold_count = 0;
    int unreachable_count = 0;
    int dce_count = 0;
    int jump_cleanup_count = 0;

    int total_changes() const {
        return const_fold_count + prop_count + algebra_count +
               branch_fold_count + unreachable_count + dce_count + jump_cleanup_count;
    }
};

class Optimizer {
public:
    explicit Optimizer(TACProgram program) : program_(std::move(program)) {}

    TACProgram optimize(OptStats* stats = nullptr);

private:
    TACProgram program_;

    bool pass_constant_folding(OptStats* stats);
    bool pass_local_propagation(OptStats* stats);
    bool pass_algebraic_simplification(OptStats* stats);
    bool pass_branch_folding(OptStats* stats);
    bool pass_unreachable_removal(OptStats* stats);
    bool pass_dead_code_elimination(OptStats* stats);
    bool pass_jump_cleanup(OptStats* stats);
};

} // namespace cmm

