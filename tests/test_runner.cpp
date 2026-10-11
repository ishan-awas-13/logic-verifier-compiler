#include "lexer.h"
#include <iostream>
#include <cassert>
#include <fstream>
#include <sstream>

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
    
    std::cout << "All tests passed!\n";
    return 0;
}
