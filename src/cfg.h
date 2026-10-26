#pragma once

#include "tac.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

namespace cmm {

struct BasicBlock {
    int index = 0;
    std::string id;                                // "B0", "B1", ...
    std::vector<std::string> labels;               // label(s) it starts with
    std::vector<const TACInstr*> instructions;     // instructions it contains
    std::vector<int> succs;                        // successor block indices
    std::vector<int> preds;                        // predecessor block indices
    bool is_unreachable = false;

    enum class EdgeType { FallThrough, Jump, TrueBranch, FalseBranch };
    struct Edge {
        int target = 0;
        EdgeType type = EdgeType::FallThrough;
    };
    std::vector<Edge> out_edges;
};

class CFG {
public:
    std::vector<BasicBlock> blocks;
    std::unordered_map<std::string, int> label_to_block;

    static CFG build(const TACProgram& tac_program);

    void print(std::ostream& os) const;
    void print_dot(std::ostream& os) const;

    bool validate_structure(std::string& err) const;
};

} // namespace cmm

