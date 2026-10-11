#include "lexer.h"
#include "parser.h"
#include "ast_printer.h"
#include "sema.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: c-- <file.cmm> [--tokens | --ast | --sema]\n";
        return 1;
    }

    std::string filename = argv[1];
    bool print_tokens = false;
    bool print_ast = false;
    bool run_sema = false;

    if (argc >= 3) {
        std::string arg = argv[2];
        if (arg == "--tokens") print_tokens = true;
        else if (arg == "--ast") print_ast = true;
        else if (arg == "--sema") run_sema = true;
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

    cmm::SemanticAnalyzer sema;
    bool sema_ok = sema.analyze(program);

    for (const auto& diag : sema.get_diagnostics()) {
        if (diag.severity == cmm::DiagnosticSeverity::Error) {
            std::cerr << diag.message << "\n";
        } else {
            std::cout << diag.message << "\n";
        }
    }

    if (run_sema && sema_ok) {
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
    }

    return sema_ok ? 0 : 1;
}
