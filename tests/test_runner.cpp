#include "lexer.h"
#include "parser.h"
#include "sema.h"
#include "tac_gen.h"
#include "cfg.h"
#include "library_registry.h"
#include "link_check.h"
#include <iostream>
#include <cassert>
#include <fstream>
#include <sstream>
#include <vector>

void test_executable(const std::string& filename, const std::string& flag, bool expect_error) {
    std::cout << "Testing " << filename << " with " << flag << "...\n";
    std::string cmd = "build\\c--.exe " + filename + " " + flag + " > temp.out 2> temp.err";
    int ret = std::system(cmd.c_str());
    if (expect_error) {
        assert(ret != 0 && "Expected error exit code");
    } else {
        assert(ret == 0 && "Expected success exit code");
    }
}

void test_golden_tac(const std::string& filename, const std::vector<std::string>& expected_snippets) {
    std::cout << "Testing golden --tac for " << filename << "...\n";
    std::string cmd = "build\\c--.exe " + filename + " --tac > temp.out 2> temp.err";
    int ret = std::system(cmd.c_str());
    assert(ret == 0 && "Expected success exit code for golden --tac");
    std::ifstream out("temp.out");
    std::stringstream buf;
    buf << out.rdbuf();
    std::string content = buf.str();
    for (const auto& snippet : expected_snippets) {
        if (content.find(snippet) == std::string::npos) {
            std::cerr << "Golden TAC failed for " << filename << "\n";
            std::cerr << "Expected snippet not found: [" << snippet << "]\n";
            std::cerr << "Actual output:\n" << content << "\n";
            assert(false && "Missing expected golden TAC snippet");
        }
    }
}

void test_golden_cfg(const std::string& filename, const std::vector<std::string>& expected_snippets) {
    std::cout << "Testing golden --cfg for " << filename << "...\n";
    std::string cmd = "build\\c--.exe " + filename + " --cfg > temp.out 2> temp.err";
    int ret = std::system(cmd.c_str());
    assert(ret == 0 && "Expected success exit code for golden --cfg");
    std::ifstream out("temp.out");
    std::stringstream buf;
    buf << out.rdbuf();
    std::string content = buf.str();
    for (const auto& snippet : expected_snippets) {
        if (content.find(snippet) == std::string::npos) {
            std::cerr << "Golden CFG failed for " << filename << "\n";
            std::cerr << "Expected snippet not found: [" << snippet << "]\n";
            std::cerr << "Actual output:\n" << content << "\n";
            assert(false && "Missing expected golden CFG snippet");
        }
    }
}

void test_output_snippets(const std::string& cmd, bool expect_error, const std::vector<std::string>& snippets) {
    int ret = std::system(cmd.c_str());
    if (expect_error) {
        assert(ret != 0 && "Expected error exit code");
    } else {
        assert(ret == 0 && "Expected success exit code");
    }
    std::ifstream out("temp.out");
    std::ifstream err("temp.err");
    std::stringstream buf;
    buf << out.rdbuf() << "\n" << err.rdbuf();
    std::string content = buf.str();
    for (const auto& snippet : snippets) {
        if (content.find(snippet) == std::string::npos) {
            std::cerr << "Command: " << cmd << " failed.\nMissing snippet: [" << snippet << "]\nActual output:\n" << content << "\n";
            assert(false && "Missing expected snippet");
        }
    }
}

void test_cfg_structure(const std::string& filename) {
    std::cout << "Testing CFG structural invariants for " << filename << "...\n";
    std::ifstream file(filename);
    assert(file && "Could not open file for structural test");
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    cmm::Lexer lexer(source);
    auto tokens = lexer.tokenize();
    cmm::Parser parser(tokens);
    auto program = parser.parse();
    assert(!parser.has_error());

    cmm::SemanticAnalyzer sema;
    bool sema_ok = sema.analyze(program);
    assert(sema_ok);

    cmm::TACGenerator tac_gen;
    auto tac_prog = tac_gen.generate(program);

    cmm::CFG cfg = cmm::CFG::build(tac_prog);
    std::string err;
    bool valid = cfg.validate_structure(err);
    if (!valid) {
        std::cerr << "CFG structure validation failed for " << filename << ": " << err << "\n";
    }
    assert(valid && "CFG structure validation failed");
}

int main() {
    // AST / Syntax tests
    test_executable("examples/hello.cmm", "--ast", false);
    test_executable("examples/loop.cmm", "--ast", false);
    test_executable("examples/cond.cmm", "--ast", false);
    test_executable("examples/expr.cmm", "--ast", false);
    test_executable("examples/game_combat.cmm", "--ast", false);
    test_executable("examples/countdown.cmm", "--ast", false);
    test_executable("examples/student_grades.cmm", "--ast", false);
    
    test_executable("examples/error.cmm", "--ast", true);
    test_executable("examples/err_missing_brace.cmm", "--ast", true);
    test_executable("examples/err_forge.cmm", "--ast", true);
    test_executable("examples/err_check_no_block.cmm", "--ast", true);
    test_executable("examples/err_emit_nothing.cmm", "--ast", true);
    test_executable("examples/err_multi.cmm", "--ast", true);

    // Semantic Analysis valid programs
    test_executable("examples/game_combat.cmm", "--sema", false);
    test_executable("examples/countdown.cmm", "--sema", false);
    test_executable("examples/student_grades.cmm", "--sema", false);
    test_executable("examples/sema_ok.cmm", "--sema", false);
    test_executable("examples/sema_unused.cmm", "--sema", false);
    test_executable("examples/sema_unreachable.cmm", "--sema", false);

    // Semantic Analysis error programs
    test_executable("examples/sema_undeclared.cmm", "--sema", true);
    test_executable("examples/sema_redeclare.cmm", "--sema", true);
    test_executable("examples/sema_type_mismatch.cmm", "--sema", true);
    test_executable("examples/sema_fixed_assign.cmm", "--sema", true);
    test_executable("examples/sema_bad_operator.cmm", "--sema", true);
    test_executable("examples/sema_cond_not_boolean.cmm", "--sema", true);
    test_executable("examples/sema_stop_outside_loop.cmm", "--sema", true);
    test_executable("examples/sema_input_misuse.cmm", "--sema", true);
    test_executable("examples/sema_decimal_to_num.cmm", "--sema", true);
    test_executable("examples/sema_multi.cmm", "--sema", true);
    test_executable("examples/sema_err_mod_decimal.cmm", "--sema", true);
    test_executable("examples/sema_err_logical_nonbool.cmm", "--sema", true);
    test_executable("examples/sema_err_not_num.cmm", "--sema", true);
    test_executable("examples/sema_err_equals_word_num.cmm", "--sema", true);
    test_executable("examples/sema_err_inc_word.cmm", "--sema", true);
    test_executable("examples/sema_err_inc_fixed.cmm", "--sema", true);
    test_executable("examples/sema_err_give_non_num.cmm", "--sema", true);
    test_executable("examples/sema_err_show_list.cmm", "--sema", true);
    test_executable("examples/sema_err_list_assign_num.cmm", "--sema", true);
    test_executable("examples/sema_err_list_compare.cmm", "--sema", true);

    // Three-Address Code (TAC) generation tests
    test_executable("examples/game_combat.cmm", "--tac", false);
    test_executable("examples/countdown.cmm", "--tac", false);
    test_executable("examples/student_grades.cmm", "--tac", false);
    test_executable("examples/tac_test_shadow.cmm", "--tac", false);
    test_executable("examples/tac_test_input.cmm", "--tac", false);
    test_executable("examples/hello.cmm", "--tac", false);
    test_executable("examples/loop.cmm", "--tac", false);
    test_executable("examples/cond.cmm", "--tac", false);
    test_executable("examples/expr.cmm", "--tac", false);
    test_executable("examples/tac_arith.cmm", "--tac", false);
    test_executable("examples/tac_logic.cmm", "--tac", false);
    test_executable("examples/tac_loops.cmm", "--tac", false);
    test_executable("examples/tac_incdec.cmm", "--tac", false);
    test_executable("examples/tac_or_not.cmm", "--tac", false);
    test_executable("examples/sema_type_mismatch.cmm", "--tac", true);

    // Golden TAC tests
    test_golden_tac("examples/tac_logic.cmm", {
        "t1 = d != 0",
        "if_false t2 goto L1",
        "t3 = a / d",
        "halt"
    });
    test_golden_tac("examples/tac_arith.cmm", {
        "t1 = i2d a",
        "d = t1",
        "t2 = i2d a",
        "t3 = t2 + 2.5",
        "e = t3"
    });
    test_golden_tac("examples/tac_loops.cmm", {
        "if_true t1 goto L2",
        "goto L1",
        "if_true t3 goto L5",
        "goto L5",
        "goto L4"
    });
    test_golden_tac("examples/tac_incdec.cmm", {
        "a = a + 1",
        "a = a - 1",
        "d = d + 1.0",
        "d = d - 1.0"
    });
    test_golden_tac("examples/tac_or_not.cmm", {
        "if_true t2 goto L1",
        "if_true t3 goto L3",
        "t5 = not c"
    });

    // Control Flow Graph (CFG) execution & DOT tests
    test_executable("examples/countdown.cmm", "--cfg", false);
    test_executable("examples/tac_logic.cmm", "--cfg", false);
    test_executable("examples/tac_loops.cmm", "--cfg", false);
    test_executable("examples/game_combat.cmm", "--cfg", false);
    test_executable("examples/cfg_unreachable.cmm", "--cfg", false);
    test_executable("examples/countdown.cmm", "--dot", false);
    test_executable("examples/tac_logic.cmm", "--dot", false);
    test_executable("examples/cfg_unreachable.cmm", "--dot", false);
    test_executable("examples/sema_type_mismatch.cmm", "--cfg", true);
    test_executable("examples/sema_type_mismatch.cmm", "--dot", true);

    // Golden CFG tests
    test_golden_cfg("examples/countdown.cmm", {
        "B0  succ: B1  pred:",
        "B1 [L1]  succ: B2, B3  pred: B0, B2",
        "B2  succ: B1  pred: B1",
        "B3 [L2]  succ:  pred: B1"
    });
    test_golden_cfg("examples/tac_logic.cmm", {
        "B0  succ: B1, B2  pred:",
        "B1  succ: B2  pred: B0",
        "B2 [L1]  succ: B3, B4  pred: B0, B1",
        "B3  succ: B4  pred: B2",
        "B4 [L2]  succ:  pred: B2, B3"
    });
    test_golden_cfg("examples/tac_loops.cmm", {
        "B0  succ: B1  pred:",
        "B1 [L1]  succ: B2, B10  pred: B0, B3, B9",
        "B5 [L4]  succ: B6, B9  pred: B4, B8",
        "B10 [L2]  succ:  pred: B1"
    });
    test_golden_cfg("examples/game_combat.cmm", {
        "B2 [L1]  succ: B3, B4  pred: B0, B1",
        "B5 [L3]  succ:  pred: B3, B4",
        "B6 (unreachable)  succ:  pred:"
    });
    test_golden_cfg("examples/cfg_unreachable.cmm", {
        "B4 (unreachable)  succ: B5  pred:",
        "B7 (unreachable)  succ:  pred:"
    });

    // CFG Structural invariant checks for every compiling program
    std::vector<std::string> compiling_examples = {
        "examples/hello.cmm",
        "examples/loop.cmm",
        "examples/cond.cmm",
        "examples/expr.cmm",
        "examples/game_combat.cmm",
        "examples/countdown.cmm",
        "examples/student_grades.cmm",
        "examples/sema_ok.cmm",
        "examples/sema_unused.cmm",
        "examples/sema_unreachable.cmm",
        "examples/tac_arith.cmm",
        "examples/tac_logic.cmm",
        "examples/tac_loops.cmm",
        "examples/tac_incdec.cmm",
        "examples/tac_or_not.cmm",
        "examples/tac_test_input.cmm",
        "examples/tac_test_shadow.cmm",
        "examples/cfg_unreachable.cmm"
    };

    for (const auto& file : compiling_examples) {
        test_cfg_structure(file);
    }

    // Stage 6: Logic Verifier tests
    auto test_verifier = [](const std::string& filename, const std::string& flag, bool expect_error, const std::vector<std::string>& snippets) {
        std::cout << "Testing verifier for " << filename << " with " << flag << "...\n";
        std::string cmd = "build\\c--.exe " + filename + " " + flag + " > temp.out 2>&1";
        int ret = std::system(cmd.c_str());
        if (expect_error) {
            assert(ret != 0 && "Expected error exit code for verifier");
        } else {
            assert(ret == 0 && "Expected success exit code for verifier");
        }
        std::ifstream out("temp.out");
        std::stringstream buf;
        buf << out.rdbuf();
        std::string content = buf.str();
        for (const auto& snippet : snippets) {
            if (content.find(snippet) == std::string::npos) {
                std::cerr << "Verifier test failed for " << filename << "\n";
                std::cerr << "Expected snippet not found: [" << snippet << "]\n";
                std::cerr << "Actual output:\n" << content << "\n";
                assert(false && "Missing expected verifier snippet");
            }
        }
    };

    // LV01: contradiction
    test_verifier("examples/lv_contradiction.cmm", "--verify", false, {
        "[LV01] condition is always false: x > 10 contradicts x < 5 (branch never runs)",
        "0 error(s), 1 warning(s)"
    });
    // LV01: known constants
    test_verifier("examples/lv_known.cmm", "--verify", false, {
        "[LV01] condition is always false: 'MAX' is always 10 here, so MAX > 20 is false (branch never runs)",
        "[LV01] condition is always false: 'f' is always true here, so not f is false (branch never runs)",
        "0 error(s), 2 warning(s)"
    });
    // LV02: tautology
    test_verifier("examples/lv_tautology.cmm", "--verify", false, {
        "[LV02] condition is always true: x > 5 or x <= 5 covers every value (otherwise never runs)",
        "0 error(s), 2 warning(s)"
    });
    // LV03: self comparison
    test_verifier("examples/lv_self.cmm", "--verify", false, {"[LV03]", "0 error(s), 1 warning(s)"});
    // LV04: redundant operand
    test_verifier("examples/lv_redundant.cmm", "--verify", false, {"[LV04]", "0 error(s), 1 warning(s)"});
    // LV05: loop body never runs
    test_verifier("examples/lv_loop_never_runs.cmm", "--verify", false, {"[LV05]", "0 error(s), 1 warning(s)"});
    // LV06: loop never terminates (ERROR)
    test_verifier("examples/lv_loop_const.cmm", "--verify", true, {"[LV06]", "1 error(s), 0 warning(s)"});
    test_verifier("examples/lv_loop_unchanged.cmm", "--verify", true, {"[LV06]", "1 error(s), 0 warning(s)"});
    // LV07: step never reaches exit (ERROR)
    test_verifier("examples/lv_loop_overshoot.cmm", "--verify", true, {"[LV07]", "1 error(s), 0 warning(s)"});
    // LV08: wrong direction (ERROR)
    test_verifier("examples/lv_loop_direction.cmm", "--verify", true, {"[LV08]", "1 error(s), 0 warning(s)"});
    // LV09: unreachable post-loop
    test_verifier("examples/lv_after_loop.cmm", "--verify", true, {"[LV06]", "[LV09]", "1 error(s), 1 warning(s)"});
    // LV10: div by zero (ERROR)
    test_verifier("examples/lv_div_zero.cmm", "--verify", true, {"[LV10]", "1 error(s), 0 warning(s)"});
    // LV11: possible div by zero
    test_verifier("examples/lv_div_input.cmm", "--verify", false, {"[LV11]", "0 error(s), 1 warning(s)"});

    // Zero-finding programs (clean verification)
    test_verifier("examples/lv_loop_ok.cmm", "--verify", false, {"0 error(s), 0 warning(s)"});
    test_verifier("examples/lv_loop_stop_ok.cmm", "--verify", false, {"0 error(s), 0 warning(s)"});
    test_verifier("examples/lv_div_guarded.cmm", "--verify", false, {"0 error(s), 0 warning(s)"});
    test_verifier("examples/lv_good.cmm", "--verify", false, {"0 error(s), 0 warning(s)"});
    test_verifier("examples/countdown.cmm", "--verify", false, {"0 error(s), 0 warning(s)"});

    // Strict mode: warnings become errors
    test_verifier("examples/lv_contradiction.cmm", "--verify --strict", true, {"error: [LV01]", "1 error(s), 0 warning(s)"});

    // Trace flag works
    test_verifier("examples/lv_loop_ok.cmm", "--verify --trace", false, {"Dataflow Entry States:", "0 error(s), 0 warning(s)"});

    // Stage 7: Optimizer and Interpreter tests
    auto test_golden_opt = [](const std::string& filename, const std::string& flags, const std::vector<std::string>& snippets) {
        std::cout << "Testing golden --opt for " << filename << " " << flags << "...\n";
        std::string cmd = "build\\c--.exe " + filename + " " + flags + " > temp.out 2> temp.err";
        int ret = std::system(cmd.c_str());
        assert(ret == 0 && "Expected success exit code for golden --opt");
        std::ifstream out("temp.out");
        std::stringstream buf;
        buf << out.rdbuf();
        std::string content = buf.str();
        for (const auto& snippet : snippets) {
            if (content.find(snippet) == std::string::npos) {
                std::cerr << "Golden opt failed for " << filename << "\n";
                std::cerr << "Expected snippet not found: [" << snippet << "]\n";
                std::cerr << "Actual output:\n" << content << "\n";
                assert(false && "Missing expected golden opt snippet");
            }
        }
    };

    auto test_run_tac = [](const std::string& filename, const std::string& flags, bool expect_error, const std::vector<std::string>& snippets) {
        std::cout << "Testing --run-tac for " << filename << " " << flags << "...\n";
        std::string cmd = "build\\c--.exe " + filename + " " + flags + " > temp.out 2>&1";
        int ret = std::system(cmd.c_str());
        if (expect_error) {
            assert(ret != 0 && "Expected error exit code for --run-tac");
        } else {
            assert(ret == 0 && "Expected success exit code for --run-tac");
        }
        std::ifstream out("temp.out");
        std::stringstream buf;
        buf << out.rdbuf();
        std::string content = buf.str();
        for (const auto& snippet : snippets) {
            if (content.find(snippet) == std::string::npos) {
                std::cerr << "Run TAC test failed for " << filename << "\n";
                std::cerr << "Expected snippet not found: [" << snippet << "]\n";
                std::cerr << "Actual output:\n" << content << "\n";
                assert(false && "Missing expected --run-tac snippet");
            }
        }
    };

    auto test_optcheck = [](const std::string& filename, const std::string& inputs) {
        std::cout << "Testing --optcheck for " << filename << (inputs.empty() ? "" : (" with inputs " + inputs)) << "...\n";
        std::string cmd = "build\\c--.exe " + filename + " --optcheck";
        if (!inputs.empty()) {
            cmd += " --inputs \"" + inputs + "\"";
        }
        cmd += " > temp.out 2>&1";
        int ret = std::system(cmd.c_str());
        assert(ret == 0 && "Expected success exit code for --optcheck");
        std::ifstream out("temp.out");
        std::stringstream buf;
        buf << out.rdbuf();
        std::string content = buf.str();
        assert(content.find("PASS") != std::string::npos && "Expected PASS in --optcheck");
    };

    // Golden --opt tests
    test_golden_opt("examples/opt_fold.cmm", "--opt", {"show 14", "halt"});
    test_golden_opt("examples/opt_prop.cmm", "--opt", {"show 12", "halt"});
    test_golden_opt("examples/opt_algebra.cmm", "--opt", {"input.num", "show n", "halt"});
    test_golden_opt("examples/opt_branch.cmm", "--opt", {"show \"yes\"", "halt"});
    test_golden_opt("examples/opt_dead.cmm", "--opt", {"show 10", "halt"});
    test_golden_opt("examples/opt_jump.cmm", "--opt", {"if_false t1 goto L1", "if_false t2 goto L3", "show \"huge\"", "show \"big\"", "show \"small\""});
    test_golden_opt("examples/opt_loops.cmm", "--opt", {"i = 0", "if_true t1 goto L2", "if_false t3 goto L3", "if_false t4 goto L4", "show i"});
    test_golden_opt("examples/opt_decimal.cmm", "--opt", {"show 4.0", "halt"});
    test_golden_opt("examples/opt_divzero.cmm", "--opt --no-verify", {"10 / 0", "show t1"});
    test_golden_opt("examples/opt_input_kept.cmm", "--opt", {"n = input.num", "halt"});

    // Golden --opt --stats tests
    test_golden_opt("examples/countdown.cmm", "--opt --stats", {"Optimization Stats:", "Rounds:", "Instructions before:", "Instructions after:"});
    test_golden_opt("examples/game_combat.cmm", "--opt --stats", {"Optimization Stats:", "Rounds:", "Instructions before:", "Instructions after:", "Total changes:"});

    // Interpreter --run-tac tests
    test_run_tac("examples/countdown.cmm", "--run-tac", false, {"Initiating launch sequence...", "5", "4", "3", "2", "1", "Liftoff!"});
    test_run_tac("examples/tac_logic.cmm", "--run-tac", false, {});
    test_run_tac("examples/tac_loops.cmm", "--run-tac", false, {"5"});
    test_run_tac("examples/student_grades.cmm", "--run-tac", false, {"Awarding Honors Citation!"});
    test_run_tac("examples/game_combat.cmm", "--run-tac", false, {"Space Arena", "Shield absorbed the damage!", "90"});
    test_run_tac("examples/rt_divzero.cmm", "--run-tac", true, {"runtime error: division by zero"});
    test_run_tac("examples/rt_step_limit.cmm", "--run-tac", true, {"step limit exceeded"});

    // Opt refusing program with verifier error
    test_executable("examples/lv_loop_overshoot.cmm", "--opt", true);

    // Equivalence --optcheck tests
    test_optcheck("examples/countdown.cmm", "");
    test_optcheck("examples/game_combat.cmm", "");
    test_optcheck("examples/student_grades.cmm", "");
    test_optcheck("examples/opt_fold.cmm", "");
    test_optcheck("examples/opt_prop.cmm", "");
    test_optcheck("examples/opt_branch.cmm", "");
    test_optcheck("examples/opt_loops.cmm", "");
    test_optcheck("examples/opt_dead.cmm", "");
    test_optcheck("examples/hello.cmm", "");
    test_optcheck("examples/cond.cmm", "");
    test_optcheck("examples/expr.cmm", "");

    // Programs with input: run multiple test inputs
    test_optcheck("examples/opt_algebra.cmm", "5");
    test_optcheck("examples/opt_algebra.cmm", "0");
    test_optcheck("examples/opt_algebra.cmm", "-12");

    test_optcheck("examples/opt_jump.cmm", "5");
    test_optcheck("examples/opt_jump.cmm", "15");
    test_optcheck("examples/opt_jump.cmm", "25");

    test_optcheck("examples/tac_test_input.cmm", "42,3.14,true,Z,hello");

    // Milestone 1: IMPORT, LOAD and LINK tests
    test_verifier("examples/import_ok.cmm", "--verify", false, {"Load and Link: 0 error(s), 0 warning(s)"});
    test_verifier("examples/import_core_only.cmm", "--verify", false, {"Load and Link: 0 error(s), 0 warning(s)"});
    test_verifier("examples/link_missing_word.cmm", "--verify", true, {"[LV12]", "Pipeline stopped before intermediate code."});
    test_verifier("examples/link_missing_list.cmm", "--verify", true, {"[LV12]", "Pipeline stopped before intermediate code."});
    test_verifier("examples/load_unknown.cmm", "--verify", true, {"[LV13]", "Available libraries: text, vector, math", "Pipeline stopped before intermediate code."});
    test_verifier("examples/load_dup.cmm", "--verify", false, {"[LV15]", "Load and Link: 0 error(s), 1 warning(s)"});
    test_verifier("examples/load_unused.cmm", "--verify", false, {"[LV16]", "Load and Link: 0 error(s), 1 warning(s)"});
    test_verifier("examples/load_multi.cmm", "--verify", true, {"[LV13]", "[LV12]", "Pipeline stopped before intermediate code."});
    test_executable("examples/import_after_stmt.cmm", "--ast", true);
    test_executable("examples/import_no_name.cmm", "--ast", true);
    test_verifier("examples/link_blocks_logic.cmm", "--verify", true, {"[LV12]", "Pipeline stopped before intermediate code."});
    test_verifier("examples/link_blocks_logic_fixed.cmm", "--verify", false, {"[LV01]", "Load and Link: 0 error(s), 0 warning(s)"});
    test_verifier("examples/strict_link.cmm", "--verify --strict", true, {"[LV15]", "Pipeline stopped before intermediate code."});

    // Test --link output format
    test_output_snippets("build\\c--.exe examples/import_ok.cmm --link > temp.out 2> temp.err", false, {
        "Libraries loaded: text (line 1)",
        "Link map:",
        "word     type      -> text   resolved"
    });

    // Test --libs output
    test_output_snippets("build\\c--.exe --libs > temp.out 2> temp.err", false, {
        "text:",
        "String and character manipulation",
        "length(word) -> num",
        "vector:",
        "math:",
        "maximum(num, num) -> num"
    });

    // Step 1 Proof Test: Adding a new library needs ONLY one new table entry with zero changes to parser, link checks, or messages
    {
        cmm::LibraryRegistry custom_reg;
        cmm::LibraryDef crypto_lib;
        crypto_lib.name = "crypto";
        crypto_lib.description = "Cryptographic functions";
        crypto_lib.provided_types = {};
        crypto_lib.functions = {
            cmm::FunctionDef{"hash", {cmm::Type::Num}, cmm::Type::Num, "crypto.hash"}
        };
        custom_reg.addLibrary(crypto_lib);

        assert(custom_reg.findLibrary("crypto") != nullptr);
        assert(custom_reg.providerOfFunction("hash") == "crypto");

        cmm::Program prog;
        prog.imports.push_back(cmm::Import{"crypto", 1, 8});
        std::vector<cmm::LibUse> uses = {
            cmm::LibUse{"hash", cmm::LibUseKind::Function, "crypto", 2, 5}
        };
        cmm::LinkChecker custom_checker(prog, uses, custom_reg);
        std::stringstream ss_out, ss_err;
        bool ok = custom_checker.check(false, ss_out, ss_err);
        assert(ok && "Custom library must resolve cleanly without code changes");
        assert(custom_checker.error_count() == 0);
        assert(custom_checker.warning_count() == 0);
        assert(custom_checker.get_link_map().size() == 1);
        assert(custom_checker.get_link_map()[0].status == "resolved");
    }

    // Milestone 2: Function calls, end-to-end execution, and error handling
    test_run_tac("examples/demo_import_ok.cmm", "--run-tac", false, {
        "String length:", "12", "Is empty:", "false", "Absolute value:", "42", "Maximum:", "99", "Minimum:", "15"
    });
    test_optcheck("examples/demo_import_ok.cmm", "");
    test_golden_tac("examples/demo_import_ok.cmm", {
        "t1 = call length(greeting)",
        "t2 = call isempty(greeting)",
        "t4 = call absolute(a)",
        "t5 = call maximum(m1, m2)",
        "t6 = call minimum(m1, m2)"
    });
    test_verifier("examples/demo_missing_import.cmm", "--verify", true, {"[LV12]", "Pipeline stopped before intermediate code."});
    test_verifier("examples/demo_unknown_library.cmm", "--verify", true, {"[LV13]", "Pipeline stopped before intermediate code."});
    test_verifier("examples/demo_undefined_function.cmm", "--verify", true, {"[LV14]", "Did you mean 'length' (library 'text')?", "Pipeline stopped before intermediate code."});
    test_verifier("examples/link_missing_func.cmm", "--verify", true, {"[LV12]", "library 'math' is not imported, but function 'maximum' is used here", "Pipeline stopped before intermediate code."});
    test_output_snippets("build\\c--.exe examples/link_map_test.cmm --link > temp.out 2> temp.err", true, {
        "length   function  -> text   resolved",
        "maximum  function  -> math   NOT IMPORTED"
    });
    test_executable("examples/sema_err_call_arity.cmm", "--sema", true);
    test_executable("examples/sema_err_call_type.cmm", "--sema", true);

    std::cout << "All tests passed successfully!\n";
    return 0;
}
