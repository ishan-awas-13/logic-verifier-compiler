#include "optimizer.h"
#include "runtime_semantics.h"
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <iostream>

namespace cmm {

TACProgram Optimizer::optimize(OptStats* stats) {
    if (stats) {
        stats->instructions_before = program_.instructions.size();
    }

    int max_rounds = 10;
    int round = 0;

    while (round < max_rounds) {
        round++;
        bool changed = false;

        if (pass_constant_folding(stats)) changed = true;
        if (pass_local_propagation(stats)) changed = true;
        if (pass_algebraic_simplification(stats)) changed = true;
        if (pass_branch_folding(stats)) changed = true;
        if (pass_unreachable_removal(stats)) changed = true;
        if (pass_dead_code_elimination(stats)) changed = true;
        if (pass_jump_cleanup(stats)) changed = true;

        if (!changed) break;
    }

    if (stats) {
        stats->rounds = round;
        stats->instructions_after = program_.instructions.size();
    }

    return program_;
}

// 1. Constant folding
bool Optimizer::pass_constant_folding(OptStats* stats) {
    bool changed = false;
    for (auto& ins : program_.instructions) {
        if (ins.arg1.kind == TACOperand::Kind::Constant && ins.arg2.kind == TACOperand::Kind::Constant) {
            TACOperand folded = TACOperand::none();

            if (ins.operand_type == Type::Num) {
                int64_t a = ins.arg1.num_val;
                int64_t b = ins.arg2.num_val;
                int64_t r = 0;
                bool ok = false;

                switch (ins.op) {
                    case TACOp::Add: ok = safe_add_num(a, b, r); break;
                    case TACOp::Sub: ok = safe_sub_num(a, b, r); break;
                    case TACOp::Mul: ok = safe_mul_num(a, b, r); break;
                    case TACOp::Div: ok = safe_div_num(a, b, r); break;
                    case TACOp::Mod: ok = safe_mod_num(a, b, r); break;
                    case TACOp::Less: folded = TACOperand::make_bool(a < b); break;
                    case TACOp::LessEqual: folded = TACOperand::make_bool(a <= b); break;
                    case TACOp::Greater: folded = TACOperand::make_bool(a > b); break;
                    case TACOp::GreaterEqual: folded = TACOperand::make_bool(a >= b); break;
                    case TACOp::Equal: folded = TACOperand::make_bool(a == b); break;
                    case TACOp::NotEqual: folded = TACOperand::make_bool(a != b); break;
                    default: break;
                }
                if (ok) folded = TACOperand::make_num(r);
            } else if (ins.operand_type == Type::Decimal) {
                double a = ins.arg1.dec_val;
                double b = ins.arg2.dec_val;
                switch (ins.op) {
                    case TACOp::Add: folded = TACOperand::make_decimal(a + b); break;
                    case TACOp::Sub: folded = TACOperand::make_decimal(a - b); break;
                    case TACOp::Mul: folded = TACOperand::make_decimal(a * b); break;
                    case TACOp::Div:
                        if (b != 0.0) folded = TACOperand::make_decimal(a / b);
                        break;
                    case TACOp::Less: folded = TACOperand::make_bool(a < b); break;
                    case TACOp::LessEqual: folded = TACOperand::make_bool(a <= b); break;
                    case TACOp::Greater: folded = TACOperand::make_bool(a > b); break;
                    case TACOp::GreaterEqual: folded = TACOperand::make_bool(a >= b); break;
                    case TACOp::Equal: folded = TACOperand::make_bool(a == b); break;
                    case TACOp::NotEqual: folded = TACOperand::make_bool(a != b); break;
                    default: break;
                }
            } else if (ins.operand_type == Type::Boolean) {
                bool a = ins.arg1.bool_val;
                bool b = ins.arg2.bool_val;
                if (ins.op == TACOp::Equal) folded = TACOperand::make_bool(a == b);
                else if (ins.op == TACOp::NotEqual) folded = TACOperand::make_bool(a != b);
            } else if (ins.operand_type == Type::Letter) {
                char a = ins.arg1.letter_val;
                char b = ins.arg2.letter_val;
                if (ins.op == TACOp::Equal) folded = TACOperand::make_bool(a == b);
                else if (ins.op == TACOp::NotEqual) folded = TACOperand::make_bool(a != b);
            } else if (ins.operand_type == Type::Word) {
                const std::string& a = ins.arg1.word_val;
                const std::string& b = ins.arg2.word_val;
                if (ins.op == TACOp::Add) folded = TACOperand::make_word(a + b);
                else if (ins.op == TACOp::Equal) folded = TACOperand::make_bool(a == b);
                else if (ins.op == TACOp::NotEqual) folded = TACOperand::make_bool(a != b);
            }

            if (!folded.is_none()) {
                ins.op = TACOp::Copy;
                ins.arg1 = folded;
                ins.arg2 = TACOperand::none();
                changed = true;
                if (stats) stats->const_fold_count++;
            }
        } else if (ins.arg1.kind == TACOperand::Kind::Constant && ins.arg2.is_none()) {
            TACOperand folded = TACOperand::none();
            if (ins.op == TACOp::Neg) {
                if (ins.arg1.type == Type::Num) {
                    int64_t r = 0;
                    if (safe_sub_num(0, ins.arg1.num_val, r)) folded = TACOperand::make_num(r);
                } else if (ins.arg1.type == Type::Decimal) {
                    folded = TACOperand::make_decimal(-ins.arg1.dec_val);
                }
            } else if (ins.op == TACOp::Not) {
                if (ins.arg1.type == Type::Boolean) {
                    folded = TACOperand::make_bool(!ins.arg1.bool_val);
                }
            } else if (ins.op == TACOp::I2D) {
                if (ins.arg1.type == Type::Num) {
                    folded = TACOperand::make_decimal(static_cast<double>(ins.arg1.num_val));
                }
            }

            if (!folded.is_none()) {
                ins.op = TACOp::Copy;
                ins.arg1 = folded;
                changed = true;
                if (stats) stats->const_fold_count++;
            }
        }
    }
    return changed;
}

// 2. Local constant and copy propagation
bool Optimizer::pass_local_propagation(OptStats* stats) {
    bool changed = false;
    CFG cfg = CFG::build(program_);

    for (const auto& block : cfg.blocks) {
        std::unordered_map<std::string, TACOperand> val_map;

        for (const auto* p_ins : block.instructions) {
            // Modify instruction in program_
            TACInstr* ins = const_cast<TACInstr*>(p_ins);

            auto substitute = [&](TACOperand& op) {
                if (op.kind == TACOperand::Kind::Variable || op.kind == TACOperand::Kind::Temp) {
                    auto it = val_map.find(op.name);
                    if (it != val_map.end()) {
                        op = it->second;
                        changed = true;
                        if (stats) stats->prop_count++;
                    }
                }
            };

            substitute(ins->arg1);
            substitute(ins->arg2);

            // If instruction assigns a result, update val_map
            if (!ins->result.is_none()) {
                std::string target = ins->result.name;

                // Invalidate any existing mappings that point to target
                for (auto it = val_map.begin(); it != val_map.end(); ) {
                    if (it->second.name == target) {
                        it = val_map.erase(it);
                    } else {
                        ++it;
                    }
                }

                // If Copy, register target -> source
                if (ins->op == TACOp::Copy) {
                    if (ins->arg1.kind == TACOperand::Kind::Constant) {
                        val_map[target] = ins->arg1;
                    } else if (ins->arg1.kind == TACOperand::Kind::Variable || ins->arg1.kind == TACOperand::Kind::Temp) {
                        // Canonicalize: if arg1 maps to something, use that
                        auto it = val_map.find(ins->arg1.name);
                        if (it != val_map.end()) {
                            val_map[target] = it->second;
                        } else {
                            val_map[target] = ins->arg1;
                        }
                    } else {
                        val_map.erase(target);
                    }
                } else {
                    val_map.erase(target);
                }
            }
        }
    }

    return changed;
}

// 3. Algebraic simplification (num only)
bool Optimizer::pass_algebraic_simplification(OptStats* stats) {
    bool changed = false;
    for (auto& ins : program_.instructions) {
        if (ins.operand_type != Type::Num) continue;

        auto is_const_0 = [](const TACOperand& op) {
            return op.kind == TACOperand::Kind::Constant && op.type == Type::Num && op.num_val == 0;
        };
        auto is_const_1 = [](const TACOperand& op) {
            return op.kind == TACOperand::Kind::Constant && op.type == Type::Num && op.num_val == 1;
        };

        if (ins.op == TACOp::Add) {
            if (is_const_0(ins.arg2)) {
                // x + 0 -> x
                ins.op = TACOp::Copy;
                ins.arg2 = TACOperand::none();
                changed = true;
                if (stats) stats->algebra_count++;
            } else if (is_const_0(ins.arg1)) {
                // 0 + x -> x
                ins.op = TACOp::Copy;
                ins.arg1 = ins.arg2;
                ins.arg2 = TACOperand::none();
                changed = true;
                if (stats) stats->algebra_count++;
            }
        } else if (ins.op == TACOp::Sub) {
            if (is_const_0(ins.arg2)) {
                // x - 0 -> x
                ins.op = TACOp::Copy;
                ins.arg2 = TACOperand::none();
                changed = true;
                if (stats) stats->algebra_count++;
            }
        } else if (ins.op == TACOp::Mul) {
            if (is_const_1(ins.arg2)) {
                // x * 1 -> x
                ins.op = TACOp::Copy;
                ins.arg2 = TACOperand::none();
                changed = true;
                if (stats) stats->algebra_count++;
            } else if (is_const_1(ins.arg1)) {
                // 1 * x -> x
                ins.op = TACOp::Copy;
                ins.arg1 = ins.arg2;
                ins.arg2 = TACOperand::none();
                changed = true;
                if (stats) stats->algebra_count++;
            } else if (is_const_0(ins.arg2) || is_const_0(ins.arg1)) {
                // x * 0 or 0 * x -> 0
                ins.op = TACOp::Copy;
                ins.arg1 = TACOperand::make_num(0);
                ins.arg2 = TACOperand::none();
                changed = true;
                if (stats) stats->algebra_count++;
            }
        } else if (ins.op == TACOp::Div) {
            if (is_const_1(ins.arg2)) {
                // x / 1 -> x
                ins.op = TACOp::Copy;
                ins.arg2 = TACOperand::none();
                changed = true;
                if (stats) stats->algebra_count++;
            }
        }
    }
    return changed;
}

// 4. Branch folding
bool Optimizer::pass_branch_folding(OptStats* stats) {
    bool changed = false;
    std::vector<TACInstr> new_instrs;

    for (auto& ins : program_.instructions) {
        if (ins.op == TACOp::IfTrue && ins.arg1.kind == TACOperand::Kind::Constant && ins.arg1.type == Type::Boolean) {
            if (ins.arg1.bool_val) {
                // Always true -> goto L
                ins.op = TACOp::Goto;
                ins.arg1 = TACOperand::none();
                new_instrs.push_back(ins);
            } else {
                // Always false -> drop
            }
            changed = true;
            if (stats) stats->branch_fold_count++;
        } else if (ins.op == TACOp::IfFalse && ins.arg1.kind == TACOperand::Kind::Constant && ins.arg1.type == Type::Boolean) {
            if (!ins.arg1.bool_val) {
                // Always false -> goto L
                ins.op = TACOp::Goto;
                ins.arg1 = TACOperand::none();
                new_instrs.push_back(ins);
            } else {
                // Always true -> drop
            }
            changed = true;
            if (stats) stats->branch_fold_count++;
        } else {
            new_instrs.push_back(ins);
        }
    }

    if (changed) {
        program_.instructions = std::move(new_instrs);
    }
    return changed;
}

// 5. Unreachable code removal
bool Optimizer::pass_unreachable_removal(OptStats* stats) {
    bool changed = false;
    CFG cfg = CFG::build(program_);

    // Find all labels targeted by reachable blocks
    std::unordered_set<std::string> targeted_labels;
    for (const auto& b : cfg.blocks) {
        if (!b.is_unreachable) {
            for (const auto* ins : b.instructions) {
                if (!ins->label.empty() && (ins->op == TACOp::Goto || ins->op == TACOp::IfTrue || ins->op == TACOp::IfFalse)) {
                    targeted_labels.insert(ins->label);
                }
            }
        }
    }

    std::vector<TACInstr> new_instrs;
    for (size_t i = 0; i < cfg.blocks.size(); ++i) {
        const auto& b = cfg.blocks[i];
        if (b.is_unreachable) {
            // Keep labels that are still targeted
            for (const auto& lbl : b.labels) {
                if (targeted_labels.count(lbl)) {
                    new_instrs.push_back(TACInstr::make_label(lbl, 0, 0));
                } else {
                    changed = true;
                    if (stats) stats->unreachable_count++;
                }
            }
            if (!b.instructions.empty()) {
                changed = true;
                if (stats) stats->unreachable_count += (int)b.instructions.size();
            }
        } else {
            for (const auto& lbl : b.labels) {
                new_instrs.push_back(TACInstr::make_label(lbl, 0, 0));
            }
            for (const auto* ins : b.instructions) {
                new_instrs.push_back(*ins);
            }
        }
    }

    if (changed) {
        program_.instructions = std::move(new_instrs);
    }
    return changed;
}

// 6. Dead code elimination via liveness analysis on the CFG
bool Optimizer::pass_dead_code_elimination(OptStats* stats) {
    CFG cfg = CFG::build(program_);
    size_t num_blocks = cfg.blocks.size();
    if (num_blocks == 0) return false;

    // Compute Gen and Kill sets for each block
    std::vector<std::unordered_set<std::string>> gen(num_blocks);
    std::vector<std::unordered_set<std::string>> kill(num_blocks);

    for (size_t i = 0; i < num_blocks; ++i) {
        for (const auto* ins : cfg.blocks[i].instructions) {
            auto check_use = [&](const TACOperand& op) {
                if (op.kind == TACOperand::Kind::Variable || op.kind == TACOperand::Kind::Temp) {
                    if (kill[i].find(op.name) == kill[i].end()) {
                        gen[i].insert(op.name);
                    }
                }
            };
            check_use(ins->arg1);
            check_use(ins->arg2);

            if (!ins->result.is_none()) {
                kill[i].insert(ins->result.name);
            }
        }
    }

    // Liveness dataflow iteration: IN = GEN union (OUT - KILL), OUT = union(IN of succs)
    std::vector<std::unordered_set<std::string>> live_in(num_blocks);
    std::vector<std::unordered_set<std::string>> live_out(num_blocks);

    bool changed_df = true;
    while (changed_df) {
        changed_df = false;
        for (int i = (int)num_blocks - 1; i >= 0; --i) {
            std::unordered_set<std::string> new_out;
            for (int s : cfg.blocks[i].succs) {
                new_out.insert(live_in[s].begin(), live_in[s].end());
            }
            live_out[i] = new_out;

            std::unordered_set<std::string> new_in = gen[i];
            for (const auto& var : live_out[i]) {
                if (kill[i].find(var) == kill[i].end()) {
                    new_in.insert(var);
                }
            }

            if (new_in != live_in[i]) {
                live_in[i] = std::move(new_in);
                changed_df = true;
            }
        }
    }

    // Now filter instructions inside each block using backward liveness tracking
    bool changed = false;
    std::vector<TACInstr> new_instrs;

    for (size_t b_idx = 0; b_idx < num_blocks; ++b_idx) {
        const auto& block = cfg.blocks[b_idx];
        for (const auto& lbl : block.labels) {
            new_instrs.push_back(TACInstr::make_label(lbl, 0, 0));
        }

        std::unordered_set<std::string> live = live_out[b_idx];
        std::vector<bool> keep(block.instructions.size(), true);

        for (int i = (int)block.instructions.size() - 1; i >= 0; --i) {
            const auto* ins = block.instructions[i];

            bool is_pure = false;
            if (!ins->result.is_none()) {
                switch (ins->op) {
                    case TACOp::Copy:
                    case TACOp::Add:
                    case TACOp::Sub:
                    case TACOp::Mul:
                    case TACOp::Neg:
                    case TACOp::Not:
                    case TACOp::I2D:
                    case TACOp::Less:
                    case TACOp::LessEqual:
                    case TACOp::Greater:
                    case TACOp::GreaterEqual:
                    case TACOp::Equal:
                    case TACOp::NotEqual:
                        is_pure = true;
                        break;
                    case TACOp::Div:
                    case TACOp::Mod:
                        // Only pure if divisor is a nonzero constant
                        if (ins->arg2.kind == TACOperand::Kind::Constant) {
                            if (ins->arg2.type == Type::Num && ins->arg2.num_val != 0) is_pure = true;
                            else if (ins->arg2.type == Type::Decimal && ins->arg2.dec_val != 0.0) is_pure = true;
                        }
                        break;
                    default:
                        is_pure = false;
                        break;
                }
            }

            if (is_pure && live.find(ins->result.name) == live.end()) {
                // Not live and pure -> eliminate!
                keep[i] = false;
                changed = true;
                if (stats) stats->dce_count++;
            } else {
                // Keep instruction and update live set
                if (!ins->result.is_none()) {
                    live.erase(ins->result.name);
                }
                if (ins->arg1.kind == TACOperand::Kind::Variable || ins->arg1.kind == TACOperand::Kind::Temp) {
                    live.insert(ins->arg1.name);
                }
                if (ins->arg2.kind == TACOperand::Kind::Variable || ins->arg2.kind == TACOperand::Kind::Temp) {
                    live.insert(ins->arg2.name);
                }
            }
        }

        for (size_t i = 0; i < block.instructions.size(); ++i) {
            if (keep[i]) {
                new_instrs.push_back(*block.instructions[i]);
            }
        }
    }

    if (changed) {
        program_.instructions = std::move(new_instrs);
    }
    return changed;
}

// 7. Jump cleanup
bool Optimizer::pass_jump_cleanup(OptStats* stats) {
    bool changed = false;

    // A. Thread jumps: if L1: goto L2, replace jumps to L1 with jumps to L2
    std::unordered_map<std::string, std::string> jump_targets;
    for (size_t i = 0; i + 1 < program_.instructions.size(); ++i) {
        if (program_.instructions[i].op == TACOp::Label && program_.instructions[i + 1].op == TACOp::Goto) {
            jump_targets[program_.instructions[i].label] = program_.instructions[i + 1].label;
        }
    }

    // Resolve chains: L1 -> L2 -> L3
    for (auto& [from, to] : jump_targets) {
        std::string curr = to;
        std::unordered_set<std::string> seen;
        while (jump_targets.count(curr) && !seen.count(curr)) {
            seen.insert(curr);
            curr = jump_targets[curr];
        }
        to = curr;
    }

    for (auto& ins : program_.instructions) {
        if (!ins.label.empty() && (ins.op == TACOp::Goto || ins.op == TACOp::IfTrue || ins.op == TACOp::IfFalse)) {
            if (jump_targets.count(ins.label) && jump_targets[ins.label] != ins.label) {
                ins.label = jump_targets[ins.label];
                changed = true;
                if (stats) stats->jump_cleanup_count++;
            }
        }
    }

    // B. Remove goto L when L is the very next instruction
    std::vector<TACInstr> step2;
    for (size_t i = 0; i < program_.instructions.size(); ++i) {
        if (program_.instructions[i].op == TACOp::Goto) {
            // Find next non-empty instruction
            size_t next_idx = i + 1;
            while (next_idx < program_.instructions.size() && program_.instructions[next_idx].op == TACOp::Label &&
                   program_.instructions[next_idx].label != program_.instructions[i].label) {
                // If it's a different label, we might skip to it if they are empty
                break;
            }
            if (next_idx < program_.instructions.size() && program_.instructions[next_idx].op == TACOp::Label &&
                program_.instructions[next_idx].label == program_.instructions[i].label) {
                // Redundant goto next label
                changed = true;
                if (stats) stats->jump_cleanup_count++;
                continue;
            }
        }
        step2.push_back(program_.instructions[i]);
    }
    program_.instructions = std::move(step2);

    // C. Delete labels nobody targets
    std::unordered_set<std::string> referenced_labels;
    for (const auto& ins : program_.instructions) {
        if (ins.op == TACOp::Goto || ins.op == TACOp::IfTrue || ins.op == TACOp::IfFalse) {
            referenced_labels.insert(ins.label);
        }
    }

    std::vector<TACInstr> step3;
    for (const auto& ins : program_.instructions) {
        if (ins.op == TACOp::Label) {
            if (!referenced_labels.count(ins.label)) {
                changed = true;
                if (stats) stats->jump_cleanup_count++;
                continue;
            }
        }
        step3.push_back(ins);
    }
    program_.instructions = std::move(step3);

    return changed;
}

} // namespace cmm

