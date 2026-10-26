import os
import sys
from reportlab.lib.pagesizes import letter, A4
from reportlab.lib import colors
from reportlab.lib.units import inch
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.platypus import (
    SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, PageBreak, KeepTogether, HRFlowable
)
from reportlab.graphics.shapes import Drawing, Rect, String, Line, Group, Polygon
from reportlab.pdfgen import canvas

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
        self.setFillColor(colors.HexColor("#64748B"))
        
        # Header (on pages after cover)
        if self._pageNumber > 1:
            self.drawString(54, 11 * inch - 36, "J-- COMPILER: LOGIC VERIFIER & LOAD-LINK ARCHITECTURE")
            self.drawRightString(8.5 * inch - 54, 11 * inch - 36, "RESEARCH & SYSTEM DESIGN SPECIFICATION")
            self.setStrokeColor(colors.HexColor("#CBD5E1"))
            self.setLineWidth(0.75)
            self.line(54, 11 * inch - 42, 8.5 * inch - 54, 11 * inch - 42)

        # Footer (on all pages)
        self.setStrokeColor(colors.HexColor("#CBD5E1"))
        self.setLineWidth(0.75)
        self.line(54, 46, 8.5 * inch - 54, 46)
        
        self.setFont("Helvetica", 8)
        self.setFillColor(colors.HexColor("#64748B"))
        self.drawString(54, 32, "J-- Language & Formal Compiler Verification Project | Confidential Technical Report")
        page_text = f"Page {self._pageNumber} of {page_count}"
        self.drawRightString(8.5 * inch - 54, 32, page_text)
        self.restoreState()

def build_pdf(output_path):
    doc = SimpleDocTemplate(
        output_path,
        pagesize=letter,
        leftMargin=54,
        rightMargin=54,
        topMargin=54,
        bottomMargin=54
    )

    styles = getSampleStyleSheet()

    # Custom styles
    title_style = ParagraphStyle(
        'DocTitle',
        parent=styles['Heading1'],
        fontName='Helvetica-Bold',
        fontSize=24,
        leading=28,
        textColor=colors.HexColor("#0F172A"),
        spaceAfter=6
    )

    subtitle_style = ParagraphStyle(
        'DocSubtitle',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=12,
        leading=16,
        textColor=colors.HexColor("#334155"),
        spaceAfter=14
    )

    author_style = ParagraphStyle(
        'AuthorMeta',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=9,
        leading=13,
        textColor=colors.HexColor("#1E293B")
    )

    h1_style = ParagraphStyle(
        'Heading1Custom',
        parent=styles['Heading1'],
        fontName='Helvetica-Bold',
        fontSize=15,
        leading=19,
        textColor=colors.HexColor("#0F172A"),
        spaceBefore=14,
        spaceAfter=8,
        keepWithNext=True
    )

    h2_style = ParagraphStyle(
        'Heading2Custom',
        parent=styles['Heading2'],
        fontName='Helvetica-Bold',
        fontSize=11,
        leading=15,
        textColor=colors.HexColor("#1E3A8A"),
        spaceBefore=10,
        spaceAfter=4,
        keepWithNext=True
    )

    body_style = ParagraphStyle(
        'BodyCustom',
        parent=styles['BodyText'],
        fontName='Helvetica',
        fontSize=9.5,
        leading=13.5,
        textColor=colors.HexColor("#1E293B"),
        spaceAfter=6
    )

    bullet_style = ParagraphStyle(
        'BulletCustom',
        parent=body_style,
        leftIndent=14,
        bulletIndent=4,
        spaceAfter=4
    )

    callout_style = ParagraphStyle(
        'CalloutText',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=9,
        leading=13,
        textColor=colors.HexColor("#0F172A")
    )

    code_style = ParagraphStyle(
        'CodeText',
        parent=styles['Normal'],
        fontName='Courier',
        fontSize=8.5,
        leading=11.5,
        textColor=colors.HexColor("#0F172A")
    )

    table_cell = ParagraphStyle(
        'TableCell',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=8,
        leading=11,
        textColor=colors.HexColor("#1E293B")
    )

    table_cell_bold = ParagraphStyle(
        'TableCellBold',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=8,
        leading=11,
        textColor=colors.HexColor("#0F172A")
    )

    story = []

    # Title & Metadata Card
    story.append(Paragraph("J-- Language & Formal Compiler Architecture", title_style))
    story.append(Paragraph("A Unified Approach to Static Module Link Verification and Intermediate Abstract Interpretation", subtitle_style))

    # Author Box
    author_text = """
    <b>Authors & Research Engineers:</b> Utkarsh Vikram Singh (24BCE2611) &nbsp;|&nbsp; Ishan Awasthi (24BAI0207)<br/>
    <b>Target Architecture:</b> J-- Systems Compiler (C++20 Native Engine, Strict Dataflow Verifier, Multi-Pass IR Optimizer)
    """
    author_table = Table(
        [[Paragraph(author_text, author_style)]],
        colWidths=[504]
    )
    author_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor("#F1F5F9")),
        ('BOX', (0,0), (-1,-1), 1, colors.HexColor("#CBD5E1")),
        ('PADDING', (0,0), (-1,-1), 8),
        ('BOTTOMPADDING', (0,0), (-1,-1), 8),
    ]))
    story.append(author_table)
    story.append(Spacer(1, 12))

    # SECTION 1: PROBLEM STATEMENT
    story.append(Paragraph("1. Problem Statement: The Semantic Blindspot of Modern Compilers", h1_style))
    prob_p1 = """
    Modern production compilers (such as GCC, Clang, and standard Java/C++ frontends) excel at verifying structural syntax and lexical grammar. However, they possess a catastrophic blindspot: <b>they are indifferent to program logic, latent numerical failures, and modular linkage integrity</b>. A program that compiles with zero warnings can harbor fatal algorithmic contradictions, non-terminating loops, impossible branches, or guaranteed division-by-zero traps that only trigger during production deployment.
    """
    story.append(Paragraph(prob_p1, body_style))

    prob_box = """
    <b>The Core Dilemma:</b> Traditional static analysis tools (linters) exist outside the compiler pipeline as optional, post-hoc linters. Because linters lack deep integration into the compiler's intermediate representation (IR) and control flow graph (CFG), they produce copious false positives or miss deep path-dependent states. Conversely, classical compiler optimizers run <i>after</i> intermediate code generation and prioritize code reduction over diagnostics: they quietly prune unreachable code blocks and fold constants without informing the software engineer of the underlying logical defect.
    """
    prob_table = Table([[Paragraph(prob_box, callout_style)]], colWidths=[504])
    prob_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor("#FEF2F2")),
        ('BOX', (0,0), (-1,-1), 1, colors.HexColor("#FCA5A5")),
        ('PADDING', (0,0), (-1,-1), 8),
    ]))
    story.append(prob_table)
    story.append(Spacer(1, 10))

    # SECTION 2: THE J-- PARADIGM & ARCHITECTURAL UNIQUENESS
    story.append(Paragraph("2. The J-- Paradigm: Mandatory In-Pipeline Verification", h1_style))
    story.append(Paragraph(
        "J-- is a modern, statically typed systems language engineered to eliminate runtime surprises through deterministic compiler verification. Instead of relegating safety checks to external analyzers or runtime exception handlers, J-- elevates verification into <b>mandatory, first-class compilation phases</b>. Specifically, J-- introduces two groundbreaking compiler phases:",
        body_style
    ))

    story.append(Paragraph("• <b>Phase 4 (Load and Link Verification):</b> A data-driven registry phase that statically resolves imports, types, and function symbols, blocking malicious or missing references before intermediate code generation.", bullet_style))
    story.append(Paragraph("• <b>Phase 5 (The Logic Verifier):</b> A formal static analyzer operating on the Control Flow Graph via Interval Abstract Interpretation and Tri-State Logic, positioned between Three-Address Code generation and the IR Optimizer.", bullet_style))
    story.append(Spacer(1, 8))

    # ARCHITECTURE DIAGRAM
    story.append(Paragraph("3. Compiler Pipeline Architecture & Pipeline Placement", h1_style))
    story.append(Paragraph(
        "The following diagram illustrates the complete 9-stage pipeline of the J-- compiler, highlighting the strategic placement of the Load/Link Checker and the Logic Verifier:",
        body_style
    ))

    # Draw Architecture Box Flow
    d = Drawing(504, 110)
    # Background card
    d.add(Rect(0, 0, 504, 110, fillColor=colors.HexColor("#F8FAFC"), strokeColor=colors.HexColor("#E2E8F0"), strokeWidth=1, rx=4, ry=4))
    
    # Stages definition: (x, y, w, h, text1, text2, is_special)
    stages_row1 = [
        (10, 60, 68, 36, "1. Lexer", "Tokens", False),
        (88, 60, 68, 36, "2. Parser", "AST Tree", False),
        (166, 60, 76, 36, "3. Semantic", "Types & Scope", False),
        (252, 60, 116, 36, "4. Load & Link", "[Stage 1 Verifier]", True),
        (378, 60, 116, 36, "5. TAC Gen", "3-Address IR", False),
    ]

    stages_row2 = [
        (10, 12, 100, 36, "6. CFG Builder", "Basic Blocks", False),
        (120, 12, 136, 36, "7. Logic Verifier", "[Stage 2 AI Verifier]", True),
        (266, 12, 116, 36, "8. IR Optimizer", "Multi-Pass DCE", False),
        (392, 12, 102, 36, "9. Execution", "TAC Interpreter", False),
    ]

    for (x, y, w, h, t1, t2, special) in stages_row1:
        bg = colors.HexColor("#0F172A") if special else colors.HexColor("#FFFFFF")
        stroke = colors.HexColor("#0F172A") if special else colors.HexColor("#94A3B8")
        txt1 = colors.HexColor("#FFFFFF") if special else colors.HexColor("#0F172A")
        txt2 = colors.HexColor("#93C5FD") if special else colors.HexColor("#64748B")
        d.add(Rect(x, y, w, h, fillColor=bg, strokeColor=stroke, strokeWidth=1.5 if special else 1, rx=3, ry=3))
        d.add(String(x + w/2, y + 21, t1, textAnchor='middle', fontName='Helvetica-Bold', fontSize=8, fillColor=txt1))
        d.add(String(x + w/2, y + 9, t2, textAnchor='middle', fontName='Helvetica', fontSize=7, fillColor=txt2))

    for (x, y, w, h, t1, t2, special) in stages_row2:
        bg = colors.HexColor("#0F172A") if special else colors.HexColor("#FFFFFF")
        stroke = colors.HexColor("#0F172A") if special else colors.HexColor("#94A3B8")
        txt1 = colors.HexColor("#FFFFFF") if special else colors.HexColor("#0F172A")
        txt2 = colors.HexColor("#93C5FD") if special else colors.HexColor("#64748B")
        d.add(Rect(x, y, w, h, fillColor=bg, strokeColor=stroke, strokeWidth=1.5 if special else 1, rx=3, ry=3))
        d.add(String(x + w/2, y + 21, t1, textAnchor='middle', fontName='Helvetica-Bold', fontSize=8, fillColor=txt1))
        d.add(String(x + w/2, y + 9, t2, textAnchor='middle', fontName='Helvetica', fontSize=7, fillColor=txt2))

    # Flow arrows
    d.add(Line(78, 78, 88, 78, strokeColor=colors.HexColor("#0F172A"), strokeWidth=1.5))
    d.add(Line(156, 78, 166, 78, strokeColor=colors.HexColor("#0F172A"), strokeWidth=1.5))
    d.add(Line(242, 78, 252, 78, strokeColor=colors.HexColor("#0F172A"), strokeWidth=1.5))
    d.add(Line(368, 78, 378, 78, strokeColor=colors.HexColor("#0F172A"), strokeWidth=1.5))
    
    # Turn down to row 2
    d.add(Line(436, 60, 436, 52, strokeColor=colors.HexColor("#0F172A"), strokeWidth=1.5))
    d.add(Line(436, 52, 60, 52, strokeColor=colors.HexColor("#0F172A"), strokeWidth=1.5))
    d.add(Line(60, 52, 60, 48, strokeColor=colors.HexColor("#0F172A"), strokeWidth=1.5))

    d.add(Line(110, 30, 120, 30, strokeColor=colors.HexColor("#0F172A"), strokeWidth=1.5))
    d.add(Line(256, 30, 266, 30, strokeColor=colors.HexColor("#0F172A"), strokeWidth=1.5))
    d.add(Line(382, 30, 392, 30, strokeColor=colors.HexColor("#0F172A"), strokeWidth=1.5))

    story.append(d)
    story.append(Spacer(1, 8))

    # PLACEMENT JUSTIFICATION CALLOUT
    story.append(Paragraph("Strategic Placement Justification:", h2_style))
    story.append(Paragraph(
        "<b>1. Why Load/Link runs before TAC Generation:</b> Intermediate code generation assumes a statically closed world. If an unimported type (e.g. <code>word</code> without <code>import text;</code>) or an undefined library function reaches TAC generation, the emitter either produces invalid quadruples or requires fallback stubs. Halting at Phase 4 ensures zero corrupt IR is emitted.",
        body_style
    ))
    story.append(Paragraph(
        "<b>2. Why Logic Verification runs after CFG and BEFORE the Optimizer:</b><br/>"
        "• <i>Why NOT on AST?</i> AST lacks explicit loop back-edges, basic block convergence points, and unified phi-like states. CFG decomposes jumps into distinct predecessors and successors, enabling formal mathematical fixed-point convergence.<br/>"
        "• <i>Why NOT after Optimizer?</i> If the optimizer runs first, constant folding and Dead Code Elimination (DCE) eradicate faulty branches (e.g. eliminating an always-false condition and deleting its body). The developer never sees a diagnostic, while the underlying logic bug remains undetected. The Goldilocks zone is precisely between CFG construction and optimization.",
        body_style
    ))
    story.append(Spacer(1, 10))

    # SECTION 4: LOAD AND LINK VERIFIER
    story.append(Paragraph("4. Stage 1 Verifier: Built-in Module Registry & Load-Link System", h1_style))
    story.append(Paragraph(
        "In modern systems programming, external dynamic linkers often fail at runtime due to mismatched shared libraries or misspelled symbols. J-- resolves this by embedding a <b>Data-Driven Library Registry</b> directly into the compiler engine. Libraries are not separate binary DLLs; they are formal static specifications integrated into the compilation universe.",
        body_style
    ))

    # Table of Built-in Libraries
    lib_data = [
        [Paragraph("Library", table_cell_bold), Paragraph("Types Provided", table_cell_bold), Paragraph("Core Function Signatures", table_cell_bold), Paragraph("Purpose & Design Need", table_cell_bold)],
        [Paragraph("<code>text</code>", table_cell), Paragraph("<code>word</code>", table_cell), Paragraph("<code>length(word) -> num<br/>isempty(word) -> boolean</code>", table_cell), Paragraph("Safe heap-allocated string operations with explicit length verification.", table_cell)],
        [Paragraph("<code>math</code>", table_cell), Paragraph("None (extends <code>num</code>, <code>decimal</code>)", table_cell), Paragraph("<code>absolute(num) -> num<br/>maximum(num, num) -> num<br/>minimum(num, num) -> num</code>", table_cell), Paragraph("Essential numerical algorithms with arithmetic overflow safeguards.", table_cell)],
        [Paragraph("<code>vector</code>", table_cell), Paragraph("<code>list</code>", table_cell), Paragraph("<code>append(list, any)<br/>pop(list) -> any</code>", table_cell), Paragraph("Dynamically sized sequential arrays with bounds-checked access.", table_cell)]
    ]
    lib_table = Table(lib_data, colWidths=[60, 80, 160, 204])
    lib_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), colors.HexColor("#F1F5F9")),
        ('GRID', (0,0), (-1,-1), 0.5, colors.HexColor("#CBD5E1")),
        ('PADDING', (0,0), (-1,-1), 5),
    ]))
    story.append(lib_table)
    story.append(Spacer(1, 8))

    story.append(Paragraph("Formal Load and Link Rules (LV12 – LV16):", h2_style))
    link_rules_data = [
        [Paragraph("Rule", table_cell_bold), Paragraph("Severity", table_cell_bold), Paragraph("Condition & Linker Behavior", table_cell_bold), Paragraph("Diagnostic Message Example", table_cell_bold)],
        [Paragraph("<b>LV12</b>", table_cell), Paragraph("FATAL ERROR", table_cell_bold), Paragraph("Missing Import: Using a registered type or function without explicit <code>import &lt;lib&gt;;</code>.", table_cell), Paragraph("<i>library 'text' is not imported, but type 'word' is used here</i>", table_cell)],
        [Paragraph("<b>LV13</b>", table_cell), Paragraph("FATAL ERROR", table_cell_bold), Paragraph("Unknown Library: Importing a name not present in the compiler registry.", table_cell), Paragraph("<i>cannot load library 'banana': no such library. Available: text, vector, math</i>", table_cell)],
        [Paragraph("<b>LV14</b>", table_cell), Paragraph("FATAL ERROR", table_cell_bold), Paragraph("Undefined Reference: Function call not found in library. Computes Levenshtein distance for typo suggestion.", table_cell), Paragraph("<i>undefined reference to 'lengt'. Did you mean 'length' (library 'text')?</i>", table_cell)],
        [Paragraph("<b>LV15</b>", table_cell), Paragraph("WARNING", table_cell), Paragraph("Duplicate Import: Importing the same module multiple times in the compilation unit.", table_cell), Paragraph("<i>library 'text' is already imported (first imported on line 1)</i>", table_cell)],
        [Paragraph("<b>LV16</b>", table_cell), Paragraph("WARNING", table_cell), Paragraph("Unused Import: Importing a library but referencing none of its types or functions.", table_cell), Paragraph("<i>library 'math' is imported but never used</i>", table_cell)]
    ]
    link_rules_table = Table(link_rules_data, colWidths=[40, 75, 185, 204])
    link_rules_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), colors.HexColor("#F1F5F9")),
        ('GRID', (0,0), (-1,-1), 0.5, colors.HexColor("#CBD5E1")),
        ('PADDING', (0,0), (-1,-1), 5),
    ]))
    story.append(link_rules_table)
    story.append(Spacer(1, 10))

    # SECTION 5: THE LOGIC VERIFIER
    story.append(Paragraph("5. Stage 2 Verifier: Abstract Interpretation on the CFG", h1_style))
    story.append(Paragraph(
        "The Logic Verifier is the flagship innovation of the J-- compiler. Operating directly on the Control Flow Graph, it tracks variable properties through an <b>Interval Abstract Domain</b> combined with <b>Tri-State Logic</b> (<code>True</code>, <code>False</code>, <code>Unknown</code>).",
        body_style
    ))

    story.append(Paragraph("How the Analysis Engine Functions:", h2_style))
    story.append(Paragraph(
        "1. <b>Abstract State Representation:</b> Each numeric variable $x$ is mapped to an interval $[min, max] \\subseteq [-\\infty, +\\infty]$. Boolean conditions evaluate to a TriBool value.<br/>"
        "2. <b>Transfer Functions:</b> Arithmetic quadruples apply rigorous interval arithmetic. For example, if $a \\in [2, 5]$ and $b \\in [10, 20]$, then $a + b \\in [12, 25]$.<br/>"
        "3. <b>Branch Edge Refinement:</b> At conditional branch nodes (<code>check</code>, <code>until</code>), the verifier forks the abstract state and refines intervals along both True and False successor edges. For instance, the True branch of <code>check (x > 5)</code> constrains $x \\in [6, \\infty)$.<br/>"
        "4. <b>Fixed-Point Loop Convergence:</b> For cyclic graphs (loops), predecessor states are joined at loop entry nodes until variable states achieve mathematical fixed-point convergence.",
        body_style
    ))
    story.append(Spacer(1, 8))

    story.append(Paragraph("Formal Logic Verifier Danger Levels & Rules (LV01 – LV11):", h2_style))
    logic_rules_data = [
        [Paragraph("Code", table_cell_bold), Paragraph("Severity", table_cell_bold), Paragraph("Formal Vulnerability Definition", table_cell_bold), Paragraph("Compiler Proof & Diagnostic Output", table_cell_bold)],
        [Paragraph("<b>LV01</b>", table_cell), Paragraph("WARNING", table_cell), Paragraph("Contradiction: Branch condition is proven always false across all entry paths.", table_cell), Paragraph("<i>condition is always false: x > 10 contradicts x &lt; 5 (branch never runs)</i>", table_cell)],
        [Paragraph("<b>LV02</b>", table_cell), Paragraph("WARNING", table_cell), Paragraph("Tautology: Condition is proven always true; otherwise block is unreachable.", table_cell), Paragraph("<i>condition is always true: 'x' is always 5 here (otherwise never runs)</i>", table_cell)],
        [Paragraph("<b>LV03</b>", table_cell), Paragraph("WARNING", table_cell), Paragraph("Self-Comparison: Comparing identical operands (<code>x equals x</code>).", table_cell), Paragraph("<i>self-comparison is always true: comparing 'x' to itself</i>", table_cell)],
        [Paragraph("<b>LV04</b>", table_cell), Paragraph("WARNING", table_cell), Paragraph("Redundant Condition: One conjunct subsumes another in logical AND/OR.", table_cell), Paragraph("<i>one operand of and/or is redundant: x > 5 is already implied by x > 10</i>", table_cell)],
        [Paragraph("<b>LV05</b>", table_cell), Paragraph("WARNING", table_cell), Paragraph("Loop Never Runs: J-- <code>until (c)</code> loop exits on true. If true on entry, body is dead.", table_cell), Paragraph("<i>loop body never runs: exit condition is already true on loop entry</i>", table_cell)],
        [Paragraph("<b>LV06</b>", table_cell), Paragraph("FATAL ERROR", table_cell_bold), Paragraph("Infinite Loop: Condition is constant false and no reachable <code>stop</code> exists.", table_cell), Paragraph("<i>loop never terminates: condition is constant false and no exit exists</i>", table_cell)],
        [Paragraph("<b>LV07</b>", table_cell), Paragraph("FATAL ERROR", table_cell_bold), Paragraph("Exit Overshoot: Step increments skip the exact equality exit test.", table_cell), Paragraph("<i>step skips loop exit: counter starts at 0, steps by 2, misses target 5</i>", table_cell)],
        [Paragraph("<b>LV08</b>", table_cell), Paragraph("FATAL ERROR", table_cell_bold), Paragraph("Wrong Direction: Induction variable step moves strictly away from exit bound.", table_cell), Paragraph("<i>wrong direction: step moves 'x' away from target value 10</i>", table_cell)],
        [Paragraph("<b>LV09</b>", table_cell), Paragraph("WARNING", table_cell), Paragraph("Unreachable Post-Loop: Code statements positioned after an endless loop.", table_cell), Paragraph("<i>unreachable code after non-terminating loop</i>", table_cell)],
        [Paragraph("<b>LV10</b>", table_cell), Paragraph("FATAL ERROR", table_cell_bold), Paragraph("Definite Division by Zero: Divisor interval is proven to be exactly $[0, 0]$.", table_cell), Paragraph("<i>division by zero: divisor is provably 0</i>", table_cell)],
        [Paragraph("<b>LV11</b>", table_cell), Paragraph("WARNING", table_cell), Paragraph("Unchecked Input Divisor: User input used directly without non-zero check.", table_cell), Paragraph("<i>possible division by zero: divisor from input() is unvalidated</i>", table_cell)]
    ]
    logic_rules_table = Table(logic_rules_data, colWidths=[38, 70, 196, 200])
    logic_rules_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), colors.HexColor("#F1F5F9")),
        ('GRID', (0,0), (-1,-1), 0.5, colors.HexColor("#CBD5E1")),
        ('PADDING', (0,0), (-1,-1), 4.5),
    ]))
    story.append(logic_rules_table)
    story.append(Spacer(1, 10))

    # SECTION 6: ZERO FALSE ALARMS & THE OPTIMIZER
    story.append(Paragraph("6. Precision Verification: Zero False Alarms & IR Optimizer", h1_style))
    story.append(Paragraph(
        "A static analyzer that produces false alarms will be disabled by developers. J-- achieves near-zero false alarms by acknowledging programmer guards:",
        body_style
    ))
    story.append(Paragraph(
        "• <b>Guarded Division Safety:</b> In <code>num n = input(); check (n > 0) { show(100 / n); }</code>, the verifier refines $n \\in [1, \\infty)$ inside the branch and emits <b>ZERO warnings</b> for LV10 or LV11.<br/>"
        "• <b>Reachable Loop Exits:</b> In <code>until (false) { check (x > 5) stop; }</code>, the presence of a reachable <code>stop</code> edge clears LV06, proving the loop has a valid exit path.",
        body_style
    ))
    story.append(Spacer(1, 6))

    story.append(Paragraph("Interaction with Phase 8 (The Multi-Pass Optimizer):", h2_style))
    story.append(Paragraph(
        "Once Phase 7 verifies that the program contains no fatal logical defects, Phase 8 optimizes the TAC representation through iterative rounds of constant folding, algebraic identity reduction ($x * 0 \\to 0$, $x + 0 \\to x$), branch folding, and jump cleanup. Because the Logic Verifier ran first, optimizations are guaranteed to be semantics-preserving and never obscure developer bugs.",
        body_style
    ))
    story.append(Spacer(1, 10))

    # SECTION 7: SUMMARY CONCLUSION
    story.append(Paragraph("7. Architectural Summary & Impact", h1_style))
    conclusion_text = """
    <b>Conclusion:</b> The J-- compiler demonstrates that compile-time verification can be both exhaustive and highly practical. By integrating <b>Load/Link Validation</b> at the modular boundary and <b>Logic Verification on the Control Flow Graph</b> prior to optimization, J-- prevents an entire class of runtime bugs, infinite loops, and hardware exceptions. The architecture proves that compilers need not be silent conduits of developer oversights—they can be active, mathematically sound partners in constructing trustworthy software.
    """
    conclusion_table = Table([[Paragraph(conclusion_text, callout_style)]], colWidths=[504])
    conclusion_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor("#EFF6FF")),
        ('BOX', (0,0), (-1,-1), 1, colors.HexColor("#BFDBFE")),
        ('PADDING', (0,0), (-1,-1), 8),
    ]))
    story.append(conclusion_table)

    # Build Document
    doc.build(story, canvasmaker=NumberedCanvas)
    print(f"Report built successfully at: {output_path}")

if __name__ == '__main__':
    downloads_dir = r'C:\Users\thaku\Downloads'
    out_file = os.path.join(downloads_dir, 'J--_Compiler_Logic_Verifier_Report.pdf')
    build_pdf(out_file)

