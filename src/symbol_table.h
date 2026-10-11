#pragma once

#include "types.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace cmm {

struct Symbol {
    std::string name;
    Type type;
    bool is_fixed;
    int line;
    int col;
    int scope_depth;
    bool used = false;
};

class SymbolTable {
public:
    SymbolTable();

    void enter_scope();
    void exit_scope();
    int current_depth() const { return current_depth_; }

    // Returns true on success, false if redeclared in the same scope
    bool declare(const std::string& name, Type type, bool is_fixed, int line, int col);

    // Look up from innermost scope to outermost
    Symbol* lookup(const std::string& name);

    // Returns all declared symbols in order of declaration
    const std::vector<std::shared_ptr<Symbol>>& get_all_symbols() const { return all_symbols_; }

private:
    int current_depth_ = 0;
    std::vector<std::unordered_map<std::string, std::shared_ptr<Symbol>>> scopes_;
    std::vector<std::shared_ptr<Symbol>> all_symbols_;
};

} // namespace cmm

