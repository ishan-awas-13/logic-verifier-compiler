#include "cfg.h"
#include <queue>
#include <algorithm>

namespace cmm {

CFG CFG::build(const TACProgram& tac_program) {
    CFG cfg;
    if (tac_program.instructions.empty()) return cfg;

    BasicBlock current_block;
    current_block.index = 0;
    current_block.id = "B0";

    for (const auto& instr : tac_program.instructions) {
        if (instr.op == TACOp::Label) {
            if (!current_block.instructions.empty()) {
                cfg.blocks.push_back(std::move(current_block));
                current_block = BasicBlock();
                current_block.index = (int)cfg.blocks.size();
                current_block.id = "B" + std::to_string(current_block.index);
            }
            current_block.labels.push_back(instr.label);
        } else {
            current_block.instructions.push_back(&instr);
            if (instr.op == TACOp::Goto || instr.op == TACOp::IfTrue ||
                instr.op == TACOp::IfFalse || instr.op == TACOp::Give ||
                instr.op == TACOp::Halt) {
                cfg.blocks.push_back(std::move(current_block));
                current_block = BasicBlock();
                current_block.index = (int)cfg.blocks.size();
                current_block.id = "B" + std::to_string(current_block.index);
            }
        }
    }

    if (!current_block.instructions.empty() || !current_block.labels.empty()) {
        cfg.blocks.push_back(std::move(current_block));
    }

    // Map labels to block index
    for (size_t i = 0; i < cfg.blocks.size(); ++i) {
        for (const auto& lbl : cfg.blocks[i].labels) {
            cfg.label_to_block[lbl] = (int)i;
        }
    }

    auto add_edge = [&](int from, int to, BasicBlock::EdgeType type) {
        cfg.blocks[from].out_edges.push_back({to, type});
        if (std::find(cfg.blocks[from].succs.begin(), cfg.blocks[from].succs.end(), to) == cfg.blocks[from].succs.end()) {
            cfg.blocks[from].succs.push_back(to);
        }
        if (std::find(cfg.blocks[to].preds.begin(), cfg.blocks[to].preds.end(), from) == cfg.blocks[to].preds.end()) {
            cfg.blocks[to].preds.push_back(from);
        }
    };

    // Add edges
    for (size_t i = 0; i < cfg.blocks.size(); ++i) {
        if (cfg.blocks[i].instructions.empty()) {
            if (i + 1 < cfg.blocks.size()) {
                add_edge((int)i, (int)i + 1, BasicBlock::EdgeType::FallThrough);
            }
            continue;
        }

        const auto* last = cfg.blocks[i].instructions.back();
        switch (last->op) {
            case TACOp::Goto: {
                auto it = cfg.label_to_block.find(last->label);
                if (it != cfg.label_to_block.end()) {
                    add_edge((int)i, it->second, BasicBlock::EdgeType::Jump);
                }
                break;
            }
            case TACOp::IfTrue: {
                int fall_through = (int)i + 1;
                auto it = cfg.label_to_block.find(last->label);
                if (fall_through < (int)cfg.blocks.size()) {
                    add_edge((int)i, fall_through, BasicBlock::EdgeType::FalseBranch);
                }
                if (it != cfg.label_to_block.end()) {
                    add_edge((int)i, it->second, BasicBlock::EdgeType::TrueBranch);
                }
                break;
            }
            case TACOp::IfFalse: {
                int fall_through = (int)i + 1;
                auto it = cfg.label_to_block.find(last->label);
                if (fall_through < (int)cfg.blocks.size()) {
                    add_edge((int)i, fall_through, BasicBlock::EdgeType::TrueBranch);
                }
                if (it != cfg.label_to_block.end()) {
                    add_edge((int)i, it->second, BasicBlock::EdgeType::FalseBranch);
                }
                break;
            }
            case TACOp::Halt:
            case TACOp::Give:
                // No successors
                break;
            default: {
                if (i + 1 < cfg.blocks.size()) {
                    add_edge((int)i, (int)i + 1, BasicBlock::EdgeType::FallThrough);
                }
                break;
            }
        }
    }

    // Sort succs and preds numerically for deterministic output
    for (auto& b : cfg.blocks) {
        std::sort(b.succs.begin(), b.succs.end());
        std::sort(b.preds.begin(), b.preds.end());
    }

    // Reachability analysis from B0
    std::vector<bool> reachable(cfg.blocks.size(), false);
    if (!cfg.blocks.empty()) {
        std::queue<int> q;
        reachable[0] = true;
        q.push(0);
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (int v : cfg.blocks[u].succs) {
                if (!reachable[v]) {
                    reachable[v] = true;
                    q.push(v);
                }
            }
        }
    }

    for (size_t i = 0; i < cfg.blocks.size(); ++i) {
        cfg.blocks[i].is_unreachable = !reachable[i];
    }

    return cfg;
}

void CFG::print(std::ostream& os) const {
    for (const auto& b : blocks) {
        os << b.id;
        if (!b.labels.empty()) {
            os << " [";
            for (size_t i = 0; i < b.labels.size(); ++i) {
                if (i > 0) os << ", ";
                os << b.labels[i];
            }
            os << "]";
        }
        if (b.is_unreachable) {
            os << " (unreachable)";
        }
        std::string succ_str;
        for (size_t i = 0; i < b.succs.size(); ++i) {
            if (i > 0) succ_str += ", ";
            succ_str += "B" + std::to_string(b.succs[i]);
        }
        std::string pred_str;
        for (size_t i = 0; i < b.preds.size(); ++i) {
            if (i > 0) pred_str += ", ";
            pred_str += "B" + std::to_string(b.preds[i]);
        }
        if (succ_str.empty()) os << "  succ:";
        else os << "  succ: " << succ_str;
        if (pred_str.empty()) os << "  pred:";
        else os << "  pred: " << pred_str;
        os << "\n";

        for (const auto* instr : b.instructions) {
            os << "  " << instr->to_string() << "\n";
        }
    }
}

void CFG::print_dot(std::ostream& os) const {
    os << "digraph CFG {\n";
    os << "  node [shape=box, fontname=\"Courier\"];\n";
    os << "  edge [fontname=\"Courier\"];\n\n";

    for (const auto& b : blocks) {
        os << "  " << b.id << " [";
        if (b.is_unreachable) {
            os << "style=dashed, ";
        }
        os << "label=\"" << b.id;
        if (!b.labels.empty()) {
            os << " [";
            for (size_t i = 0; i < b.labels.size(); ++i) {
                if (i > 0) os << ", ";
                os << b.labels[i];
            }
            os << "]";
        }
        if (b.is_unreachable) {
            os << " (unreachable)";
        }
        for (const auto* instr : b.instructions) {
            os << "\\l  ";
            std::string s = instr->to_string();
            for (char c : s) {
                if (c == '"') os << "\\\"";
                else if (c == '\\') os << "\\\\";
                else os << c;
            }
        }
        os << "\\l\"];\n";
    }

    os << "\n";
    for (const auto& b : blocks) {
        for (const auto& edge : b.out_edges) {
            os << "  " << b.id << " -> B" << edge.target;
            if (edge.type == BasicBlock::EdgeType::TrueBranch) {
                os << " [label=\"true\"]";
            } else if (edge.type == BasicBlock::EdgeType::FalseBranch) {
                os << " [label=\"false\"]";
            }
            os << ";\n";
        }
    }

    os << "}\n";
}

bool CFG::validate_structure(std::string& err) const {
    for (const auto& b : blocks) {
        for (const auto* instr : b.instructions) {
            if (instr->op == TACOp::Goto || instr->op == TACOp::IfTrue || instr->op == TACOp::IfFalse) {
                if (label_to_block.find(instr->label) == label_to_block.end()) {
                    err = "Jump target label '" + instr->label + "' in block " + b.id + " does not exist in CFG";
                    return false;
                }
            }
        }
    }

    for (const auto& b : blocks) {
        for (int succ_idx : b.succs) {
            if (succ_idx < 0 || succ_idx >= (int)blocks.size()) {
                err = "Block " + b.id + " has invalid successor index " + std::to_string(succ_idx);
                return false;
            }
            const auto& succ_block = blocks[succ_idx];
            bool found = false;
            for (int p : succ_block.preds) {
                if (p == b.index) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                err = "Block " + b.id + " lists " + succ_block.id + " as successor, but " + succ_block.id + " does not list " + b.id + " as predecessor";
                return false;
            }
        }

        for (int pred_idx : b.preds) {
            if (pred_idx < 0 || pred_idx >= (int)blocks.size()) {
                err = "Block " + b.id + " has invalid predecessor index " + std::to_string(pred_idx);
                return false;
            }
            const auto& pred_block = blocks[pred_idx];
            bool found = false;
            for (int s : pred_block.succs) {
                if (s == b.index) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                err = "Block " + b.id + " lists " + pred_block.id + " as predecessor, but " + pred_block.id + " does not list " + b.id + " as successor";
                return false;
            }
        }
    }

    return true;
}

} // namespace cmm

