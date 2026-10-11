#include "symbol_table.h"

namespace cmm {

SymbolTable::SymbolTable() {
    enter_scope(); // Global scope (depth 0)
}

void SymbolTable::enter_scope() {
    scopes_.emplace_back();
    current_depth_ = static_cast<int>(scopes_.size()) - 1;
}

void SymbolTable::exit_scope() {
    if (!scopes_.empty()) {
        scopes_.pop_back();
    }
    current_depth_ = static_cast<int>(scopes_.size()) - 1;
}

bool SymbolTable::declare(const std::string& name, Type type, bool is_fixed, int line, int col) {
    if (scopes_.empty()) {
        enter_scope();
    }

    auto& current_scope = scopes_.back();
    if (current_scope.find(name) != current_scope.end()) {
        return false; // Already declared in the same scope
    }

    auto sym = std::make_shared<Symbol>();
    sym->name = name;
    sym->type = type;
    sym->is_fixed = is_fixed;
    sym->line = line;
    sym->col = col;
    sym->scope_depth = current_depth_;
    sym->used = false;

    current_scope[name] = sym;
    all_symbols_.push_back(sym);
    return true;
}

Symbol* SymbolTable::lookup(const std::string& name) {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return found->second.get();
        }
    }
    return nullptr;
}

} // namespace cmm

