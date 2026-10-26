#include "lexer.h"
#include "parser.h"
#include "ast_printer.h"
#include "sema.h"
#include "tac_gen.h"
#include "cfg.h"
#include "verifier.h"
#include "tac_interp.h"
#include "optimizer.h"
#include "library_registry.h"
#include "link_check.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <deque>

static std::deque<std::string> parse_inputs(const std::string& str) {
    std::deque<std::string> res;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, ',')) {
        // trim spaces if any
        size_t first = token.find_first_not_of(" \t\r\n");
        size_t last = token.find_last_not_of(" \t\r\n");
        if (first != std::string::npos && last != std::string::npos) {
            res.push_back(token.substr(first, last - first + 1));
        } else {
            res.push_back("");
        }
    }
    return res;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: c-- <file.cmm> [--tokens | --ast | --sema | --link | --libs | --tac | --cfg | --dot | --verify | --opt | --run-tac | --optcheck] [--stats] [--no-verify] [--inputs <csv>] [--trace] [--strict]\n";
        return 1;
    }

    const auto& registry = cmm::LibraryRegistry::default_registry();

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--libs") {
            for (const auto& lib : registry.get_libraries()) {
                std::cout << lib.name << ":\n";
                std::cout << "  Description: " << lib.description << "\n";
                std::cout << "  Types: ";
                if (lib.provided_types.empty()) {
                    std::cout << "none\n";
                } else {
                    for (size_t t = 0; t < lib.provided_types.size(); ++t) {
                        if (t > 0) std::cout << ", ";
                        std::cout << to_string(lib.provided_types[t]);
                    }
                    std::cout << "\n";
                }
                std::cout << "  Functions:";
                if (lib.functions.empty()) {
                    std::cout << " none\n";
                } else {
                    std::cout << "\n";
                    for (const auto& fn : lib.functions) {
                        std::cout << "    " << fn.name << "(";
                        for (size_t p = 0; p < fn.param_types.size(); ++p) {
                            if (p > 0) std::cout << ", ";
                            std::cout << to_string(fn.param_types[p]);
                        }
                        std::cout << ") -> " << to_string(fn.return_type) << "\n";
                    }
                }
                std::cout << "\n";
            }
            return 0;
        }
    }

    std::string filename = argv[1];
    bool print_tokens = false;
    bool print_ast = false;
    bool run_sema = false;
    bool run_link = false;
    bool print_tac = false;
    bool print_cfg = false;
    bool print_dot = false;
    bool run_verify = false;
    bool trace_verify = false;
    bool strict_verify = false;
    bool run_opt = false;
    bool opt_stats = false;
    bool no_verify = false;
    bool run_tac = false;
    bool opt_check = false;
    std::string inputs_arg = "";
    bool has_inputs = false;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--tokens") print_tokens = true;
        else if (arg == "--ast") print_ast = true;
        else if (arg == "--sema") run_sema = true;
        else if (arg == "--link") run_link = true;
        else if (arg == "--tac") print_tac = true;
        else if (arg == "--cfg") print_cfg = true;
        else if (arg == "--dot") print_dot = true;
        else if (arg == "--verify") run_verify = true;
        else if (arg == "--trace") trace_verify = true;
        else if (arg == "--strict") strict_verify = true;
        else if (arg == "--opt") run_opt = true;
        else if (arg == "--stats") opt_stats = true;
        else if (arg == "--no-verify") no_verify = true;
        else if (arg == "--run-tac") run_tac = true;
        else if (arg == "--optcheck") opt_check = true;
        else if (arg == "--inputs") {
            if (i + 1 < argc) {
                inputs_arg = argv[++i];
                has_inputs = true;
            }
        }
    }

    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open file " << filename << "\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    cmm::Lexer lexer(source);
    auto tokens = lexer.tokenize();

    if (print_tokens) {
        std::cout << std::left << std::setw(6) << "Line" 
                  << std::setw(6) << "Col" 
                  << std::setw(15) << "Type" 
                  << "Lexeme\n";
        std::cout << std::string(40, '-') << "\n";

        for (const auto& token : tokens) {
            std::string lexeme = std::string(token.lexeme);
            if (token.type == cmm::TokenType::Newline) lexeme = "";
            std::cout << std::left << std::setw(6) << token.line 
                      << std::setw(6) << token.col 
                      << std::setw(15) << cmm::to_string(token.type) 
                      << lexeme << "\n";
        }
        return 0;
    }

    cmm::Parser parser(tokens);
    cmm::Program program = parser.parse();

    if (parser.has_error()) {
        for (const auto& err : parser.get_errors()) {
            std::cerr << err.message << "\n";
        }
        return 1;
    }

    if (print_ast) {
        cmm::AstPrinter printer;
        printer.print(program);
        return 0;
    }

    cmm::SemanticAnalyzer sema(registry);
    bool sema_ok = sema.analyze(program);

    for (const auto& diag : sema.get_diagnostics()) {
        if (diag.severity == cmm::DiagnosticSeverity::Error) {
            std::cerr << diag.message << "\n";
        } else {
            std::cout << diag.message << "\n";
        }
    }

    if (run_sema) {
        if (sema_ok) {
            std::cout << "Symbol Table:\n";
            std::cout << std::left << std::setw(16) << "Name"
                      << std::setw(12) << "Type"
                      << std::setw(8) << "Fixed"
                      << std::setw(8) << "Scope"
                      << std::setw(14) << "Declared"
                      << "Read\n";
            std::cout << std::string(64, '-') << "\n";
            for (const auto& sym : sema.get_symbol_table().get_all_symbols()) {
                std::string decl_str = "line " + std::to_string(sym->line) + ":" + std::to_string(sym->col);
                std::cout << std::left << std::setw(16) << sym->name
                          << std::setw(12) << cmm::to_string(sym->type)
                          << std::setw(8) << (sym->is_fixed ? "yes" : "no")
                          << std::setw(8) << sym->scope_depth
                          << std::setw(14) << decl_str
                          << (sym->used ? "yes" : "no") << "\n";
            }
            return 0;
        }
        return 1;
    }

    if (!sema_ok) {
        return 1;
    }

    // Stage 1: Load and Link checks
    cmm::LinkChecker link_checker(program, sema.get_lib_uses(), registry);
    if (run_link) {
        bool link_ok = link_checker.check(strict_verify, std::cout, std::cerr);
        link_checker.print_link_report(std::cout);
        return link_ok ? 0 : 1;
    }

    bool stage1_ok = link_checker.check(strict_verify, std::cout, std::cerr);
    if (!stage1_ok) {
        std::cerr << "Pipeline stopped before intermediate code.\n";
        return 1;
    }

    if (print_tac) {
        cmm::TACGenerator tac_gen;
        cmm::TACProgram tac_prog = tac_gen.generate(program);
        tac_prog.print(std::cout);
        return 0;
    }

    if (print_cfg) {
        cmm::TACGenerator tac_gen;
        cmm::TACProgram tac_prog = tac_gen.generate(program);
        cmm::CFG cfg = cmm::CFG::build(tac_prog);
        cfg.print(std::cout);
        return 0;
    }

    if (print_dot) {
        cmm::TACGenerator tac_gen;
        cmm::TACProgram tac_prog = tac_gen.generate(program);
        cmm::CFG cfg = cmm::CFG::build(tac_prog);
        cfg.print_dot(std::cout);
        return 0;
    }

    if (run_verify) {
        cmm::TACGenerator tac_gen;
        cmm::TACProgram tac_prog = tac_gen.generate(program);
        cmm::CFG cfg = cmm::CFG::build(tac_prog);
        cmm::LogicVerifier verifier(cfg);
        bool verify_ok = verifier.verify(strict_verify, trace_verify, std::cout, std::cerr, link_checker.error_count(), link_checker.warning_count());
        return verify_ok ? 0 : 1;
    }

    if (run_tac) {
        cmm::TACGenerator tac_gen;
        cmm::TACProgram tac_prog = tac_gen.generate(program);
        cmm::TACInterpreter interp(tac_prog);
        std::deque<std::string> inputs = has_inputs ? parse_inputs(inputs_arg) : std::deque<std::string>();
        auto res = interp.run(inputs);
        std::cout << res.output;
        if (res.error) {
            std::cerr << res.status << "\n";
            return 1;
        }
        return (int)res.exit_code;
    }

    if (run_opt) {
        cmm::TACGenerator tac_gen;
        cmm::TACProgram tac_prog = tac_gen.generate(program);

        if (!no_verify) {
            cmm::CFG cfg = cmm::CFG::build(tac_prog);
            cmm::LogicVerifier verifier(cfg);
            bool verify_ok = verifier.verify(strict_verify, false, std::cout, std::cerr, link_checker.error_count(), link_checker.warning_count());
            if (!verify_ok) {
                return 1;
            }
        }

        cmm::OptStats stats;
        cmm::Optimizer opt(tac_prog);
        cmm::TACProgram opt_prog = opt.optimize(&stats);

        if (opt_stats) {
            std::cout << "Optimization Stats:\n";
            std::cout << "  Rounds: " << stats.rounds << "\n";
            std::cout << "  Instructions before: " << stats.instructions_before << "\n";
            std::cout << "  Instructions after:  " << stats.instructions_after << "\n";
            std::cout << "  Constant folding:    " << stats.const_fold_count << "\n";
            std::cout << "  Local propagation:   " << stats.prop_count << "\n";
            std::cout << "  Algebraic simplif.:  " << stats.algebra_count << "\n";
            std::cout << "  Branch folding:      " << stats.branch_fold_count << "\n";
            std::cout << "  Unreachable removal: " << stats.unreachable_count << "\n";
            std::cout << "  Dead code elim:      " << stats.dce_count << "\n";
            std::cout << "  Jump cleanup:        " << stats.jump_cleanup_count << "\n";
            std::cout << "  Total changes:       " << stats.total_changes() << "\n";
        }

        opt_prog.print(std::cout);
        return 0;
    }

    if (opt_check) {
        if (!sema_ok) return 1;
        cmm::TACGenerator tac_gen;
        cmm::TACProgram orig_prog = tac_gen.generate(program);

        cmm::Optimizer opt(orig_prog);
        cmm::TACProgram opt_prog = opt.optimize(nullptr);

        std::deque<std::string> in1 = has_inputs ? parse_inputs(inputs_arg) : std::deque<std::string>();
        std::deque<std::string> in2 = in1;

        cmm::TACInterpreter interp1(orig_prog);
        auto res1 = interp1.run(in1);

        cmm::TACInterpreter interp2(opt_prog);
        auto res2 = interp2.run(in2);

        std::cout << "=== Original Output ===\n" << res1.output;
        std::cout << "Exit code: " << res1.exit_code << "\n";
        std::cout << "=== Optimized Output ===\n" << res2.output;
        std::cout << "Exit code: " << res2.exit_code << "\n";

        if (res1.output == res2.output && res1.exit_code == res2.exit_code && res1.status == res2.status) {
            std::cout << "PASS\n";
            return 0;
        } else {
            std::cout << "FAIL\n";
            if (res1.output != res2.output) {
                std::cout << "Output mismatch!\n";
            }
            if (res1.exit_code != res2.exit_code) {
                std::cout << "Exit code mismatch: " << res1.exit_code << " vs " << res2.exit_code << "\n";
            }
            if (res1.status != res2.status) {
                std::cout << "Status mismatch: " << res1.status << " vs " << res2.status << "\n";
            }
            return 1;
        }
    }

    return sema_ok ? 0 : 1;
}
