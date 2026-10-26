import os
import sys
from reportlab.lib.pagesizes import letter
from reportlab.lib import colors
from reportlab.lib.units import inch
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.platypus import (
    SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, PageBreak, KeepTogether, HRFlowable
)
from reportlab.graphics.shapes import Drawing, Rect, String, Line, Group
from reportlab.pdfgen import canvas
import pypdf

class NumberedCanvas(canvas.Canvas):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._saved_page_states = []

    def showPage(self):
        self._saved_page_states.append(dict(self.__dict__))
        self._startPage()

    def save(self):
        num_pages = len(self._saved_page_states)
        for state in self._saved_page_states:
            self.__dict__.update(state)
            self.draw_page_decorations(num_pages)
            super().showPage()
        super().save()

    def draw_page_decorations(self, page_count):
        self.saveState()
        self.setFont("Helvetica-Bold", 8)
        self.setFillColor(colors.HexColor("#334155"))
        
        # Header (pages > 1)
        if self._pageNumber > 1:
            self.drawString(54, 11 * inch - 36, "J-- COMPILER: CODEBASE ARCHITECTURE & SUBSYSTEM DEEP-DIVE")
            self.drawRightString(8.5 * inch - 54, 11 * inch - 36, "IMPLEMENTATION BLUEPRINT & LOGIC PROOFS")
            self.setStrokeColor(colors.HexColor("#CBD5E1"))
            self.setLineWidth(0.75)
            self.line(54, 11 * inch - 42, 8.5 * inch - 54, 11 * inch - 42)

        # Footer (all pages)
        self.setStrokeColor(colors.HexColor("#CBD5E1"))
        self.setLineWidth(0.75)
        self.line(54, 46, 8.5 * inch - 54, 46)
        
        self.setFont("Helvetica", 8)
        self.setFillColor(colors.HexColor("#64748B"))
        self.drawString(54, 32, "Utkarsh Vikram Singh (24BCE2611) & Ishan Awasthi (24BAI0207) | J-- System Architects")
        page_text = f"Page {self._pageNumber} of {page_count}"
        self.drawRightString(8.5 * inch - 54, 32, page_text)
        self.restoreState()

def build_codebase_guide_pdf(output_path):
    doc = SimpleDocTemplate(
        output_path,
        pagesize=letter,
        leftMargin=54,
        rightMargin=54,
        topMargin=54,
        bottomMargin=54
    )

    styles = getSampleStyleSheet()

    title_style = ParagraphStyle(
        'DocTitle',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=20,
        leading=24,
        textColor=colors.HexColor("#0F172A"),
        spaceAfter=3
    )

    subtitle_style = ParagraphStyle(
        'DocSubtitle',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=10,
        leading=13.5,
        textColor=colors.HexColor("#334155"),
        spaceAfter=8
    )

    author_style = ParagraphStyle(
        'AuthorStyle',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=8,
        leading=11,
        textColor=colors.HexColor("#1E293B")
    )

    h1_style = ParagraphStyle(
        'Heading1_Custom',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=12,
        leading=15,
        textColor=colors.HexColor("#0F172A"),
        spaceBefore=10,
        spaceAfter=5,
        keepWithNext=True
    )

    h2_style = ParagraphStyle(
        'Heading2_Custom',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=9.5,
        leading=12.5,
        textColor=colors.HexColor("#1E293B"),
        spaceBefore=7,
        spaceAfter=3,
        keepWithNext=True
    )

    body_style = ParagraphStyle(
        'Body_Custom',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=8.2,
        leading=11.2,
        textColor=colors.HexColor("#334155"),
        spaceAfter=4
    )

    code_style = ParagraphStyle(
        'CodeStyle',
        parent=styles['Normal'],
        fontName='Courier',
        fontSize=7.2,
        leading=9.2,
        textColor=colors.HexColor("#0F172A")
    )

    table_cell = ParagraphStyle(
        'TableCell',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=7.5,
        leading=9.5,
        textColor=colors.HexColor("#1E293B")
    )

    table_cell_bold = ParagraphStyle(
        'TableCellBold',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=7.5,
        leading=9.5,
        textColor=colors.HexColor("#0F172A")
    )

    callout_style = ParagraphStyle(
        'Callout',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=8,
        leading=11,
        textColor=colors.HexColor("#1E3A8A")
    )

    story = []

    # ========================================================
    # TITLE & EXECUTIVE HEADER
    # ========================================================
    story.append(Paragraph("J-- Compiler Architecture &amp; Implementation Guide", title_style))
    story.append(Paragraph("Comprehensive Subsystem Deep-Dive: Logic Verifier Dataflow, Modular Load &amp; Linker Mechanics, Regex Scanning, and File Directory", subtitle_style))

    author_card = """
    <b>Principal Authors &amp; System Architects:</b><br/>
    &bull; <b>Utkarsh Vikram Singh</b> (Registration No: <b>24BCE2611</b>) - Core Pipeline, Abstract Interpretation &amp; Verifier Engine<br/>
    &bull; <b>Ishan Awasthi</b> (Registration No: <b>24BAI0207</b>) - Modular Load &amp; Link Registry, CFG Dataflow &amp; Optimizer Framework<br/>
    <b>Technical Purpose:</b> In-depth architectural blueprint and codebase walkthrough for examiners and engineers auditing the J-- compiler source repository.<br/>
    <b>Target Codebase:</b> J-- Systems Compiler (C++20, Intermediate Three-Address IR, Abstract Interpretation Lattice, Multi-Pass Canonical Optimizer).
    """
    author_table = Table([[Paragraph(author_card, author_style)]], colWidths=[504])
    author_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor("#F8FAFC")),
        ('BOX', (0,0), (-1,-1), 1, colors.HexColor("#CBD5E1")),
        ('PADDING', (0,0), (-1,-1), 6),
    ]))
    story.append(author_table)
    story.append(Spacer(1, 8))

    # ========================================================
    # SECTION 1: COMPLETE DIRECTORY MAP & FILE RESPONSIBILITY TABLE
    # ========================================================
    story.append(Paragraph("1. Codebase Directory Layout &amp; File Responsibility Directory", h1_style))
    story.append(Paragraph(
        "The J-- compiler source tree is organized into strictly decoupled directories: <code>src/</code> contains the compiler core engine; <code>build/</code> contains CMake build outputs and executable binaries (<code>c--.exe</code>); <code>examples/</code> hosts demonstration programs; and <code>tests/</code> holds the ground-truth test suite (<code>test_runner.cpp</code>). The table below details every single file in the implementation:",
        body_style
    ))

    file_matrix_data = [
        [Paragraph("File Path", table_cell_bold), Paragraph("Lines", table_cell_bold), Paragraph("Primary Architectural Responsibility", table_cell_bold), Paragraph("Key Classes, Structs &amp; APIs Exported", table_cell_bold)],
        [
            Paragraph("<code>src/main.cpp</code>", table_cell),
            Paragraph("345", table_cell),
            Paragraph("Compiler CLI entry point, argument parsing, 9-stage pipeline dispatch, quarantine orchestration, and stdout/stderr reporting.", table_cell),
            Paragraph("<code>main()</code>, <code>parse_inputs()</code>, flags: <code>--verify</code>, <code>--link</code>, <code>--opt</code>, <code>--run-tac</code>, <code>--optcheck</code>", table_cell)
        ],
        [
            Paragraph("<code>src/lexer.h<br/>src/lexer.cpp</code>", table_cell),
            Paragraph("320", table_cell),
            Paragraph("Lexical scanner; transforms raw J-- source text into typed token stream. Strips comments and whitespace.", table_cell),
            Paragraph("<code>class Lexer</code>, <code>keywords</code> table, <code>tokenize()</code>, <code>number()</code>, <code>string()</code>, <code>identifier()</code>", table_cell)
        ],
        [
            Paragraph("<code>src/token.h</code>", table_cell),
            Paragraph("52", table_cell),
            Paragraph("Token data structures and enumeration defining all lexical token types, line/column markers, and string lexeme views.", table_cell),
            Paragraph("<code>enum class TokenType</code>, <code>struct Token</code> (type, lexeme, line, col), <code>to_string(TokenType)</code>", table_cell)
        ],
        [
            Paragraph("<code>src/parser.h<br/>src/parser.cpp</code>", table_cell),
            Paragraph("610", table_cell),
            Paragraph("Recursive-descent syntax analyzer. Enforces grammar rules: top-level imports preceding statements, expression precedence climbing.", table_cell),
            Paragraph("<code>class Parser</code>, <code>parse()</code>, <code>parse_import()</code>, <code>statement()</code>, <code>expression()</code>", table_cell)
        ],
        [
            Paragraph("<code>src/ast.h</code>", table_cell),
            Paragraph("280", table_cell),
            Paragraph("Abstract Syntax Tree class hierarchy representing statements, expressions, import nodes, binary operators, and literals.", table_cell),
            Paragraph("<code>struct Program</code> (imports, statements), <code>StmtNode</code>, <code>ExprNode</code>, <code>BinaryExpr</code>, <code>CheckStmt</code>, <code>UntilStmt</code>", table_cell)
        ],
        [
            Paragraph("<code>src/ast_printer.h<br/>src/ast_printer.cpp</code>", table_cell),
            Paragraph("150", table_cell),
            Paragraph("Pretty-printer for AST visualization emitted during compiler <code>--ast</code> debug mode.", table_cell),
            Paragraph("<code>class AstPrinter</code>, <code>print(Program)</code>, indented node tree serialization", table_cell)
        ],
        [
            Paragraph("<code>src/types.h</code>", table_cell),
            Paragraph("55", table_cell),
            Paragraph("J-- type system definition; primitive types (num, decimal, word, letter, boolean, list, void) and type formatting.", table_cell),
            Paragraph("<code>enum class Type</code>, <code>to_string(Type)</code>, <code>type_from_string()</code>", table_cell)
        ],
        [
            Paragraph("<code>src/symbol_table.h<br/>src/symbol_table.cpp</code>", table_cell),
            Paragraph("90", table_cell),
            Paragraph("Scoped symbol table for lexical declarations, type resolution, mutability tracking (fixed), and declaration coordinates.", table_cell),
            Paragraph("<code>class SymbolTable</code>, <code>struct Symbol</code>, <code>enter_scope()</code>, <code>exit_scope()</code>, <code>lookup()</code>", table_cell)
        ],
        [
            Paragraph("<code>src/sema.h<br/>src/sema.cpp</code>", table_cell),
            Paragraph("510", table_cell),
            Paragraph("Semantic analyzer. Performs static type-checking and collects the <code>LibUse</code> side-table for the Load &amp; Link stage.", table_cell),
            Paragraph("<code>class SemanticAnalyzer</code>, <code>analyze()</code>, <code>get_lib_uses()</code>, <code>check_types()</code>", table_cell)
        ],
        [
            Paragraph("<code>src/library_registry.h<br/>src/library_registry.cpp</code>", table_cell),
            Paragraph("160", table_cell),
            Paragraph("<b>LOAD &amp; LINK CORE:</b> Built-in data-driven module registry defining standard libraries (text, math, vector) and intrinsic bindings.", table_cell),
            Paragraph("<code>class LibraryRegistry</code>, <code>struct LibraryDef</code>, <code>struct FunctionDef</code>, <code>findLibrary()</code>, <code>providerOfType()</code>", table_cell)
        ],
        [
            Paragraph("<code>src/link_check.h<br/>src/link_check.cpp</code>", table_cell),
            Paragraph("295", table_cell),
            Paragraph("<b>LOAD &amp; LINK VERIFIER:</b> Audits modular defects (LV12 - LV16), executes fuzzy Levenshtein typo suggestion, and quarantine halting.", table_cell),
            Paragraph("<code>class LinkChecker</code>, <code>levenshtein_distance()</code>, <code>run_checks()</code>, <code>print_link_report()</code>", table_cell)
        ],
        [
            Paragraph("<code>src/tac.h<br/>src/tac.cpp</code>", table_cell),
            Paragraph("290", table_cell),
            Paragraph("Intermediate Three-Address Code data representation; instructions, operands, labels, and IR formatting.", table_cell),
            Paragraph("<code>struct TACInstr</code>, <code>struct TACOperand</code>, <code>enum class TACOp</code> (Add, Sub, Mul, Div, Mod, Jmp, JmpIf, Call)", table_cell)
        ],
        [
            Paragraph("<code>src/tac_gen.h<br/>src/tac_gen.cpp</code>", table_cell),
            Paragraph("470", table_cell),
            Paragraph("Translates validated AST nodes into linearized Three-Address Code intermediate representation.", table_cell),
            Paragraph("<code>class TACGenerator</code>, <code>generate(Program)</code>, temporary register allocator (<code>t0, t1...</code>), label generator", table_cell)
        ],
        [
            Paragraph("<code>src/cfg.h<br/>src/cfg.cpp</code>", table_cell),
            Paragraph("340", table_cell),
            Paragraph("Partitions linearized TAC instructions into Basic Blocks; establishes forward and backward control flow edges.", table_cell),
            Paragraph("<code>class CFG</code>, <code>struct BasicBlock</code>, <code>build(TACProgram)</code>, predecessor/successor graph, loop back-edges", table_cell)
        ],
        [
            Paragraph("<code>src/interval.h<br/>src/interval.cpp</code>", table_cell),
            Paragraph("175", table_cell),
            Paragraph("<b>LOGIC VERIFIER LATTICE:</b> Mathematical interval domain D_int = [lo, hi]; implements interval arithmetic, Join, Meet, and containment.", table_cell),
            Paragraph("<code>class Interval</code>, <code>join()</code>, <code>meet()</code>, <code>contains_zero()</code>, arithmetic: <code>+</code>, <code>-</code>, <code>*</code>, <code>/</code>", table_cell)
        ],
        [
            Paragraph("<code>src/abstract_state.h</code>", table_cell),
            Paragraph("267", table_cell),
            Paragraph("<b>LOGIC VERIFIER LATTICE:</b> Abstract variable store; Tri-State boolean logic (True, False, Unknown) and abstract value representation.", table_cell),
            Paragraph("<code>enum class TriBool</code>, <code>struct AbstractValue</code>, <code>class AbstractState</code>, <code>tri_and()</code>, <code>tri_or()</code>, <code>tri_not()</code>", table_cell)
        ],
        [
            Paragraph("<code>src/verifier.h<br/>src/verifier.cpp</code>", table_cell),
            Paragraph("1480", table_cell),
            Paragraph("<b>LOGIC VERIFIER ENGINE:</b> Dataflow abstract interpretation on CFG; chaotic iteration, path refinement, loop fixed-point, and proof formatting.", table_cell),
            Paragraph("<code>class LogicVerifier</code>, <code>run_dataflow()</code>, <code>transfer()</code>, <code>check_findings()</code>, rules LV01 - LV11", table_cell)
        ],
        [
            Paragraph("<code>src/optimizer.h<br/>src/optimizer.cpp</code>", table_cell),
            Paragraph("680", table_cell),
            Paragraph("Multi-pass IR canonical optimizer; dead code elimination (DCE), constant folding, algebraic simplification, and optcheck verification.", table_cell),
            Paragraph("<code>class Optimizer</code>, <code>optimize()</code>, <code>fold_constants()</code>, <code>eliminate_dead_code()</code>, <code>--optcheck</code>", table_cell)
        ],
        [
            Paragraph("<code>src/tac_interp.h<br/>src/tac_interp.cpp</code>", table_cell),
            Paragraph("430", table_cell),
            Paragraph("Virtual machine execution interpreter for Three-Address Code; native runtime execution with stdin input queue and stdout emission.", table_cell),
            Paragraph("<code>class TACInterpreter</code>, <code>run(TACProgram)</code>, memory store, intrinsic dispatch, program exit status", table_cell)
        ]
    ]

    file_table = Table(file_matrix_data, colWidths=[90, 30, 204, 180])
    file_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), colors.HexColor("#0F172A")),
        ('TEXTCOLOR', (0,0), (-1,0), colors.white),
        ('GRID', (0,0), (-1,-1), 0.5, colors.HexColor("#CBD5E1")),
        ('PADDING', (0,0), (-1,-1), 2.5),
        ('ROWBACKGROUNDS', (0,1), (-1,-1), [colors.white, colors.HexColor("#F8FAFC")]),
    ]))
    story.append(file_table)
    story.append(PageBreak())

    # ========================================================
    # SECTION 2: LOAD AND LINKER ARCHITECTURE DEEP-DIVE
    # ========================================================
    story.append(Paragraph("2. Load and Linker Architecture Deep-Dive", h1_style))
    story.append(Paragraph(
        "<b>Responsible Subsystem Files:</b> <code>src/library_registry.h</code>, <code>src/library_registry.cpp</code>, <code>src/link_check.h</code>, <code>src/link_check.cpp</code>, <code>src/sema.cpp</code> (Side-table tracking), and <code>src/main.cpp</code> (Quarantine enforcement).",
        body_style
    ))

    story.append(Paragraph("A. Architectural Motivation: Zero-C-Dependency Built-in Registry", h2_style))
    story.append(Paragraph(
        "Traditional compiled languages (such as C/C++) rely on preprocessor header inclusion (<code>#include &lt;stdio.h&gt;</code>). This introduces fragile text copy-pasting, header pollution, slow compilation times, and non-deterministic macros. J-- resolves this by embedding a <b>data-driven, immutable built-in module registry</b> directly into the compiler binary. Libraries are native language modules requiring zero external C headers or filesystem includes.",
        body_style
    ))

    story.append(Paragraph("B. Data Structures: LibraryRegistry, LibraryDef &amp; FunctionDef", h2_style))
    story.append(Paragraph(
        "In <code>src/library_registry.h</code>, every module is modeled as a formal descriptor struct:",
        body_style
    ))

    def escape_code(text):
        return text.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;').replace('\n', '<br/>')

    reg_code = """// src/library_registry.h - Built-in Module Registry Data Structures
struct FunctionDef {
    std::string name;                   // e.g., "length", "absolute"
    std::vector<Type> param_types;      // e.g., {Type::Word}
    Type return_type;                   // e.g., Type::Num
    std::string intrinsic_id;           // Internal VM opcode dispatch
};

struct LibraryDef {
    std::string name;                   // "text", "math", "vector"
    std::string description;            // Human-readable summary
    std::vector<Type> provided_types;   // e.g., {Type::Word}
    std::vector<FunctionDef> functions; // Exported intrinsic functions
};"""
    story.append(Table([[Paragraph(f"<code>{escape_code(reg_code)}</code>", code_style)]], colWidths=[504], style=[
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor("#F8FAFC")),
        ('BOX', (0,0), (-1,-1), 0.5, colors.HexColor("#CBD5E1")),
        ('PADDING', (0,0), (-1,-1), 4),
    ]))
    story.append(Spacer(1, 4))

    story.append(Paragraph("C. Step 1: Semantic Analysis Side-Table Collection (LibUse)", h2_style))
    story.append(Paragraph(
        "During Phase 3 (Semantic Analysis in <code>src/sema.cpp</code>), the type checker validates assignments and function argument types as usual. However, <b>semantic analysis does NOT emit missing import errors</b>. Instead, it records every usage of a registered library feature into an append-only side-table of <code>LibUse</code> records:",
        body_style
    ))

    libuse_code = """// src/library_registry.h & src/sema.cpp - Side-Table Recording
struct LibUse {
    std::string symbol;      // Type name (e.g. "word") or Function ("length")
    LibUseKind kind;         // LibUseKind::Type or LibUseKind::Function
    std::string provider;     // Registry provider ("text") or empty if unregistered
    int line;                // 1-based source coordinate
    int col;                 // 1-based source column
};"""
    story.append(Table([[Paragraph(f"<code>{escape_code(libuse_code)}</code>", code_style)]], colWidths=[504], style=[
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor("#F8FAFC")),
        ('BOX', (0,0), (-1,-1), 0.5, colors.HexColor("#CBD5E1")),
        ('PADDING', (0,0), (-1,-1), 4),
    ]))
    story.append(Spacer(1, 4))

    story.append(Paragraph("D. Step 2 &amp; 3: The Load and Link Two-Phase Verification Algorithm", h2_style))
    story.append(Paragraph(
        "Phase 4 (Load &amp; Link in <code>src/link_check.cpp</code>) executes two sequential verification passes against the AST and the <code>lib_uses</code> side-table:<br/>"
        "&bull; <b>The LOAD Pass:</b> Audits program import statements in order (<code>program.imports</code>). For each import, <code>registry.findLibrary(imp.name)</code> is queried. If the library does not exist, a fatal <b>[LV13] LOAD ERROR</b> is generated. If the library was already imported earlier, an <b>[LV15] WARNING</b> (Duplicate Import) is recorded.<br/>"
        "&bull; <b>The LINK Pass:</b> Iterates through all recorded <code>lib_uses</code> in source order. For each used symbol, the checker tests whether its provider module is present in <code>loaded_libraries_</code>. If the module is not loaded, an <b>[LV12] LINK ERROR</b> is recorded. If a function is called that has NO provider in the registry at all, an <b>[LV14] LINK ERROR</b> is raised.",
        body_style
    ))

    story.append(Paragraph("E. Step 4: Fuzzy Symbol Suggestion via Levenshtein Dynamic Programming", h2_style))
    story.append(Paragraph(
        "When an undefined function symbol is encountered (LV14), <code>src/link_check.cpp</code> executes a dynamic programming matrix algorithm to compute the Levenshtein edit distance between the typed symbol and all registered intrinsics:",
        body_style
    ))

    lev_code = """// src/link_check.cpp - Levenshtein Edit Distance Algorithm
static int levenshtein_distance(const std::string& s1, const std::string& s2) {
    int m = s1.size(), n = s2.size();
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));
    for (int i = 0; i <= m; i++) dp[i][0] = i;
    for (int j = 0; j <= n; j++) dp[0][j] = j;
    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            if (s1[i - 1] == s2[j - 1]) dp[i][j] = dp[i - 1][j - 1];
            else dp[i][j] = 1 + std::min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]});
        }
    }
    return dp[m][n];
}
// If min_distance <= 2, emit: Did you mean '<candidate>' (library '<provider>')?"""
    story.append(Table([[Paragraph(f"<code>{escape_code(lev_code)}</code>", code_style)]], colWidths=[504], style=[
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor("#F8FAFC")),
        ('BOX', (0,0), (-1,-1), 0.5, colors.HexColor("#CBD5E1")),
        ('PADDING', (0,0), (-1,-1), 4),
    ]))
    story.append(Spacer(1, 4))

    story.append(Paragraph("F. Step 5: The Intermediate Code Quarantine Enforcement Guarantee", h2_style))
    story.append(Paragraph(
        "In <code>src/main.cpp</code> (lines 216-220), if <code>link_checker.check()</code> returns <code>false</code> (fatal errors LV12, LV13, or LV14), the pipeline immediately exits with code 1 and prints: <b>'Pipeline stopped before intermediate code.'</b> This design invariant guarantees that corrupted types or dangling function pointers can NEVER bleed into TAC generation, CFG construction, or optimization.",
        body_style
    ))
    story.append(PageBreak())

    # ========================================================
    # SECTION 3: LOGIC VERIFIER ARCHITECTURE DEEP-DIVE
    # ========================================================
    story.append(Paragraph("3. Logic Verifier Architecture &amp; Dataflow Mechanics", h1_style))
    story.append(Paragraph(
        "<b>Responsible Subsystem Files:</b> <code>src/verifier.h</code>, <code>src/verifier.cpp</code>, <code>src/interval.h</code>, <code>src/interval.cpp</code>, <code>src/abstract_state.h</code>, <code>src/cfg.h</code>, and <code>src/cfg.cpp</code>.",
        body_style
    ))

    story.append(Paragraph("A. Why the Logic Verifier Runs on the Control Flow Graph (CFG)", h2_style))
    story.append(Paragraph(
        "Earlier in the compiler's evolution, static verification was attempted at the Abstract Syntax Tree (AST) level. Experimental investigation proved that AST-level verification fails due to 4 structural barriers: (1) combinatorial explosion of nested tree expressions, (2) absence of explicit jump targets and loop back-edges, (3) failure of loop fixed-point iteration to converge without a flow graph, and (4) complete absence of intermediate register states. Positioning Phase 7 on the CFG after TAC Generation provides canonical three-operand instructions with explicit basic blocks, making mathematical abstract interpretation exact.",
        body_style
    ))

    story.append(Paragraph("B. The Mathematical Abstract Domain D_int and Lattice Structure", h2_style))
    story.append(Paragraph(
        "In <code>src/interval.h</code>, every numerical variable <i>x</i> is represented by a closed interval on the extended real line: <code>I(x) = [lo, hi]</code>, where <code>lo, hi in Z union {-inf, +inf}</code>. The domain forms a complete lattice:<br/>"
        "&bull; <b>Bottom (&perp;):</b> <code>is_empty = true</code>. Represents an unreachable execution path or contradictory constraints.<br/>"
        "&bull; <b>Top (&top;):</b> <code>[-inf, +inf]</code>. Represents an unconstrained variable, such as fresh external input from <code>input()</code>.<br/>"
        "&bull; <b>Singleton:</b> <code>[c, c]</code>. Represents a known compile-time constant.<br/>"
        "&bull; <b>Join Operator (&cup;):</b> <code>[l1, u1] join [l2, u2] = [min(l1, l2), max(u1, u2)]</code>. Used when control flow merges after branches.<br/>"
        "&bull; <b>Meet Operator (&cap;):</b> <code>[l1, u1] meet [l2, u2] = [max(l1, l2), min(u1, u2)]</code>. If <code>max > min</code>, result is &perp; (empty).",
        body_style
    ))

    story.append(Paragraph("C. Interval Transfer Functions Across Three-Address Instructions", h2_style))
    story.append(Paragraph(
        "In <code>src/verifier.cpp</code> (<code>transfer_instr()</code>), each TAC instruction updates the abstract environment &sigma;:",
        body_style
    ))

    transfer_code = """// src/interval.h & src/verifier.cpp - Interval Transfer Functions
Interval Interval::add(const Interval& o) const { return Interval(lo + o.lo, hi + o.hi); }
Interval Interval::sub(const Interval& o) const { return Interval(lo - o.hi, hi - o.lo); }
Interval Interval::mul(const Interval& o) const {
    double p1 = lo * o.lo, p2 = lo * o.hi, p3 = hi * o.lo, p4 = hi * o.hi;
    return Interval(std::min({p1, p2, p3, p4}), std::max({p1, p2, p3, p4}));
}
// Division-by-Zero Detection:
if (divisor.is_singleton() && divisor.lo == 0.0) {
    report("LV10", FindingSeverity::Error, line, col, "division by zero: divisor is exactly 0.");
} else if (divisor.contains(0.0) && divisor.from_input) {
    report("LV11", FindingSeverity::Warning, line, col, "potential division by zero from input.");
}"""
    story.append(Table([[Paragraph(f"<code>{escape_code(transfer_code)}</code>", code_style)]], colWidths=[504], style=[
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor("#F8FAFC")),
        ('BOX', (0,0), (-1,-1), 0.5, colors.HexColor("#CBD5E1")),
        ('PADDING', (0,0), (-1,-1), 4),
    ]))
    story.append(Spacer(1, 4))

    story.append(Paragraph("D. Path-Sensitive Edge Refinement &amp; Tri-State Boolean Logic", h2_style))
    story.append(Paragraph(
        "In <code>src/abstract_state.h</code>, boolean expressions evaluate to a 3-valued algebra: <code>TriBool = {False, True, Unknown}</code>. When control splits at a conditional branch (<code>if_true t0 goto L1</code>), the verifier performs <b>branch edge refinement</b>:<br/>"
        "&bull; <b>True Edge Refinement:</b> Constraints satisfied by condition <i>C</i> are intersected with current state: <code>&sigma;_true = &sigma; meet C</code>.<br/>"
        "&bull; <b>False Edge Refinement:</b> Negated constraints are intersected: <code>&sigma;_false = &sigma; meet (not C)</code>.<br/>"
        "If <code>&sigma;_true == &perp;</code>, the branch condition is a <b>[LV01] Contradiction</b> (always false). If <code>&sigma;_false == &perp;</code>, the condition is a <b>[LV02] Tautology</b> (always true). This mechanism also powers <b>guarded safety</b>: inside <code>check (d != 0)</code>, zero is purged from <i>d</i>'s interval along the True edge, eliminating false alarms on subsequent divisions.",
        body_style
    ))

    story.append(Paragraph("E. Chaotic Loop Fixed-Point Iteration with Widening &amp; Narrowing", h2_style))
    story.append(Paragraph(
        "For cyclic CFGs (loops), <code>src/verifier.cpp</code> executes chaotic worklist iteration until states at all block entries reach a fixed point: <code>&sigma;^(k+1) = &sigma;^(k)</code>. To guarantee convergence in finite steps, an interval <b>widening operator</b> (&nabla;) extrapolates unstable bounds that grow monotonically to &plusmn;&infin; after 3 iterations. Once the fixed point is found, an interval <b>narrowing operator</b> (&Delta;) traverses the loop to recover precise loop induction bounds. Loop rules are verified directly from this state:<br/>"
        "&bull; <b>LV05 (Never Runs):</b> Entry state satisfies loop termination condition prior to the first iteration.<br/>"
        "&bull; <b>LV06 (Infinite Loop):</b> Induction variable is not modified along any back-edge path.<br/>"
        "&bull; <b>LV07 (Exit Overshoot):</b> Induction variable step size does not evenly divide distance to exit target: <code>(exit - start) % step != 0</code>.<br/>"
        "&bull; <b>LV08 (Wrong Direction):</b> Counter moves away from the termination threshold (e.g., decrementing when exit target is higher).<br/>"
        "&bull; <b>LV09 (Dead Post-Loop Code):</b> Loop has no reachable exit edges, proving post-loop basic blocks dead.",
        body_style
    ))
    story.append(PageBreak())

    # ========================================================
    # SECTION 4: LEXER, PARSER & REGEX SCANNING MECHANICS
    # ========================================================
    story.append(Paragraph("4. Lexer, Parser &amp; Regex Scanning Mechanics", h1_style))
    story.append(Paragraph(
        "<b>Responsible Subsystem Files:</b> <code>src/lexer.h</code>, <code>src/lexer.cpp</code>, <code>src/token.h</code>, <code>src/parser.h</code>, <code>src/parser.cpp</code>, and <code>src/ast.h</code>.",
        body_style
    ))

    story.append(Paragraph("A. Main Lines of Regex and Lexical Scanning Algorithms", h2_style))
    story.append(Paragraph(
        "The Lexer in <code>src/lexer.cpp</code> implements a high-performance hand-written deterministic finite automaton (DFA) that precisely matches the formal regular expression grammar of J--:",
        body_style
    ))

    regex_summary_data = [
        [Paragraph("Token Category", table_cell_bold), Paragraph("Formal Regular Expression (Regex)", table_cell_bold), Paragraph("Lexer Implementation Mechanics (src/lexer.cpp)", table_cell_bold)],
        [
            Paragraph("<b>Keywords</b>", table_cell),
            Paragraph("<code>num|decimal|word|letter|boolean|list|import|fixed|check|otherwise|until|foreach|do_until|stop|skip|show|input|give|true|false|equals|notequals|and|or|not</code>", table_cell),
            Paragraph("Scanned via <code>identifier()</code>, followed by an O(1) hash map lookup in <code>keywords</code> table (lines 65-91). If matched, yields corresponding <code>TokenType</code>.", table_cell)
        ],
        [
            Paragraph("<b>Identifiers</b>", table_cell),
            Paragraph("<code>[a-zA-Z_][a-zA-Z0-9_]*</code>", table_cell),
            Paragraph("<code>if (std::isalpha(c) || c == '_')</code> scans contiguous alphanumeric/underscore characters. Yields <code>TokenType::Identifier</code> if not in keyword table.", table_cell)
        ],
        [
            Paragraph("<b>Integer Numbers</b>", table_cell),
            Paragraph("<code>[0-9]+</code>", table_cell),
            Paragraph("<code>number()</code> accumulates ASCII digits via <code>std::isdigit()</code>. Verified as discrete 64-bit integer 'num'.", table_cell)
        ],
        [
            Paragraph("<b>Decimal Floats</b>", table_cell),
            Paragraph("<code>[0-9]+\\.[0-9]+</code>", table_cell),
            Paragraph("When a '.' is encountered with a trailing digit, switches to float mode. Yields <code>TokenType::Decimal</code>.", table_cell)
        ],
        [
            Paragraph("<b>String Literals</b>", table_cell),
            Paragraph("<code>\"([^\"\\\\]|\\\\.)*\"</code>", table_cell),
            Paragraph("Scanned on '\"' delimiter until matching unescaped '\"'. Newlines within string increment source line counter; unclosed string emits error.", table_cell)
        ],
        [
            Paragraph("<b>Char Literals</b>", table_cell),
            Paragraph("<code>'([^'\\\\]|\\\\.)'</code>", table_cell),
            Paragraph("Scanned on '\'' delimiter. Expects single character or escape sequence followed by closing '\''.", table_cell)
        ],
        [
            Paragraph("<b>Comments</b>", table_cell),
            Paragraph("<code>#[^\\r\\n]*</code>", table_cell),
            Paragraph("<code>skip_whitespace_and_comments()</code> consumes '#' through the end of the line. Comments are discarded without generating tokens.", table_cell)
        ],
        [
            Paragraph("<b>Multi-Char Operators</b>", table_cell),
            Paragraph("<code>== | != | &lt;= | &gt;= | \\+\\+ | --</code>", table_cell),
            Paragraph("Lookahead helper <code>match('=')</code> checks following byte to differentiate '=' from '==', '+' from '++', etc.", table_cell)
        ]
    ]

    regex_table = Table(regex_summary_data, colWidths=[90, 194, 220])
    regex_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), colors.HexColor("#0F172A")),
        ('TEXTCOLOR', (0,0), (-1,0), colors.white),
        ('GRID', (0,0), (-1,-1), 0.5, colors.HexColor("#CBD5E1")),
        ('PADDING', (0,0), (-1,-1), 3),
        ('ROWBACKGROUNDS', (0,1), (-1,-1), [colors.white, colors.HexColor("#F8FAFC")]),
    ]))
    story.append(regex_table)
    story.append(Spacer(1, 6))

    story.append(Paragraph("B. Recursive-Descent Parser Grammar Invariants", h2_style))
    story.append(Paragraph(
        "In <code>src/parser.cpp</code>, the parser enforces two critical grammar rules that preserve compilation integrity:<br/>"
        "&bull; <b>Top-Level Import Position:</b> Grammar rule <code>program -&gt; import* statement* EOF</code>. The loop in <code>parse()</code> requires all imports to precede any other statement. If <code>parse_import()</code> is encountered after any statement has been parsed, it immediately records a syntax error: <i>'imports must appear at the top of the program.'</i><br/>"
        "&bull; <b>Expression Precedence Climbing:</b> Binary expressions are parsed using Pratt-style precedence levels: Logical OR (lowest) &rarr; Logical AND &rarr; Equality (equals, notequals) &rarr; Relational (&lt;, &gt;, &lt;=, &gt;=) &rarr; Additive (+, -) &rarr; Multiplicative (*, /, %) &rarr; Unary (-, not, ++, --). This ensures AST nodes represent mathematically sound operator hierarchy.",
        body_style
    ))
    story.append(Spacer(1, 4))

    # ========================================================
    # SECTION 5: OPTIMIZER & NATIVE EXECUTION SYNERGY
    # ========================================================
    story.append(Paragraph("5. Synergy with Phase 8 IR Optimizer &amp; TAC Virtual Machine", h1_style))
    story.append(Paragraph(
        "<b>Responsible Subsystem Files:</b> <code>src/optimizer.h</code>, <code>src/optimizer.cpp</code>, <code>src/tac_interp.h</code>, <code>src/tac_interp.cpp</code>, and <code>src/runtime_semantics.h</code>.",
        body_style
    ))
    story.append(Paragraph(
        "<b>The Optimizer Paradox Solved:</b> Classical compilers place optimization ahead of or in lieu of verification. When a standard compiler sees <code>x &gt; 50 and x &lt; 20</code>, its Dead Code Elimination (DCE) pass silently discards the developer's entire block without warning. In J--, the <b>Logic Verifier executes BEFORE the optimizer</b>. Once the Logic Verifier audits the code and emits proofs, Phase 8 (Optimizer) applies aggressive algebraic transformations without fear of silencing latent bugs:<br/>"
        "&bull; <b>Constant Folding &amp; Propagation:</b> Pre-calculates constant arithmetic (e.g., <code>10 + 20 -&gt; 30</code>).<br/>"
        "&bull; <b>Algebraic Identity Reduction:</b> Transforms <code>x * 0 -&gt; 0</code>, <code>x + 0 -&gt; x</code>, <code>x * 1 -&gt; x</code>.<br/>"
        "&bull; <b>Dead Code Elimination (DCE):</b> Prunes basic blocks whose incoming edges have been proven unreachable.<br/>"
        "&bull; <b>The <code>--optcheck</code> Mode:</b> Executes unoptimized and optimized TAC in parallel, verifying character-for-character execution equivalence.<br/>"
        "&bull; <b>Native TAC Interpreter:</b> <code>src/tac_interp.cpp</code> implements a clean virtual machine executing Three-Address Code with simulated stdin and stdout streams.",
        body_style
    ))
    story.append(Spacer(1, 6))

    # Concluding Sign-off Box
    concl = """
    <b>Architectural Evaluation Summary:</b> The J-- compiler architecture proves that compile-time logic verification is achievable with zero false alarms when situated between Intermediate Three-Address Code Generation and Canonical Optimization. By unifying a <b>Data-Driven Load/Link Registry</b> with <b>CFG Abstract Interpretation</b>, J-- establishes a new benchmark for compiler reliability.
    <br/><br/>
    <b>Report Authors:</b> Utkarsh Vikram Singh (24BCE2611) &nbsp;|&nbsp; Ishan Awasthi (24BAI0207)
    """
    concl_table = Table([[Paragraph(concl, callout_style)]], colWidths=[504])
    concl_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor("#EFF6FF")),
        ('BOX', (0,0), (-1,-1), 1, colors.HexColor("#93C5FD")),
        ('PADDING', (0,0), (-1,-1), 6),
    ]))
    story.append(concl_table)

    doc.build(story, canvasmaker=NumberedCanvas)
    print(f"Codebase Guide PDF built successfully at: {output_path}")

if __name__ == '__main__':
    out_file = r"C:\Users\thaku\Downloads\J--_Compiler_Codebase_Architecture_Guide.pdf"
    build_codebase_guide_pdf(out_file)
