import os
import sys
from reportlab.lib.pagesizes import letter
from reportlab.lib import colors
from reportlab.lib.units import inch
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.platypus import (
    SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, PageBreak, KeepTogether, HRFlowable
)
from reportlab.graphics.shapes import Drawing, Rect, String, Line, Group, Polygon
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
            self.drawString(54, 11 * inch - 36, "J-- COMPILER: PIPELINE ARCHITECTURE & STATE FLOW")
            self.drawRightString(8.5 * inch - 54, 11 * inch - 36, "SYSTEM FLOWCHART & GATES")
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
        fontSize=11.5,
        leading=14.5,
        textColor=colors.HexColor("#0F172A"),
        spaceBefore=8,
        spaceAfter=4,
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
        fontSize=8,
        leading=11,
        textColor=colors.HexColor("#334155"),
        spaceAfter=4
    )

    table_cell = ParagraphStyle(
        'TableCell',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=7.2,
        leading=9.2,
        textColor=colors.HexColor("#1E293B")
    )

    table_cell_bold = ParagraphStyle(
        'TableCellBold',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=7.2,
        leading=9.2,
        textColor=colors.HexColor("#0F172A")
    )

    callout_style = ParagraphStyle(
        'Callout',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=7.5,
        leading=10.5,
        textColor=colors.HexColor("#1E3A8A")
    )

    story = []

    # Title & Authors
    story.append(Paragraph("J-- Compiler Architecture &amp; Execution State Flow", title_style))
    story.append(Paragraph("Complete 9-Stage Pipeline, Subsystem Dataflows, Quarantine Gates &amp; Data Transformations", subtitle_style))

    author_card = """
    <b>System Architects &amp; Researchers:</b><br/>
    &bull; <b>Utkarsh Vikram Singh</b> (Registration No: <b>24BCE2611</b>) - Core Compiler Pipeline, Abstract Interpretation &amp; Verifier Engine<br/>
    &bull; <b>Ishan Awasthi</b> (Registration No: <b>24BAI0207</b>) - Modular Load &amp; Link Registry, CFG Dataflow &amp; Optimizer Framework<br/>
    <b>Target Engine:</b> J-- Native Compiler (C++20, Intermediate Three-Address IR, CFG Abstract Interpreter, Multi-Pass Canonical Optimizer).
    """
    author_table = Table([[Paragraph(author_card, author_style)]], colWidths=[504])
    author_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor("#F8FAFC")),
        ('BOX', (0,0), (-1,-1), 1, colors.HexColor("#CBD5E1")),
        ('PADDING', (0,0), (-1,-1), 5),
    ]))
    story.append(author_table)
    story.append(Spacer(1, 6))

    # Architecture Visual Schematic
    story.append(Paragraph("1. High-Level Architectural Flowchart", h1_style))

    d = Drawing(504, 130)
    d.add(Rect(0, 0, 504, 130, fillColor=colors.HexColor("#F8FAFC"), strokeColor=colors.HexColor("#CBD5E1"), strokeWidth=1, rx=4, ry=4))

    # Row 1 stages
    r1 = [
        (10, 75, 80, 42, "1. Lexer", "Token Stream", False),
        (105, 75, 85, 42, "2. Parser", "Abstract Syntax", False),
        (205, 75, 90, 42, "3. Semantic", "Types & Scopes", False),
        (310, 75, 95, 42, "4. Load & Link", "[Stage 1 Verifier]", True),
        (420, 75, 74, 42, "5. TAC Gen", "3-Address IR", False),
    ]

    # Row 2 stages
    r2 = [
        (10, 15, 95, 42, "6. CFG Builder", "Basic Blocks", False),
        (120, 15, 120, 42, "7. Logic Verifier", "[Stage 2 Verifier]", True),
        (255, 15, 110, 42, "8. IR Optimizer", "Multi-Pass DCE", False),
        (380, 15, 114, 42, "9. Runtime VM", "stdout / Exit 0", False),
    ]

    for x, y, w, h, t1, t2, is_gate in r1:
        bg = colors.HexColor("#0F172A") if is_gate else colors.white
        tc1 = colors.white if is_gate else colors.HexColor("#0F172A")
        tc2 = colors.HexColor("#93C5FD") if is_gate else colors.HexColor("#475569")
        bc = colors.HexColor("#0F172A") if is_gate else colors.HexColor("#94A3B8")
        d.add(Rect(x, y, w, h, fillColor=bg, strokeColor=bc, strokeWidth=1.2, rx=3, ry=3))
        d.add(String(x + w/2, y + 24, t1, fontName="Helvetica-Bold", fontSize=8, textAnchor="middle", fillColor=tc1))
        d.add(String(x + w/2, y + 10, t2, fontName="Helvetica", fontSize=6.5, textAnchor="middle", fillColor=tc2))

    # Connectors Row 1
    d.add(Line(90, 96, 105, 96, strokeColor=colors.HexColor("#0F172A"), strokeWidth=1.5))
    d.add(Line(190, 96, 205, 96, strokeColor=colors.HexColor("#0F172A"), strokeWidth=1.5))
    d.add(Line(295, 96, 310, 96, strokeColor=colors.HexColor("#0F172A"), strokeWidth=1.5))
    d.add(Line(405, 96, 420, 96, strokeColor=colors.HexColor("#0F172A"), strokeWidth=1.5))

    for x, y, w, h, t1, t2, is_gate in r2:
        bg = colors.HexColor("#0F172A") if is_gate else colors.white
        tc1 = colors.white if is_gate else colors.HexColor("#0F172A")
        tc2 = colors.HexColor("#93C5FD") if is_gate else colors.HexColor("#475569")
        bc = colors.HexColor("#0F172A") if is_gate else colors.HexColor("#94A3B8")
        d.add(Rect(x, y, w, h, fillColor=bg, strokeColor=bc, strokeWidth=1.2, rx=3, ry=3))
        d.add(String(x + w/2, y + 24, t1, fontName="Helvetica-Bold", fontSize=8, textAnchor="middle", fillColor=tc1))
        d.add(String(x + w/2, y + 10, t2, fontName="Helvetica", fontSize=6.5, textAnchor="middle", fillColor=tc2))

    # Connectors Row 2
    d.add(Line(457, 75, 457, 62, strokeColor=colors.HexColor("#0F172A"), strokeWidth=1.5))
    d.add(Line(457, 62, 57, 62, strokeColor=colors.HexColor("#0F172A"), strokeWidth=1.5))
    d.add(Line(57, 62, 57, 57, strokeColor=colors.HexColor("#0F172A"), strokeWidth=1.5))

    d.add(Line(105, 36, 120, 36, strokeColor=colors.HexColor("#0F172A"), strokeWidth=1.5))
    d.add(Line(240, 36, 255, 36, strokeColor=colors.HexColor("#0F172A"), strokeWidth=1.5))
    d.add(Line(365, 36, 380, 36, strokeColor=colors.HexColor("#0F172A"), strokeWidth=1.5))

    story.append(d)
    story.append(Spacer(1, 6))

    # Table of 9 stages
    story.append(Paragraph("2. Detailed Step-by-Step Data Transformation Pipeline", h1_style))

    pipeline_data = [
        [Paragraph("Stage", table_cell_bold), Paragraph("Source File", table_cell_bold), Paragraph("Input Data", table_cell_bold), Paragraph("Transformation & Operation", table_cell_bold), Paragraph("Output Data", table_cell_bold), Paragraph("Failure Gate", table_cell_bold)],
        [
            Paragraph("<b>1. Lexer</b>", table_cell_bold),
            Paragraph("<code>lexer.cpp</code>", table_cell),
            Paragraph("Raw <code>.jmm</code> text", table_cell),
            Paragraph("DFA scanning, strips comments (<code>#...</code>), maps 25 keywords in O(1).", table_cell),
            Paragraph("<code>vector&lt;Token&gt;</code>", table_cell),
            Paragraph("Exit 1 on bad token / unclosed quote", table_cell)
        ],
        [
            Paragraph("<b>2. Parser</b>", table_cell_bold),
            Paragraph("<code>parser.cpp</code>", table_cell),
            Paragraph("Token stream", table_cell),
            Paragraph("Recursive-descent parsing. Enforces <code>import* statement* EOF</code>.", table_cell),
            Paragraph("AST Root (<code>Program</code>)", table_cell),
            Paragraph("Exit 1 on syntax / misplaced import", table_cell)
        ],
        [
            Paragraph("<b>3. Semantic</b>", table_cell_bold),
            Paragraph("<code>sema.cpp</code>", table_cell),
            Paragraph("AST", table_cell),
            Paragraph("Scope resolution, type checking, fixed mutation check, records <code>LibUse</code>.", table_cell),
            Paragraph("Decorated AST + <code>LibUse</code>", table_cell),
            Paragraph("Exit 1 on undeclared var / type mismatch", table_cell)
        ],
        [
            Paragraph("<b>4. Load & Link</b>", table_cell_bold),
            Paragraph("<code>link_check.cpp</code>", table_cell),
            Paragraph("Imports + <code>LibUse</code> + Registry", table_cell),
            Paragraph("Validates modules vs registry (text, math, vector). Fuzzy Levenshtein (D &le; 2).", table_cell),
            Paragraph("Resolved module bindings", table_cell),
            Paragraph("<b>QUARANTINE:</b> Exit 1 halts before TAC (LV12-LV14)", table_cell)
        ],
        [
            Paragraph("<b>5. TAC Gen</b>", table_cell_bold),
            Paragraph("<code>tac_gen.cpp</code>", table_cell),
            Paragraph("Clean AST", table_cell),
            Paragraph("Quadruple lowering, temporary virtual register allocation (<code>t0, t1...</code>).", table_cell),
            Paragraph("Linearized <code>TACProgram</code>", table_cell),
            Paragraph("Guaranteed clean by Semantic pass", table_cell)
        ],
        [
            Paragraph("<b>6. CFG Builder</b>", table_cell_bold),
            Paragraph("<code>cfg.cpp</code>", table_cell),
            Paragraph("TAC instructions", table_cell),
            Paragraph("Leader identification, basic block partitioning, edge/back-edge construction.", table_cell),
            Paragraph("Control Flow Graph", table_cell),
            Paragraph("Deterministic graph structure", table_cell)
        ],
        [
            Paragraph("<b>7. Logic Verifier</b>", table_cell_bold),
            Paragraph("<code>verifier.cpp</code>", table_cell),
            Paragraph("CFG", table_cell),
            Paragraph("Interval lattice D_int = [lo, hi], edge refinement, loop widening/narrowing.", table_cell),
            Paragraph("Verified CFG + Proofs", table_cell),
            Paragraph("Exit 1 on fatal loop flaws / div-0 (LV06-LV10)", table_cell)
        ],
        [
            Paragraph("<b>8. Optimizer</b>", table_cell_bold),
            Paragraph("<code>optimizer.cpp</code>", table_cell),
            Paragraph("Verified CFG", table_cell),
            Paragraph("Constant folding, algebraic identities (x*0 &rarr; 0), Dead Code Elimination (DCE).", table_cell),
            Paragraph("Canonical Optimized TAC", table_cell),
            Paragraph("Preserves semantic equivalence", table_cell)
        ],
        [
            Paragraph("<b>9. Runtime VM</b>", table_cell_bold),
            Paragraph("<code>tac_interp.cpp</code>", table_cell),
            Paragraph("Optimized TAC + stdin", table_cell),
            Paragraph("Virtual machine fetch-decode-execute loop, intrinsic dispatch for built-in libs.", table_cell),
            Paragraph("Final Output (stdout)", table_cell),
            Paragraph("Clean Exit 0 on normal termination", table_cell)
        ],
    ]

    pipe_table = Table(pipeline_data, colWidths=[60, 68, 80, 150, 76, 70])
    pipe_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), colors.HexColor("#0F172A")),
        ('TEXTCOLOR', (0,0), (-1,0), colors.white),
        ('GRID', (0,0), (-1,-1), 0.5, colors.HexColor("#CBD5E1")),
        ('PADDING', (0,0), (-1,-1), 2.5),
        ('ROWBACKGROUNDS', (0,1), (-1,-1), [colors.white, colors.HexColor("#F8FAFC")]),
    ]))
    story.append(pipe_table)
    story.append(Spacer(1, 6))

    # Architectural Gates Box
    gates_text = """
    <b>The Two Mandatory Architectural Gates:</b><br/>
    &bull; <b>Gate 1 (Intermediate Code Quarantine - Stage 4):</b> If an unimported type (<code>word</code>), unknown library (<code>import banana;</code>), or undefined function is used, the pipeline immediately halts with exit code 1 and prints: <i>'Pipeline stopped before intermediate code.'</i> No corrupt IR or unverified pointer can ever enter TAC generation.<br/>
    &bull; <b>Gate 2 (Logic Verifier Pre-Optimization Placement - Stage 7):</b> Runs directly on the CFG before the optimizer. This solves the <i>'Optimizer Paradox'</i> where traditional Dead Code Elimination (DCE) silently deletes buggy blocks (e.g. <code>x &gt; 50 and x &lt; 20</code>) without notifying the developer. J-- catches and proves bugs mathematically prior to dead code pruning.
    """
    gates_table = Table([[Paragraph(gates_text, callout_style)]], colWidths=[504])
    gates_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor("#EFF6FF")),
        ('BOX', (0,0), (-1,-1), 1, colors.HexColor("#93C5FD")),
        ('PADDING', (0,0), (-1,-1), 5),
    ]))
    story.append(gates_table)

    doc.build(story, canvasmaker=NumberedCanvas)
    print(f"Architecture PDF built successfully at: {output_path}")

if __name__ == '__main__':
    out_file = r"C:\Users\thaku\Downloads\J--_Compiler_Working_Architecture_Diagram.pdf"
    build_pdf(out_file)

