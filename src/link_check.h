#pragma once

#include "ast.h"
#include "library_registry.h"
#include <string>
#include <vector>
#include <iostream>

namespace cmm {

enum class LinkDiagSeverity {
    Warning,
    Error
};

struct LinkDiagnostic {
    std::string code; // LV12, LV13, LV14, LV15, LV16
    LinkDiagSeverity severity = LinkDiagSeverity::Warning;
    int line = 0;
    int col = 0;
    std::string message;

    bool operator<(const LinkDiagnostic& other) const {
        if (line != other.line) return line < other.line;
        if (col != other.col) return col < other.col;
        return code < other.code;
    }
};

struct LinkMapEntry {
    std::string symbol;
    std::string kind;     // "type" or "function"
    std::string provider; // provider name or "none"
    std::string status;   // "resolved", "NOT IMPORTED", "UNDEFINED"
    int line = 0;
};

class LinkChecker {
public:
    LinkChecker(const Program& program, const std::vector<LibUse>& lib_uses, const LibraryRegistry& registry);

    bool check(bool strict, std::ostream& out, std::ostream& err);

    int error_count() const { return error_count_; }
    int warning_count() const { return warning_count_; }

    const std::vector<LinkDiagnostic>& get_diagnostics() const { return diagnostics_; }
    const std::vector<std::string>& get_loaded_libraries() const { return loaded_libraries_; }
    const std::vector<std::pair<std::string, int>>& get_loaded_libraries_with_lines() const { return loaded_with_lines_; }
    const std::vector<LinkMapEntry>& get_link_map() const { return link_map_; }

    void print_link_report(std::ostream& out) const;

private:
    const Program& program_;
    const std::vector<LibUse>& lib_uses_;
    const LibraryRegistry& registry_;

    std::vector<LinkDiagnostic> diagnostics_;
    std::vector<std::string> loaded_libraries_;
    std::vector<std::pair<std::string, int>> loaded_with_lines_;
    std::vector<LinkMapEntry> link_map_;

    int error_count_ = 0;
    int warning_count_ = 0;

    void run_checks(bool strict);
};

} // namespace cmm

