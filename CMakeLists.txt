cmake_minimum_required(VERSION 3.20)
project(C-- VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

# Library for the compiler components
add_library(cmm_lib
    src/lexer.cpp
    src/parser.cpp
    src/ast_printer.cpp
    src/symbol_table.cpp
    src/sema.cpp
    src/library_registry.cpp
    src/link_check.cpp
    src/tac.cpp
    src/tac_gen.cpp
    src/cfg.cpp
    src/interval.cpp
    src/verifier.cpp
    src/tac_interp.cpp
    src/optimizer.cpp
)
target_include_directories(cmm_lib PUBLIC src)

# Main executable
add_executable(c--
    src/main.cpp
)
target_link_libraries(c-- PRIVATE cmm_lib)

# Test runner
enable_testing()
add_executable(cmm_test
    tests/test_runner.cpp
)
target_link_libraries(cmm_test PRIVATE cmm_lib)
add_test(NAME lexer_tests COMMAND cmm_test)


