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
            self.drawString(54, 11 * inch - 36, "J-- COMPILER: 40-PROGRAM UNSEEN DASHBOARD TEST SUITE")
            self.drawRightString(8.5 * inch - 54, 11 * inch - 36, "EXAMINER LIVE AUDIT & VERIFICATION MANUAL")
            self.setStrokeColor(colors.HexColor("#CBD5E1"))
            self.setLineWidth(0.75)
            self.line(54, 11 * inch - 42, 8.5 * inch - 54, 11 * inch - 42)

        # Footer (all pages)
        self.setStrokeColor(colors.HexColor("#CBD5E1"))
        self.setLineWidth(0.75)
        self.line(54, 46, 8.5 * inch - 54, 46)
        
        self.setFont("Helvetica", 8)
        self.setFillColor(colors.HexColor("#64748B"))
        self.drawString(54, 32, "Utkarsh Vikram Singh (24BCE2611) & Ishan Awasthi (24BAI0207) | J-- System Verifier")
        page_text = f"Page {self._pageNumber} of {page_count}"
        self.drawRightString(8.5 * inch - 54, 32, page_text)
        self.restoreState()

def build_test_suite_pdf(output_path):
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
        fontSize=21,
        leading=25,
        textColor=colors.HexColor("#0F172A"),
        spaceAfter=3
    )

    subtitle_style = ParagraphStyle(
        'DocSubtitle',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=10.5,
        leading=14,
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
        spaceBefore=6,
        spaceAfter=3,
        keepWithNext=True
    )

    body_style = ParagraphStyle(
        'Body_Custom',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=8.5,
        leading=11.5,
        textColor=colors.HexColor("#334155"),
        spaceAfter=4
    )

    card_header_style = ParagraphStyle(
        'CardHeader',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=9,
        leading=11,
        textColor=colors.white
    )

    card_meta_style = ParagraphStyle(
        'CardMeta',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=7.5,
        leading=10,
        textColor=colors.HexColor("#0F172A")
    )

    card_desc_style = ParagraphStyle(
        'CardDesc',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=7.5,
        leading=10,
        textColor=colors.HexColor("#334155")
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

    story = []

    # ========================================================
    # COVER / EXECUTIVE HEADER
    # ========================================================
    story.append(Paragraph("J-- Compiler &amp; Logic Verifier", title_style))
    story.append(Paragraph("Comprehensive 40-Program Unseen Test Suite for Examiner Live Demonstration", subtitle_style))

    author_card = """
    <b>Principal Investigators &amp; System Designers:</b><br/>
    &bull; <b>Utkarsh Vikram Singh</b> (Reg No: <b>24BCE2611</b>) - Core Compiler Pipeline, Abstract Interpretation &amp; Verifier Engine<br/>
    &bull; <b>Ishan Awasthi</b> (Reg No: <b>24BAI0207</b>) - Modular Load &amp; Link Registry, CFG Dataflow &amp; Optimizer Framework<br/>
    <b>Test Suite Scope:</b> 40 Fresh, Unseen, Hand-Verified J-- Programs across 8 Comprehensive Engineering Categories (5 tests per category).<br/>
    <b>Execution Target:</b> J-- Native Compiler (C++20, Intermediate Three-Address IR, CFG Abstract Interpreter, Multi-Pass Canonical Optimizer).
    """
    author_table = Table([[Paragraph(author_card, author_style)]], colWidths=[504])
    author_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor("#F8FAFC")),
        ('BOX', (0,0), (-1,-1), 1, colors.HexColor("#CBD5E1")),
        ('PADDING', (0,0), (-1,-1), 6),
    ]))
    story.append(author_table)
    story.append(Spacer(1, 8))

    story.append(Paragraph("1. Examiner Demonstration Protocol &amp; Instructions", h1_style))
    story.append(Paragraph(
        "This document provides <b>40 brand-new, uncoded, and unseen test programs</b> specifically designed to demonstrate the full diagnostic power of the J-- compiler and its projector dashboard to an examiner. Every program has been verified against the ground-truth binary. Examiners can copy and paste any snippet directly into the dashboard scratchpad (<code>Untitled.jmm</code>) or click <b>RUN PIPELINE (CTRL+ENTER)</b> to witness instant stage-by-stage verification.",
        body_style
    ))
    story.append(Paragraph(
        "<b>Dashboard Visual Checklist for the Examiner:</b><br/>"
        "&bull; <b>Pipeline Strip Indicators:</b> Successful phases display a white badge with a checkmark (<font color='#16A34A'><b>&check;</b></font>); the failing phase turns solid dark with an inverted cross (<font color='#DC2626'><b>&empty;</b></font>); downstream phases are marked as <i>Not Reached</i>.<br/>"
        "&bull; <b>Editor Highlighting:</b> Offending code lines are highlighted with a soft transparent tint (<code>rgba(220, 38, 38, 0.08)</code>) and a crisp 3px left crimson bar without obscuring source text.<br/>"
        "&bull; <b>Diagnostics Drawer:</b> Emits precise failure proofs, variable intervals, loop step overshoot math, or fuzzy typo suggestions.",
        body_style
    ))
    story.append(Spacer(1, 6))

    # Master Table of 40 Tests
    story.append(Paragraph("2. Master Test Suite Matrix (40 Programs at a Glance)", h1_style))
    
    matrix_data = [
        [Paragraph("Cat", table_cell_bold), Paragraph("ID", table_cell_bold), Paragraph("Scenario / Program Title", table_cell_bold), Paragraph("Halted Stage", table_cell_bold), Paragraph("Exit", table_cell_bold), Paragraph("Target Rule / Diagnostic", table_cell_bold)],
        # Cat 1
        [Paragraph("1", table_cell), Paragraph("SYN-01", table_cell_bold), Paragraph("Unclosed Block (Missing '}')", table_cell), Paragraph("Parser", table_cell), Paragraph("1", table_cell), Paragraph("Syntax Error: expected '}'", table_cell)],
        [Paragraph("1", table_cell), Paragraph("SYN-02", table_cell_bold), Paragraph("Misplaced Import Statement", table_cell), Paragraph("Parser", table_cell), Paragraph("1", table_cell), Paragraph("Imports must appear at top", table_cell)],
        [Paragraph("1", table_cell), Paragraph("SYN-03", table_cell_bold), Paragraph("Chained Operator Sequence (++*)", table_cell), Paragraph("Parser", table_cell), Paragraph("1", table_cell), Paragraph("Unexpected token in expression", table_cell)],
        [Paragraph("1", table_cell), Paragraph("SYN-04", table_cell_bold), Paragraph("Import Missing Library Name", table_cell), Paragraph("Parser", table_cell), Paragraph("1", table_cell), Paragraph("Expected lib name after 'import'", table_cell)],
        [Paragraph("1", table_cell), Paragraph("SYN-05", table_cell_bold), Paragraph("Missing Expression on Assignment", table_cell), Paragraph("Parser", table_cell), Paragraph("1", table_cell), Paragraph("Expected expression found ';'", table_cell)],
        # Cat 2
        [Paragraph("2", table_cell), Paragraph("SEM-01", table_cell_bold), Paragraph("Undeclared Variable in Expression", table_cell), Paragraph("Semantic", table_cell), Paragraph("1", table_cell), Paragraph("Undeclared variable 'bonus'", table_cell)],
        [Paragraph("2", table_cell), Paragraph("SEM-02", table_cell_bold), Paragraph("Decimal Assigned to Integer Num", table_cell), Paragraph("Semantic", table_cell), Paragraph("1", table_cell), Paragraph("Type mismatch: num vs decimal", table_cell)],
        [Paragraph("2", table_cell), Paragraph("SEM-03", table_cell_bold), Paragraph("Reassigning Fixed Immutable Constant", table_cell), Paragraph("Semantic", table_cell), Paragraph("1", table_cell), Paragraph("Cannot assign to fixed symbol", table_cell)],
        [Paragraph("2", table_cell), Paragraph("SEM-04", table_cell_bold), Paragraph("Incompatible Boolean Addition (+)", table_cell), Paragraph("Semantic", table_cell), Paragraph("1", table_cell), Paragraph("Invalid operand types for '+'", table_cell)],
        [Paragraph("2", table_cell), Paragraph("SEM-05", table_cell_bold), Paragraph("Scope Leak (Block Variable Outside)", table_cell), Paragraph("Semantic", table_cell), Paragraph("1", table_cell), Paragraph("Undeclared out of scope block", table_cell)],
        # Cat 3
        [Paragraph("3", table_cell), Paragraph("LV-01", table_cell_bold), Paragraph("Contradiction (x > 50 and x < 20)", table_cell), Paragraph("Verifier", table_cell), Paragraph("0", table_cell), Paragraph("[LV01] Always-False Condition", table_cell)],
        [Paragraph("3", table_cell), Paragraph("LV-02", table_cell_bold), Paragraph("Tautology Range (score >= 0 or < 100)", table_cell), Paragraph("Verifier", table_cell), Paragraph("0", table_cell), Paragraph("[LV02] Always-True Condition", table_cell)],
        [Paragraph("3", table_cell), Paragraph("LV-03", table_cell_bold), Paragraph("Reflexive Self-Comparison (t == t)", table_cell), Paragraph("Verifier", table_cell), Paragraph("0", table_cell), Paragraph("[LV03] Self-Comparison Flaw", table_cell)],
        [Paragraph("3", table_cell), Paragraph("LV-04", table_cell_bold), Paragraph("Redundant Condition (a > 20 & a > 10)", table_cell), Paragraph("Verifier", table_cell), Paragraph("0", table_cell), Paragraph("[LV04] Redundant Condition Part", table_cell)],
        [Paragraph("3", table_cell), Paragraph("LV-01b", table_cell_bold), Paragraph("Multi-Branch Invariant Contradiction", table_cell), Paragraph("Verifier", table_cell), Paragraph("0", table_cell), Paragraph("[LV01] Dead Branch Proof", table_cell)],
        # Cat 4
        [Paragraph("4", table_cell), Paragraph("LV-05", table_cell_bold), Paragraph("Loop Condition Initially Satisfied", table_cell), Paragraph("Verifier", table_cell), Paragraph("0", table_cell), Paragraph("[LV05] Loop Body Never Runs", table_cell)],
        [Paragraph("4", table_cell), Paragraph("LV-06", table_cell_bold), Paragraph("Invariant Counter Loop (No Increment)", table_cell), Paragraph("Verifier", table_cell), Paragraph("1", table_cell), Paragraph("[LV06] Loop Never Terminates", table_cell)],
        [Paragraph("4", table_cell), Paragraph("LV-07", table_cell_bold), Paragraph("Induction Overshoot (i += 2 skips 7)", table_cell), Paragraph("Verifier", table_cell), Paragraph("1", table_cell), Paragraph("[LV07] Step Skips Loop Exit", table_cell)],
        [Paragraph("4", table_cell), Paragraph("LV-08", table_cell_bold), Paragraph("Loop Moves Wrong Direction (x--)", table_cell), Paragraph("Verifier", table_cell), Paragraph("1", table_cell), Paragraph("[LV08] Induction Diverges", table_cell)],
        [Paragraph("4", table_cell), Paragraph("LV-09", table_cell_bold), Paragraph("Dead Code After Infinite Loop", table_cell), Paragraph("Verifier", table_cell), Paragraph("1", table_cell), Paragraph("[LV06],[LV09] Unreachable Block", table_cell)],
        # Cat 5
        [Paragraph("5", table_cell), Paragraph("LV-10a", table_cell_bold), Paragraph("Definite Constant Division by Zero", table_cell), Paragraph("Verifier", table_cell), Paragraph("1", table_cell), Paragraph("[LV10] Divisor is exactly 0", table_cell)],
        [Paragraph("5", table_cell), Paragraph("LV-10b", table_cell_bold), Paragraph("Computed Zero Denominator (a - b)", table_cell), Paragraph("Verifier", table_cell), Paragraph("1", table_cell), Paragraph("[LV10] Interval collapsed to [0,0]", table_cell)],
        [Paragraph("5", table_cell), Paragraph("LV-11a", table_cell_bold), Paragraph("Potential Div-0 from Raw User Input", table_cell), Paragraph("Verifier", table_cell), Paragraph("0", table_cell), Paragraph("[LV11] Divisor contains 0", table_cell)],
        [Paragraph("5", table_cell), Paragraph("LV-10c", table_cell_bold), Paragraph("Arithmetic Invariant Zero Divisor", table_cell), Paragraph("Verifier", table_cell), Paragraph("1", table_cell), Paragraph("[LV10] Fatal Zero Divisor", table_cell)],
        [Paragraph("5", table_cell), Paragraph("LV-11b", table_cell_bold), Paragraph("Modulo by Unguarded User Input", table_cell), Paragraph("Verifier", table_cell), Paragraph("0", table_cell), Paragraph("[LV11] Modulo Divisor Tainted", table_cell)],
        # Cat 6
        [Paragraph("6", table_cell), Paragraph("GRD-01", table_cell_bold), Paragraph("Guarded Division with 'divisor != 0'", table_cell), Paragraph("Execution", table_cell), Paragraph("0", table_cell), Paragraph("PASS: Zero Warnings Suppressed", table_cell)],
        [Paragraph("6", table_cell), Paragraph("GRD-02", table_cell_bold), Paragraph("Guarded Range Division ('den > 0')", table_cell), Paragraph("Execution", table_cell), Paragraph("0", table_cell), Paragraph("PASS: Zero Excluded [1, +inf]", table_cell)],
        [Paragraph("6", table_cell), Paragraph("GRD-03", table_cell_bold), Paragraph("Odd-Step Loop Exactly Hitting Exit (9)", table_cell), Paragraph("Execution", table_cell), Paragraph("0", table_cell), Paragraph("PASS: Reaches 9 in 4 iterations", table_cell)],
        [Paragraph("6", table_cell), Paragraph("GRD-04", table_cell_bold), Paragraph("Clean Induction Countdown (10 to 0)", table_cell), Paragraph("Execution", table_cell), Paragraph("0", table_cell), Paragraph("PASS: Fixed-point convergence", table_cell)],
        [Paragraph("6", table_cell), Paragraph("GRD-05", table_cell_bold), Paragraph("Safe Dual-Branch Initialization Join", table_cell), Paragraph("Execution", table_cell), Paragraph("0", table_cell), Paragraph("PASS: Joined interval [2, 5]", table_cell)],
        # Cat 7
        [Paragraph("7", table_cell), Paragraph("LV-12a", table_cell_bold), Paragraph("Unimported Type 'word' Usage", table_cell), Paragraph("Load/Link", table_cell), Paragraph("1", table_cell), Paragraph("[LV12] Library 'text' not imported", table_cell)],
        [Paragraph("7", table_cell), Paragraph("LV-12b", table_cell_bold), Paragraph("Unimported Function 'absolute' Call", table_cell), Paragraph("Load/Link", table_cell), Paragraph("1", table_cell), Paragraph("[LV12] Library 'math' not imported", table_cell)],
        [Paragraph("7", table_cell), Paragraph("LV-13", table_cell_bold), Paragraph("Unknown Library Load ('graphics')", table_cell), Paragraph("Load/Link", table_cell), Paragraph("1", table_cell), Paragraph("[LV13] Cannot load 'graphics'", table_cell)],
        [Paragraph("7", table_cell), Paragraph("LV-14", table_cell_bold), Paragraph("Typo 'lenght' with Levenshtein (D<=2)", table_cell), Paragraph("Load/Link", table_cell), Paragraph("1", table_cell), Paragraph("[LV14] Did you mean 'length'?", table_cell)],
        [Paragraph("7", table_cell), Paragraph("LV-15/16", table_cell_bold), Paragraph("Duplicate Import & Unused Import", table_cell), Paragraph("Execution", table_cell), Paragraph("0", table_cell), Paragraph("[LV15] Dup 'text', [LV16] Unused", table_cell)],
        # Cat 8
        [Paragraph("8", table_cell), Paragraph("VAL-01", table_cell_bold), Paragraph("Standard Library Text String Audit", table_cell), Paragraph("Execution", table_cell), Paragraph("0", table_cell), Paragraph("PASS: Outputs length & isempty", table_cell)],
        [Paragraph("8", table_cell), Paragraph("VAL-02", table_cell_bold), Paragraph("Math Extremum (absolute, max, min)", table_cell), Paragraph("Execution", table_cell), Paragraph("0", table_cell), Paragraph("PASS: Computes 75, 120, 75", table_cell)],
        [Paragraph("8", table_cell), Paragraph("VAL-03", table_cell_bold), Paragraph("Temperature Converter Formula", table_cell), Paragraph("Execution", table_cell), Paragraph("0", table_cell), Paragraph("PASS: Computes 86F, heat alert", table_cell)],
        [Paragraph("8", table_cell), Paragraph("VAL-04", table_cell_bold), Paragraph("Iterative Fibonacci Number Gen (N=7)", table_cell), Paragraph("Execution", table_cell), Paragraph("0", table_cell), Paragraph("PASS: Computes 13 correctly", table_cell)],
        [Paragraph("8", table_cell), Paragraph("VAL-05", table_cell_bold), Paragraph("Hierarchical Access Decision Matrix", table_cell), Paragraph("Execution", table_cell), Paragraph("0", table_cell), Paragraph("PASS: Resolves level 50 for Dev", table_cell)],
    ]

    matrix_table = Table(matrix_data, colWidths=[24, 48, 172, 60, 28, 172])
    matrix_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), colors.HexColor("#0F172A")),
        ('TEXTCOLOR', (0,0), (-1,0), colors.white),
        ('GRID', (0,0), (-1,-1), 0.5, colors.HexColor("#CBD5E1")),
        ('PADDING', (0,0), (-1,-1), 2.5),
        ('ROWBACKGROUNDS', (0,1), (-1,-1), [colors.white, colors.HexColor("#F8FAFC")]),
    ]))
    story.append(matrix_table)
    story.append(PageBreak())

    # ========================================================
    # HELPER FUNCTION FOR CREATING TEST CARDS
    # ========================================================
    def make_test_card(test_id, title, category, phase, severity, exit_code, rule, desc, code, expected_diag, visual_checklist):
        is_error = (exit_code != 0)
        badge_bg = colors.HexColor("#0F172A") if is_error else colors.HexColor("#1E293B")
        
        header_text = f"<b>[{test_id}] {title}</b> &nbsp;|&nbsp; <b>{category}</b>"
        header_table = Table([[Paragraph(header_text, card_header_style)]], colWidths=[504])
        header_table.setStyle(TableStyle([
            ('BACKGROUND', (0,0), (-1,-1), badge_bg),
            ('PADDING', (0,0), (-1,-1), 4),
        ]))

        meta_content = f"""
        <b>Target Phase:</b> {phase} &nbsp;|&nbsp; <b>Severity:</b> {severity} &nbsp;|&nbsp; <b>Expected Exit Code:</b> {exit_code} &nbsp;|&nbsp; <b>Diagnostic Tag:</b> {rule}<br/>
        <b>Test Objective:</b> {desc}
        """
        meta_table = Table([[Paragraph(meta_content, card_meta_style)]], colWidths=[504])
        meta_table.setStyle(TableStyle([
            ('BACKGROUND', (0,0), (-1,-1), colors.HexColor("#F1F5F9")),
            ('BOX', (0,0), (-1,-1), 0.5, colors.HexColor("#CBD5E1")),
            ('PADDING', (0,0), (-1,-1), 4),
        ]))

        # Code block
        code_html = "<br/>".join(code.strip().split("\n"))
        code_table = Table([[Paragraph(f"<code>{code_html}</code>", code_style)]], colWidths=[504])
        code_table.setStyle(TableStyle([
            ('BACKGROUND', (0,0), (-1,-1), colors.HexColor("#FFFFFF")),
            ('BOX', (0,0), (-1,-1), 0.5, colors.HexColor("#CBD5E1")),
            ('PADDING', (0,0), (-1,-1), 4),
        ]))

        diag_content = f"""
        <b>Expected Compiler Diagnostic / Output:</b><br/>
        <font face="Courier" color="#0F172A"><i>{expected_diag}</i></font><br/>
        <b>Examiner Projector Checklist:</b> {visual_checklist}
        """
        diag_bg = colors.HexColor("#FEF2F2") if is_error else colors.HexColor("#F0FDF4")
        diag_border = colors.HexColor("#FCA5A5") if is_error else colors.HexColor("#86EFAC")
        diag_table = Table([[Paragraph(diag_content, card_desc_style)]], colWidths=[504])
        diag_table.setStyle(TableStyle([
            ('BACKGROUND', (0,0), (-1,-1), diag_bg),
            ('BOX', (0,0), (-1,-1), 0.5, diag_border),
            ('PADDING', (0,0), (-1,-1), 4),
        ]))

        card_wrapper = Table([[header_table], [meta_table], [code_table], [diag_table]], colWidths=[504])
        card_wrapper.setStyle(TableStyle([
            ('BOX', (0,0), (-1,-1), 1, colors.HexColor("#94A3B8")),
            ('PADDING', (0,0), (-1,-1), 0),
        ]))
        return KeepTogether(card_wrapper)

    # ========================================================
    # CATEGORY 1: LEXICAL & SYNTACTIC MALFORMATIONS (Exit 1)
    # ========================================================
    story.append(Paragraph("Category 1: Lexical &amp; Syntactic Malformations (Exit Code: 1)", h1_style))
    story.append(Paragraph(
        "These five programs demonstrate the J-- Lexer and Parser frontend. Syntax errors must halt the compiler at Phase 2 (Parser), aborting intermediate code generation before Semantic Analysis or Load/Link checks are invoked.",
        body_style
    ))

    c1_tests = [
        ("SYN-01", "Unclosed Block (Missing '}')", "Category 1: Syntax", "Parser (Phase 2)", "FATAL ERROR", "1", "Syntax Error",
         "Omits the closing brace '}' of a check statement block; verifies recursive descent block recovery.",
         "num x = 50;\ncheck (x > 20) {\n    show(x);\n    num y = 10;\n    show(y);\n# Missing closing brace before EOF",
         "line 7, col 1: error: expected '}' to close block before end of file.",
         "Pipeline fails at PARSER (box turns black with empty symbol); SEMANTIC through RUN remain 'Not Reached'; line 7 highlighted."),

        ("SYN-02", "Misplaced Import Statement", "Category 1: Syntax", "Parser (Phase 2)", "FATAL ERROR", "1", "Syntax Error",
         "Places an import statement below executable variable declarations; verifies import position invariant.",
         "num a = 10;\nshow(a);\nimport text; # Error: imports must appear at top\nword s = \"hello\";\nshow(s);",
         "line 3, col 1: error: imports must appear at the top of the program.",
         "Parser halts compilation immediately. Raw Output displays exact syntax line 3 constraint failure."),

        ("SYN-03", "Malformed Chained Operators (++*)", "Category 1: Syntax", "Parser (Phase 2)", "FATAL ERROR", "1", "Syntax Error",
         "Combines postfix increment with binary multiplication without an intermediate operand.",
         "num p = 25;\nnum q = p ++* 5;\nshow(q);",
         "line 2, col 11: error: expected expression after '++'.",
         "Lexer tokenizes PlusPlus followed by Star; Parser rejects illegal token sequence on line 2."),

        ("SYN-04", "Import Missing Library Name", "Category 1: Syntax", "Parser (Phase 2)", "FATAL ERROR", "1", "Syntax Error",
         "Emits 'import ;' with a missing identifier; tests grammar rule 'import -> IMPORT ID SEMICOLON'.",
         "import ;\nnum z = 42;\nshow(z);",
         "line 1, col 8: error: expected a library name after 'import'.",
         "Halts at Phase 2. Prevents empty string lookup in the module registry."),

        ("SYN-05", "Missing Expression on Assignment", "Category 1: Syntax", "Parser (Phase 2)", "FATAL ERROR", "1", "Syntax Error",
         "Leaves variable assignment incomplete ('num tax = ;'); verifies expression parser boundary check.",
         "num price = 100;\nnum tax = ;\nshow(price);",
         "line 2, col 11: error: expected expression after '='.",
         "Parser catches empty rhs expression; red line indicator on line 2; pipeline aborts at Phase 2.")
    ]

    for t in c1_tests:
        story.append(make_test_card(*t))
        story.append(Spacer(1, 6))

    story.append(PageBreak())

    # ========================================================
    # CATEGORY 2: SEMANTIC ANALYSIS & TYPE INVARIANTS (Exit 1)
    # ========================================================
    story.append(Paragraph("Category 2: Semantic Analysis &amp; Type System Invariants (Exit Code: 1)", h1_style))
    story.append(Paragraph(
        "These five programs test Phase 3 (Semantic Analyzer). Programs pass parsing successfully (syntactically valid AST), but violate type compatibility, immutability, or lexical scope boundaries.",
        body_style
    ))

    c2_tests = [
        ("SEM-01", "Undeclared Variable in Arithmetic", "Category 2: Semantic", "Semantic (Phase 3)", "FATAL ERROR", "1", "Sema Error",
         "References symbol 'uncalculatedBonus' that was never declared in any active lexical scope.",
         "num total = 100;\nnum finalScore = total + uncalculatedBonus;\nshow(finalScore);",
         "line 2, col 26: error: undeclared variable 'uncalculatedBonus'.",
         "LEXER and PARSER pass (&check;); SEMANTIC fails (&empty;); Symbol table dump shows 'total' declared but 'uncalculatedBonus' missing."),

        ("SEM-02", "Decimal Assigned to Integer Num", "Category 2: Semantic", "Semantic (Phase 3)", "FATAL ERROR", "1", "Type Mismatch",
         "Attempts to assign decimal float literal to discrete 'num' variable without explicit casting.",
         "decimal pi = 3.14159;\nnum count = pi;\nshow(count);",
         "line 2, col 13: error: type mismatch in assignment: cannot assign 'decimal' to 'num'.",
         "Strict type enforcement flags line 2; intermediate TAC generation is aborted."),

        ("SEM-03", "Reassigning Fixed Immutable Constant", "Category 2: Semantic", "Semantic (Phase 3)", "FATAL ERROR", "1", "Fixed Mutation",
         "Attempts to mutate a constant declared with the 'fixed' storage qualifier.",
         "fixed num MAX_USERS = 500;\nshow(MAX_USERS);\nMAX_USERS = 600; # Error: cannot mutate fixed\nshow(MAX_USERS);",
         "line 3, col 1: error: cannot assign to fixed variable 'MAX_USERS'.",
         "Semantic analyzer catches mutation of immutable symbol; halts pipeline at Phase 3."),

        ("SEM-04", "Incompatible Boolean Operand in Addition", "Category 2: Semantic", "Semantic (Phase 3)", "FATAL ERROR", "1", "Type Mismatch",
         "Applies numerical addition operator '+' to a boolean operand ('num result = base + flag').",
         "num base = 40;\nboolean flag = true;\nnum result = base + flag;\nshow(result);",
         "line 3, col 21: error: invalid operand types for operator '+': 'num' and 'boolean'.",
         "Type checker verifies operator signatures; rejects non-arithmetic boolean operand."),

        ("SEM-05", "Scope Leak (Block Variable Accessed Outside)", "Category 2: Semantic", "Semantic (Phase 3)", "FATAL ERROR", "1", "Scope Error",
         "Declares 'secretToken' inside a nested check block and attempts to read it in the parent scope.",
         "check (true) {\n    num secretToken = 999;\n    show(secretToken);\n}\nshow(secretToken); # Error: out of scope",
         "line 5, col 6: error: undeclared variable 'secretToken'.",
         "Scope depth deallocation verified; symbol popped on block exit; line 5 highlighted as undeclared.")
    ]

    for t in c2_tests:
        story.append(make_test_card(*t))
        story.append(Spacer(1, 6))

    story.append(PageBreak())

    # ========================================================
    # CATEGORY 3: BRANCH & CONDITION FLAWS (LV01 - LV04)
    # ========================================================
    story.append(Paragraph("Category 3: Path-Sensitive Branch &amp; Condition Flaws (LV01 - LV04)", h1_style))
    story.append(Paragraph(
        "These programs audit Phase 7 (Logic Verifier) on conditional branching. Using abstract interval propagation, the verifier proves that branch conditions are mathematically impossible (LV01), tautological (LV02), reflexive (LV03), or redundant (LV04).",
        body_style
    ))

    c3_tests = [
        ("LV-01", "Contradictory Condition Interval", "Category 3: Conditions", "Logic Verifier (Phase 7)", "WARNING", "0", "LV01",
         "Asserts 'val > 50 and val < 20'; the intersection of [51, +inf] and [-inf, 19] is bottom (empty).",
         "num val = 15;\ncheck (val > 50 and val < 20) {\n    show(1); # Mathematically unreachable\n}\nshow(val);",
         "line 2, col 1: warning: [LV01] condition is always false (contradiction). Invariant proof: interval is empty.",
         "Pipeline reaches RUN (&check;); warning displayed in Diagnostics drawer; proof attached to node."),

        ("LV-02", "Tautology Condition Covering Domain", "Category 3: Conditions", "Logic Verifier (Phase 7)", "WARNING", "0", "LV02",
         "Asserts 'score >= 0 or score < 100' for a positive score; the union covers the entire domain.",
         "num score = 85;\ncheck (score >= 0 or score < 100) {\n    show(score); # Always executes\n}\nshow(0);",
         "line 2, col 1: warning: [LV02] condition is always true (tautology). Sub-expression is invariant.",
         "Diagnostics drawer flags redundant runtime check; optimizer simplifies branch in Phase 8."),

        ("LV-03", "Reflexive Self-Comparison", "Category 3: Conditions", "Logic Verifier (Phase 7)", "WARNING", "0", "LV03",
         "Compares variable 'temperature' directly to itself ('temperature equals temperature').",
         "num temperature = 32;\ncheck (temperature equals temperature) {\n    show(1);\n}\nshow(temperature);",
         "line 2, col 7: warning: [LV03] self-comparison detected: 'temperature equals temperature'.",
         "Self-comparison diagnostic emitted; verified via AST symbol equivalence and TAC operand identity."),

        ("LV-04", "Redundant Sub-Condition Part", "Category 3: Conditions", "Logic Verifier (Phase 7)", "WARNING", "0", "LV04",
         "Tests 'age > 20 and age > 10'; the second conjunct is subsumed by the first.",
         "num age = 25;\ncheck (age > 20 and age > 10) {\n    show(age);\n}\nshow(1);",
         "line 2, col 21: warning: [LV04] redundant condition part: 'age > 10' is already guaranteed by 'age > 20'.",
         "Interval lattice subsumption verified: [21, +inf] subset of [11, +inf]."),

        ("LV-01b", "Multi-Branch Invariant Contradiction", "Category 3: Conditions", "Logic Verifier (Phase 7)", "WARNING", "0", "LV01",
         "Checks equality to two mutually exclusive constants simultaneously in a single conjunction.",
         "num level = 5;\ncheck (level equals 10 and level equals 20) {\n    show(999);\n}\notherwise {\n    show(level);\n}",
         "line 2, col 1: warning: [LV01] condition is always false: [10, 10] meet [20, 20] is empty.",
         "Logic Verifier proves true branch dead; otherwise block executes and outputs '5'.")
    ]

    for t in c3_tests:
        story.append(make_test_card(*t))
        story.append(Spacer(1, 6))

    story.append(PageBreak())

    # ========================================================
    # CATEGORY 4: LOOP TRAPS & FLAWS (LV05 - LV09)
    # ========================================================
    story.append(Paragraph("Category 4: Loop Non-Termination, Overshoot &amp; Reversal Traps (LV05 - LV09)", h1_style))
    story.append(Paragraph(
        "Loops represent the highest danger surface for runtime hangs. J-- audits CFG back-edges using chaotic fixed-point iteration with interval widening and narrowing, halting the pipeline on provable infinite loops.",
        body_style
    ))

    c4_tests = [
        ("LV-05", "Loop Condition Initially Satisfied", "Category 4: Loops", "Logic Verifier (Phase 7)", "WARNING", "0", "LV05",
         "Induction variable initialized to 0; loop condition is 'until (counter equals 0)'; loop body never runs.",
         "num counter = 0;\nuntil (counter equals 0) {\n    show(counter);\n    counter++;\n}\nshow(100);",
         "line 2, col 1: warning: [LV05] loop body never executes: exit condition is already true on entry.",
         "Pipeline passes to RUN (&check;); outputs '100' directly; warning explains loop skipped."),

        ("LV-06", "Invariant Counter Infinite Loop", "Category 4: Loops", "Logic Verifier (Phase 7)", "FATAL ERROR", "1", "LV06",
         "Loop tests 'until (idx equals 10)', but variable 'idx' is never modified within the loop body.",
         "num idx = 0;\nuntil (idx equals 10) {\n    show(idx);\n    # Invariant: missing idx increment!\n}\nshow(1);",
         "line 2, col 1: error: [LV06] loop never terminates: condition variable 'idx' is invariant inside loop body.",
         "FATAL ERROR: Pipeline halts before optimizer/execution! Prevents infinite hang in TAC interpreter."),

        ("LV-07", "Induction Step Skips Loop Exit Target", "Category 4: Loops", "Logic Verifier (Phase 7)", "FATAL ERROR", "1", "LV07",
         "Counter increments from 0 by step 2 ('i = i + 2'); exit condition is 'i equals 7'.",
         "num i = 0;\nuntil (i equals 7) {\n    show(i);\n    i = i + 2; # 0, 2, 4, 6, 8... skips 7!\n}\nshow(i);",
         "line 2, col 1: error: [LV07] induction step skips loop exit: variable 'i' steps over target 7 (parity mismatch).",
         "Proves target overshoot mathematically: (7 - 0) % 2 != 0; halts pipeline at Phase 7."),

        ("LV-08", "Induction Diverges in Wrong Direction", "Category 4: Loops", "Logic Verifier (Phase 7)", "FATAL ERROR", "1", "LV08",
         "Loop counter starts at 0 with positive exit target 10, but decrements ('x--') each iteration.",
         "num x = 0;\nuntil (x equals 10) {\n    show(x);\n    x--; # Decrements away from positive 10!\n}\nshow(x);",
         "line 2, col 1: error: [LV08] loop moves in wrong direction: induction decrements away from target 10.",
         "FATAL ERROR: Interval widening tracks x in [-inf, 0]; exit target 10 is unreachable; halts pipeline."),

        ("LV-09", "Unreachable Dead Code After Endless Loop", "Category 4: Loops", "Logic Verifier (Phase 7)", "FATAL ERROR", "1", "LV06, LV09",
         "Combines an infinite loop with trailing code ('show(999);'); flags unreachable post-loop block.",
         "num k = 1;\nuntil (k equals 0) {\n    show(k);\n    k++; # k in [1, +inf], never reaches 0\n}\nshow(999); # Dead code",
         "line 2, col 1: error: [LV06] loop never terminates.\nline 6, col 1: error: [LV09] unreachable code after endless loop.",
         "Both LV06 and LV09 reported simultaneously; CFG post-dominator analysis proves block unreachable.")
    ]

    for t in c4_tests:
        story.append(make_test_card(*t))
        story.append(Spacer(1, 6))

    story.append(PageBreak())

    # ========================================================
    # CATEGORY 5: ARITHMETIC IMPOSSIBILITIES (LV10 - LV11)
    # ========================================================
    story.append(Paragraph("Category 5: Arithmetic Impossibilities &amp; Numerical Hazards (LV10 - LV11)", h1_style))
    story.append(Paragraph(
        "Division-by-zero crashes production systems. J-- distinguishes between <b>definite</b> zero divisors (LV10, fatal error) and <b>potential</b> zero divisors from unguarded user input (LV11, warning).",
        body_style
    ))

    c5_tests = [
        ("LV-10a", "Definite Constant Division by Zero", "Category 5: Arithmetic", "Logic Verifier (Phase 7)", "FATAL ERROR", "1", "LV10",
         "Direct division by literal constant 0 ('num quotient = numerator / denominator').",
         "num numerator = 500;\nnum denominator = 0;\nnum quotient = numerator / denominator;\nshow(quotient);",
         "line 3, col 16: error: [LV10] division by zero: divisor is exactly 0.",
         "Halts pipeline with Exit 1; prevents CPU SIGFPE crash; line 3 highlighted in crimson."),

        ("LV-10b", "Computed Zero Denominator Expression", "Category 5: Arithmetic", "Logic Verifier (Phase 7)", "FATAL ERROR", "1", "LV10",
         "Divisor is the result of subtraction between two identical variables ('num diff = a - b').",
         "num a = 12;\nnum b = 12;\nnum diff = a - b; # Evaluates to [0, 0]\nnum result = 100 / diff;\nshow(result);",
         "line 4, col 14: error: [LV10] division by zero: computed divisor 'diff' interval is [0, 0].",
         "Interval subtraction [12,12] - [12,12] collapses to [0,0]; flags fatal error."),

        ("LV-11a", "Potential Div-0 from Raw User Input", "Category 5: Arithmetic", "Logic Verifier (Phase 7)", "WARNING", "0", "LV11",
         "Divides by value obtained directly from 'input()'; interval [-inf, +inf] includes 0.",
         "num divisor = input();\nnum ratio = 1000 / divisor; # Potential division by zero\nshow(ratio);",
         "line 2, col 13: warning: [LV11] potential division by zero: divisor 'divisor' contains 0 in interval [-inf, +inf].",
         "Warning emitted without halting pipeline; notifies developer that input must be guarded."),

        ("LV-10c", "Chained Expression Collapsing to Zero", "Category 5: Arithmetic", "Logic Verifier (Phase 7)", "FATAL ERROR", "1", "LV10",
         "Sub-expression in denominator '(x - y)' evaluates to 0 where x=5 and y=5.",
         "num x = 5;\nnum y = 5;\nnum z = 50 / (x - y);\nshow(z);",
         "line 3, col 12: error: [LV10] division by zero: divisor is exactly 0.",
         "Three-Address Code temporary register t0 proven [0, 0]; compilation quarantined."),

        ("LV-11b", "Modulo by Unguarded User Input", "Category 5: Arithmetic", "Logic Verifier (Phase 7)", "WARNING", "0", "LV11",
         "Evaluates remainder modulo an unconstrained user input variable ('num rem = 45 % modulus').",
         "num modulus = input();\nnum rem = 45 % modulus;\nshow(rem);",
         "line 2, col 11: warning: [LV11] potential modulo by zero: divisor contains 0.",
         "Logic Verifier applies interval division checks to both slash '/' and percent '%' operators.")
    ]

    for t in c5_tests:
        story.append(make_test_card(*t))
        story.append(Spacer(1, 6))

    story.append(PageBreak())

    # ========================================================
    # CATEGORY 6: GUARDED SAFETY (FALSE ALARM IMMUNITY)
    # ========================================================
    story.append(Paragraph("Category 6: Guarded Safety &amp; False-Positive Immunity (Exit Code: 0)", h1_style))
    story.append(Paragraph(
        "A static analyzer that emits false alarms is discarded by developers. J-- implements <b>path-sensitive branch edge refinement</b>. When code is guarded by conditionals, zero false alarms are emitted.",
        body_style
    ))

    c6_tests = [
        ("GRD-01", "Guarded Division with '!= 0' Condition", "Category 6: Guarded Safety", "Native Run (Phase 9)", "CLEAN PASS", "0", "Zero Warnings",
         "Wraps input division inside 'check (divisor != 0)'; zero is excluded along the True CFG edge.",
         "num divisor = input();\ncheck (divisor != 0) {\n    num safeQuotient = 1000 / divisor;\n    show(safeQuotient);\n}\notherwise {\n    show(0);\n}",
         "Load and Link: 0 error(s), 0 warning(s). Logic Verifier: 0 error(s), 0 warning(s).\n[Execution Output]: (computes safeQuotient)",
         "Proves zero false alarms! Diagnostics count is 0; all pipeline phases pass &check; through RUN."),

        ("GRD-02", "Guarded Range Division ('den > 0')", "Category 6: Guarded Safety", "Native Run (Phase 9)", "CLEAN PASS", "0", "Zero Warnings",
         "Guards input division with 'den > 0'; interval refined to [1, +inf]; zero is completely excluded.",
         "num den = input();\ncheck (den > 0) {\n    num safeVal = 500 / den;\n    show(safeVal);\n}\nshow(1);",
         "Load and Link: 0 error(s), 0 warning(s). Logic Verifier: 0 error(s), 0 warning(s).",
         "Edge intersection sigma meet [1, +inf] leaves non-zero domain; verifier stays completely silent."),

        ("GRD-03", "Odd-Step Loop Exactly Hitting Exit", "Category 6: Guarded Safety", "Native Run (Phase 9)", "CLEAN PASS", "0", "Zero Warnings",
         "Initial value 1, step 2, target 9: passes through 1, 3, 5, 7, 9; exact termination proven.",
         "num i = 1;\nuntil (i equals 9) {\n    show(i);\n    i = i + 2;\n}\nshow(i);",
         "Clean execution; outputs: 1, 3, 5, 7, 9.\nLogic Verifier: 0 errors, 0 warnings.",
         "Proves LV07 is not triggered when induction math (9 - 1) % 2 == 0 is satisfied."),

        ("GRD-04", "Clean Fixed-Point Countdown Loop", "Category 6: Guarded Safety", "Native Run (Phase 9)", "CLEAN PASS", "0", "Zero Warnings",
         "Clean decrement loop counting down from 10 to 0; terminates in exactly 10 iterations.",
         "num count = 10;\nuntil (count equals 0) {\n    show(count);\n    count--;\n}\nshow(0);",
         "Clean execution; outputs numbers 10 down to 0.\nLogic Verifier: 0 errors, 0 warnings.",
         "Fixed-point loop iteration converges without widening distortion; clean pass."),

        ("GRD-05", "Safe Dual-Branch Variable Join", "Category 6: Guarded Safety", "Native Run (Phase 9)", "CLEAN PASS", "0", "Zero Warnings",
         "Initializes 'offset' to 5 on true edge and 2 on false edge; joined interval is [2, 5].",
         "num selector = input();\nnum offset = 0;\ncheck (selector > 10) {\n    offset = 5;\n}\notherwise {\n    offset = 2;\n}\nnum outcome = 20 / offset;\nshow(outcome);",
         "Load and Link: 0 error(s), 0 warning(s). Logic Verifier: 0 error(s), 0 warning(s).",
         "Lattice join [5,5] join [2,2] = [2,5]; 0 is not in [2,5]; division is proven safe.")
    ]

    for t in c6_tests:
        story.append(make_test_card(*t))
        story.append(Spacer(1, 6))

    story.append(PageBreak())

    # ========================================================
    # CATEGORY 7: MODULAR LOAD & LINK ERRORS (LV12 - LV16)
    # ========================================================
    story.append(Paragraph("Category 7: Modular Load &amp; Link Diagnostic Suite (LV12 - LV16)", h1_style))
    story.append(Paragraph(
        "J-- features a zero-C-dependency, data-driven built-in module registry. Phase 4 (Load & Link Verifier) validates imports and symbol usage, providing fuzzy Levenshtein suggestions for typos.",
        body_style
    ))

    c7_tests = [
        ("LV-12a", "Unimported Library Type 'word'", "Category 7: Load/Link", "Load & Link (Phase 4)", "FATAL ERROR", "1", "LV12",
         "Declares a variable of type 'word' without declaring 'import text;' at the top of the program.",
         "word greeting = \"hello world\";\nshow(greeting);",
         "line 1, col 1: error: [LV12] library 'text' is not imported, but type 'word' is used here. Add \"import text;\" at top.",
         "Fails at LOAD AND LINK; TAC Gen & downstream phases halted with 'Pipeline stopped before intermediate code.'"),

        ("LV-12b", "Unimported Library Function 'absolute'", "Category 7: Load/Link", "Load & Link (Phase 4)", "FATAL ERROR", "1", "LV12",
         "Calls math intrinsic 'absolute(neg)' without declaring 'import math;'.",
         "num neg = -45;\nnum pos = absolute(neg);\nshow(pos);",
         "line 2, col 11: error: [LV12] library 'math' is not imported, but function 'absolute' is used here. Add \"import math;\".",
         "LinkChecker intercepts unresolved intrinsic symbol; emits targeted import remediation."),

        ("LV-13", "Unknown Module Load Request", "Category 7: Load/Link", "Load & Link (Phase 4)", "FATAL ERROR", "1", "LV13",
         "Attempts to load nonexistent library 'graphics' ('import graphics;').",
         "import graphics;\nnum x = 10;\nshow(x);",
         "line 1, col 8: error: [LV13] load error: cannot load library 'graphics': no such library. Available: text, vector, math.",
         "Fails immediately at Phase 4. Emits list of all valid registered modules."),

        ("LV-14", "Typo with Fuzzy Levenshtein Suggestion", "Category 7: Load/Link", "Load & Link (Phase 4)", "FATAL ERROR", "1", "LV14",
         "Calls 'lenght(str)' instead of 'length(str)'; Levenshtein distance is 2 (transposed letters).",
         "import text;\nword str = \"analyzer\";\nnum len = lenght(str);\nshow(len);",
         "line 3, col 11: error: [LV14] undefined reference to function 'lenght'. Did you mean 'length' (library 'text')?",
         "Levenshtein algorithm computes edit distance D=2; offers exact library and function recommendation!"),

        ("LV-15/16", "Duplicate Import &amp; Unused Module", "Category 7: Load/Link", "Native Run (Phase 9)", "WARNING", "0", "LV15, LV16",
         "Imports 'text' twice (LV15) and imports 'math' but never uses any math functions (LV16).",
         "import text;\nimport text; # LV15 Duplicate\nimport math; # LV16 Unused\nword msg = \"systems\";\nshow(msg);",
         "line 2, col 8: warning: [LV15] library 'text' is already imported.\nline 3, col 8: warning: [LV16] library 'math' is imported but never used.",
         "Warnings emitted cleanly without halting compilation; program proceeds to RUN and outputs 'systems'.")
    ]

    for t in c7_tests:
        story.append(make_test_card(*t))
        story.append(Spacer(1, 6))

    story.append(PageBreak())

    # ========================================================
    # CATEGORY 8: VALID COMPLEX PROGRAMS (FULL EXECUTION)
    # ========================================================
    story.append(Paragraph("Category 8: Valid Complex Algorithmic Execution Suite (Exit Code: 0)", h1_style))
    story.append(Paragraph(
        "These five programs demonstrate complete pipeline execution from Lexer through Parser, Semantic Analysis, Load & Link, TAC Generation, CFG Construction, Logic Verification, Optimization, and Native Intermediate Execution.",
        body_style
    ))

    c8_tests = [
        ("VAL-01", "Text Standard Library Audit", "Category 8: Valid Run", "Native Run (Phase 9)", "SUCCESS", "0", "Clean Run",
         "Validates string processing with 'length()' and 'isempty()' intrinsic functions.",
         "import text;\nword title = \"LogicVerifier\";\nnum len = length(title);\nboolean empty = isempty(title);\nshow(len);\nshow(empty);",
         "[Execution Output]:\n13\nfalse\n(Load & Link: 0 errors. Logic Verifier: 0 errors)",
         "All 9 pipeline stage boxes turn white with checkmarks; Terminal displays exact string metrics."),

        ("VAL-02", "Mathematical Extremum & Absolute", "Category 8: Valid Run", "Native Run (Phase 9)", "SUCCESS", "0", "Clean Run",
         "Evaluates absolute value, maximum, and minimum intrinsics from the built-in 'math' library.",
         "import math;\nnum a = -75;\nnum b = 120;\nnum absA = absolute(a);\nnum maxVal = maximum(absA, b);\nnum minVal = minimum(absA, b);\nshow(absA);\nshow(maxVal);\nshow(minVal);",
         "[Execution Output]:\n75\n120\n75\n(All verification stages clean)",
         "Optimizer folds invariant TAC intrinsics; output displays mathematically exact calculations."),

        ("VAL-03", "Temperature Conversion & Threshold Alert", "Category 8: Valid Run", "Native Run (Phase 9)", "SUCCESS", "0", "Clean Run",
         "Converts Celsius to Fahrenheit: F = (C * 9) / 5 + 32, followed by threshold check.",
         "num celsius = 30;\nnum fahrenheit = (celsius * 9) / 5 + 32;\nshow(fahrenheit);\ncheck (fahrenheit > 80) {\n    show(1);\n}\notherwise {\n    show(0);\n}",
         "[Execution Output]:\n86\n1\n(Fahrenheit 86 > 80; alert branch taken)",
         "Arithmetic order of operations evaluated accurately; conditional branch executed cleanly."),

        ("VAL-04", "Iterative Fibonacci Generator (N=7)", "Category 8: Valid Run", "Native Run (Phase 9)", "SUCCESS", "0", "Clean Run",
         "Computes the 7th Fibonacci number iteratively using two sliding induction accumulators.",
         "num n = 7;\nnum prev1 = 1;\nnum prev2 = 0;\nnum current = 0;\nnum step = 1;\nuntil (step equals n) {\n    current = prev1 + prev2;\n    prev2 = prev1;\n    prev1 = current;\n    step++;\n}\nshow(current);",
         "[Execution Output]:\n13\n(Correct 7th Fibonacci sequence value)",
         "Induction step correctly terminates at step==7; zero non-termination warnings emitted."),

        ("VAL-05", "Hierarchical Access Decision Matrix", "Category 8: Valid Run", "Native Run (Phase 9)", "SUCCESS", "0", "Clean Run",
         "Evaluates nested check-otherwise statements to resolve user permission levels.",
         "num role = 2; # 1=Admin, 2=Developer, 3=Guest\nnum accessLevel = 0;\ncheck (role equals 1) {\n    accessLevel = 100;\n}\notherwise {\n    check (role equals 2) {\n        accessLevel = 50;\n    }\n    otherwise {\n        accessLevel = 10;\n    }\n}\nshow(accessLevel);",
         "[Execution Output]:\n50\n(Developer access privilege granted)",
         "Multi-level nested basic blocks linked in CFG; TAC jump threading optimizes nested branches.")
    ]

    for t in c8_tests:
        story.append(make_test_card(*t))
        story.append(Spacer(1, 6))

    doc.build(story, canvasmaker=NumberedCanvas)
    print(f"Test Suite PDF built successfully at: {output_path}")

if __name__ == '__main__':
    out_file = r"C:\Users\thaku\Downloads\J--_Dashboard_Test_Programs_Suite.pdf"
    build_test_suite_pdf(out_file)
