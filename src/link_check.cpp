#include "link_check.h"
#include <algorithm>
#include <iomanip>
#include <map>
#include <set>

namespace cmm {

static int levenshtein_distance(const std::string& s1, const std::string& s2) {
    int m = static_cast<int>(s1.size());
    int n = static_cast<int>(s2.size());
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));
    for (int i = 0; i <= m; i++) dp[i][0] = i;
    for (int j = 0; j <= n; j++) dp[0][j] = j;
    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            if (s1[i - 1] == s2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                dp[i][j] = 1 + std::min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]});
            }
        }
    }
    return dp[m][n];
}

LinkChecker::LinkChecker(const Program& program, const std::vector<LibUse>& lib_uses, const LibraryRegistry& registry)
    : program_(program), lib_uses_(lib_uses), registry_(registry) {}

void LinkChecker::run_checks(bool strict) {
    diagnostics_.clear();
    loaded_libraries_.clear();
    loaded_with_lines_.clear();
    link_map_.clear();
    error_count_ = 0;
    warning_count_ = 0;

    std::vector<std::string> avail_names = registry_.allLibraryNames();
    std::string avail_str;
    for (size_t i = 0; i < avail_names.size(); ++i) {
        if (i > 0) avail_str += ", ";
        avail_str += avail_names[i];
    }

    // 1. LOAD: for each import in order
    std::vector<std::pair<std::string, int>> seen_imports;
    for (const auto& imp : program_.imports) {
        const LibraryDef* def = registry_.findLibrary(imp.name);
        if (!def) {
            std::string msg = "load error: cannot load library '" + imp.name + "': no such library. Available libraries: " + avail_str + ".";
            diagnostics_.push_back(LinkDiagnostic{"LV13", LinkDiagSeverity::Error, imp.line, imp.col, msg});
            continue;
        }

        // Check if already imported
        auto it = std::find_if(seen_imports.begin(), seen_imports.end(),
                               [&](const std::pair<std::string, int>& p) { return p.first == imp.name; });
        if (it != seen_imports.end()) {
            std::string msg = "library '" + imp.name + "' is already imported (first imported on line " + std::to_string(it->second) + ").";
            diagnostics_.push_back(LinkDiagnostic{"LV15", LinkDiagSeverity::Warning, imp.line, imp.col, msg});
        } else {
            seen_imports.push_back({imp.name, imp.line});
            loaded_libraries_.push_back(imp.name);
            loaded_with_lines_.push_back({imp.name, imp.line});
        }
    }

    // 2. LINK: for each recorded use in source order
    std::map<std::string, int> resolved_counts;
    for (const auto& lib_name : loaded_libraries_) {
        resolved_counts[lib_name] = 0;
    }

    struct UnresolvedInfo {
        int first_line = 0;
        int first_col = 0;
        LibUseKind kind = LibUseKind::Type;
        std::string provider;
        int count = 0;
    };
    std::map<std::string, UnresolvedInfo> unresolved_map;
    std::vector<std::string> unresolved_order;

    for (const auto& use : lib_uses_) {
        LinkMapEntry entry;
        entry.symbol = use.symbol;
        entry.kind = (use.kind == LibUseKind::Type) ? "type" : "function";
        entry.line = use.line;

        if (use.provider.empty()) {
            entry.provider = "none";
            entry.status = "UNDEFINED";

            auto it = unresolved_map.find(use.symbol);
            if (it == unresolved_map.end()) {
                unresolved_map[use.symbol] = UnresolvedInfo{use.line, use.col, use.kind, "", 1};
                unresolved_order.push_back(use.symbol);
            } else {
                it->second.count++;
            }
        } else {
            entry.provider = use.provider;
            bool is_loaded = (std::find(loaded_libraries_.begin(), loaded_libraries_.end(), use.provider) != loaded_libraries_.end());
            if (!is_loaded) {
                entry.status = "NOT IMPORTED";

                auto it = unresolved_map.find(use.symbol);
                if (it == unresolved_map.end()) {
                    unresolved_map[use.symbol] = UnresolvedInfo{use.line, use.col, use.kind, use.provider, 1};
                    unresolved_order.push_back(use.symbol);
                } else {
                    it->second.count++;
                }
            } else {
                entry.status = "resolved";
                resolved_counts[use.provider]++;
            }
        }
        link_map_.push_back(entry);
    }

    // Diagnostics for unresolved symbols (only first use reported)
    for (const auto& sym : unresolved_order) {
        const auto& info = unresolved_map[sym];
        std::string more_uses = (info.count > 1) ? (" (" + std::to_string(info.count - 1) + " more uses)") : "";

        if (info.provider.empty()) {
            std::string msg = "link error: undefined reference to function '" + sym + "': not provided by any library.";
            // Edit distance suggestion
            std::string best_cand = "";
            std::string best_prov = "";
            int min_d = 999;
            for (const auto& fn_name : registry_.allFunctionNames()) {
                int d = levenshtein_distance(sym, fn_name);
                if (d <= 2 && d < min_d) {
                    min_d = d;
                    best_cand = fn_name;
                    best_prov = registry_.providerOfFunction(fn_name);
                }
            }
            if (!best_cand.empty()) {
                msg += " Did you mean '" + best_cand + "' (library '" + best_prov + "')?";
            }
            diagnostics_.push_back(LinkDiagnostic{"LV14", LinkDiagSeverity::Error, info.first_line, info.first_col, msg});
        } else {
            std::string kind_str = (info.kind == LibUseKind::Type) ? "type" : "function";
            std::string msg = "link error: library '" + info.provider + "' is not imported, but " + kind_str + " '" + sym + "' is used here" + more_uses + ". Add \"import " + info.provider + ";\" at the top of the program.";
            diagnostics_.push_back(LinkDiagnostic{"LV12", LinkDiagSeverity::Error, info.first_line, info.first_col, msg});
        }
    }

    // 3. For each loaded library with zero resolved uses -> LV16 warning
    for (const auto& p : loaded_with_lines_) {
        if (resolved_counts[p.first] == 0) {
            std::string msg = "library '" + p.first + "' is imported but never used.";
            // For import line, find the col from program.imports
            int imp_col = 8;
            for (const auto& imp : program_.imports) {
                if (imp.name == p.first && imp.line == p.second) {
                    imp_col = imp.col;
                    break;
                }
            }
            diagnostics_.push_back(LinkDiagnostic{"LV16", LinkDiagSeverity::Warning, p.second, imp_col, msg});
        }
    }

    // Sort diagnostics by line, then col
    std::sort(diagnostics_.begin(), diagnostics_.end());

    // Tally error and warning counts
    for (auto& d : diagnostics_) {
        if (strict && d.severity == LinkDiagSeverity::Warning) {
            d.severity = LinkDiagSeverity::Error;
        }
        if (d.severity == LinkDiagSeverity::Error) {
            error_count_++;
        } else {
            warning_count_++;
        }
    }
}

bool LinkChecker::check(bool strict, std::ostream& out, std::ostream& err) {
    run_checks(strict);

    for (const auto& d : diagnostics_) {
        std::string sev_str = (d.severity == LinkDiagSeverity::Error) ? "error" : "warning";
        std::string line_msg = "line " + std::to_string(d.line) + ", col " + std::to_string(d.col) +
                               ": " + sev_str + ": [" + d.code + "] " + d.message;
        if (d.severity == LinkDiagSeverity::Error) {
            err << line_msg << "\n";
        } else {
            out << line_msg << "\n";
        }
    }

    return (error_count_ == 0);
}

void LinkChecker::print_link_report(std::ostream& out) const {
    if (loaded_with_lines_.empty()) {
        out << "Libraries loaded: none\n";
    } else {
        out << "Libraries loaded: ";
        for (size_t i = 0; i < loaded_with_lines_.size(); ++i) {
            if (i > 0) out << ", ";
            out << loaded_with_lines_[i].first << " (line " << loaded_with_lines_[i].second << ")";
        }
        out << "\n";
    }

    out << "Link map:\n";
    for (const auto& e : link_map_) {
        out << "  " << std::left << std::setw(9) << e.symbol
            << std::setw(10) << e.kind
            << "-> " << std::setw(7) << e.provider
            << std::setw(15) << e.status
            << "line " << e.line << "\n";
    }
}

} // namespace cmm

