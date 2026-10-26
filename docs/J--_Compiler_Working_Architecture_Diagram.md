# J-- Compiler Architecture & Execution State Flow

**System Authors:** Utkarsh Vikram Singh (24BCE2611) & Ishan Awasthi (24BAI0207)
**Target Engine:** J-- Native Compiler (C++20, Intermediate Three-Address IR, CFG Abstract Interpreter, Multi-Pass Canonical Optimizer)
**Specification:** End-to-End 9-Stage Pipeline, Subsystem Dataflows, Quarantine Gates & Data Transformations

---

## 1. Complete Architecture State & Dataflow Diagram

```mermaid
flowchart TD
    %% Source Code Input
    SRC(["📄 Source Code (.jmm)"]):::inputStyle

    %% Stage 1: Lexer
    subgraph STAGE_1 ["Stage 1: Lexer (src/lexer.cpp)"]
        LEX["Character DFA Scanner<br/>Keyword Map (25 Keywords)<br/>Regex Tokenizer"]
    end

    SRC -->|Character Stream| LEX
    LEX -->|Lexical Error| ERR_LEX["❌ Halt Exit 1<br/>(Unrecognized Token / Unclosed String)"]:::errStyle

    %% Stage 2: Parser
    subgraph STAGE_2 ["Stage 2: Parser (src/parser.cpp)"]
        PARSE["Recursive Descent Parser<br/>Pratt Operator Precedence<br/>Top-Level Import Invariant"]
    end

    LEX -->|Token Stream| PARSE
    PARSE -->|Syntax Error| ERR_PARSE["❌ Halt Exit 1<br/>(Missing Brace / Misplaced Import)"]:::errStyle

    %% Stage 3: Semantic Analyzer
    subgraph STAGE_3 ["Stage 3: Semantic Analyzer (src/sema.cpp)"]
        SEMA["Scope Resolution<br/>Type Checking & Immutability<br/>Populate Symbol Table"]
        LIBUSE_REC[("Side-Table: LibUse<br/>(Symbol, Kind, Provider, Pos)")]:::sideTable
    end

    PARSE -->|Abstract Syntax Tree (AST)| SEMA
    SEMA -.->|Record Features| LIBUSE_REC
    SEMA -->|Semantic Error| ERR_SEMA["❌ Halt Exit 1<br/>(Type Mismatch / Undeclared Var)"]:::errStyle

    %% Stage 4: Load & Link Verifier
    subgraph STAGE_4 ["Stage 4: Load & Link Verifier (src/link_check.cpp)"]
        REG[("Library Registry<br/>text | math | vector")]:::sideTable
        LOAD_PASS["1. LOAD Pass: Check Imports vs Registry<br/>(Detect LV13 Unknown, LV15 Duplicate)"]
        LINK_PASS["2. LINK Pass: Check LibUse vs Loaded Libs<br/>(Detect LV12 Unimported, LV16 Unused)"]
        FUZZY["3. Levenshtein Typo Matcher<br/>(D ≤ 2 Suggestion for LV14)"]
    end

    SEMA -->|Decorated AST| LOAD_PASS
    REG -.->|Query Modules| LOAD_PASS
    LIBUSE_REC -.->|Audit Uses| LINK_PASS
    LOAD_PASS --> LINK_PASS --> FUZZY

    FUZZY -->|Fatal LV12, LV13, LV14| ERR_QUARANTINE["🛑 QUARANTINE HALT (Exit 1)<br/>'Pipeline stopped before intermediate code'"]:::quarantineStyle

    %% Stage 5: TAC Generator
    subgraph STAGE_5 ["Stage 5: TAC Generator (src/tac_gen.cpp)"]
        TAC_GEN["AST-to-IR Lowering<br/>Temp Register Alloc (t0, t1...)<br/>Linearized Quadruples"]
    end

    FUZZY -->|Module Linkage Clean| TAC_GEN

    %% Stage 6: CFG Builder
    subgraph STAGE_6 ["Stage 6: CFG Builder (src/cfg.cpp)"]
        CFG_BUILD["Leader Identification<br/>Basic Block Partitioning<br/>Edge & Loop Back-Edge Wiring"]
    end

    TAC_GEN -->|Linearized TAC Program| CFG_BUILD

    %% Stage 7: Logic Verifier
    subgraph STAGE_7 ["Stage 7: Logic Verifier (src/verifier.cpp)"]
        INTERVAL["Interval Abstract Domain D_int<br/>[lo, hi] Bounds Lattice"]
        EDGE_REF["Path-Sensitive Edge Refinement<br/>True: σ ⊓ cond | False: σ ⊓ ¬cond"]
        FIXED_PT["Chaotic Loop Fixed-Point Iteration<br/>Widening (∇) & Narrowing (Δ)"]
        FLAW_DET["Flaw Detection & Proof Generator<br/>(LV01-LV04 Condition, LV05-LV09 Loops, LV10-LV11 Div-0)"]
    end

    CFG_BUILD -->|Control Flow Graph| INTERVAL
    INTERVAL --> EDGE_REF --> FIXED_PT --> FLAW_DET

    FLAW_DET -->|Fatal Errors LV06, LV07, LV08, LV09, LV10| ERR_LOGIC["❌ Halt Exit 1<br/>(Loop Overshoot / Div-by-Zero)"]:::errStyle

    %% Stage 8: Optimizer
    subgraph STAGE_8 ["Stage 8: IR Optimizer (src/optimizer.cpp)"]
        OPT["Constant Folding & Propagation<br/>Algebraic Identities (x * 0 → 0)<br/>Dead Code Elimination (DCE)<br/>Branch Folding & Jump Threading"]
    end

    FLAW_DET -->|Passed / Warnings Only| OPT

    %% Stage 9: Native Execution
    subgraph STAGE_9 ["Stage 9: TAC Interpreter (src/tac_interp.cpp)"]
        VM["Virtual Machine Bytecode Dispatch<br/>Built-in Intrinsics Execution<br/>Standard I/O Streams"]
    end

    OPT -->|Optimized Canonical TAC| VM
    STDIN[("⌨️ stdin Inputs")]:::inputStyle -.-> VM
    VM --> OUT(["🖥️ Program Execution Output (stdout)<br/>Exit Code 0"]):::outputStyle

    %% Node Styles
    classDef inputStyle fill:#F1F5F9,stroke:#0F172A,stroke-width:2px,color:#0F172A,font-weight:bold;
    classDef outputStyle fill:#DCFCE7,stroke:#16A34A,stroke-width:2px,color:#14532D,font-weight:bold;
    classDef errStyle fill:#FEE2E2,stroke:#DC2626,stroke-width:1.5px,color:#7F1D1D;
    classDef quarantineStyle fill:#111827,stroke:#DC2626,stroke-width:2px,color:#FFFFFF,font-weight:bold;
    classDef sideTable fill:#FEF3C7,stroke:#D97706,stroke-width:1.5px,color:#78350F;
```

---

## 2. Step-by-Step Data Transformation Pipeline

| Stage                       | What Goes In                                | Core Operation                                                                         | What Comes Out                               | Failure Gate                                                               |
| :-------------------------- | :------------------------------------------ | :------------------------------------------------------------------------------------- | :------------------------------------------- | :------------------------------------------------------------------------- |
| **1. Lexer**          | Raw `.jmm` source text                    | DFA regex scanning, comments stripped (`#`)                                          | `std::vector<Token>`                       | Halts with Exit 1 on bad tokens.                                           |
| **2. Parser**         | Tokens                                      | Recursive descent, imports-first grammar rule                                          | Abstract Syntax Tree (`Program`)           | Halts with Exit 1 on syntax errors.                                        |
| **3. Semantic**       | AST                                         | Scope tracking, type checks, mutability (`fixed`)                                    | Symbol Table + `LibUse` table              | Halts with Exit 1 on type mismatch.                                        |
| **4. Load & Link**    | `Program.imports` + `LibUse` side-table | Validates against internal registry (`text`, `math`), Levenshtein typo match       | Resolved library bindings                    | **QUARANTINE:** Halts with Exit 1 before TAC if LV12, LV13, or LV14. |
| **5. TAC Gen**        | Validated AST                               | Quadruple lowering, virtual registers `t0, t1...`                                    | Linearized Three-Address Code                | N/A (Guaranteed by Semantic pass).                                         |
| **6. CFG Builder**    | Linearized TAC                              | Finds leaders, builds Basic Blocks & jump edges                                        | Control Flow Graph (CFG)                     | N/A.                                                                       |
| **7. Logic Verifier** | CFG                                         | Interval abstract domain $[lo, hi]$, branch edge refinement, loop widening/narrowing | Proved invariants & diagnostics (LV01–LV11) | Halts with Exit 1 on fatal loop / div-0 flaws.                             |
| **8. Optimizer**      | Verified CFG                                | Constant folding, algebraic reduction, Dead Code Elimination                           | Canonical Optimized TAC                      | Optional `--optcheck` equivalence check.                                 |
| **9. Runtime VM**     | Optimized TAC + Inputs                      | Virtual machine fetch-decode-execute loop                                              | Final output (stdout)                        | Clean Exit 0.                                                              |

---

## 3. The Two Critical Architectural Gates

### Gate 1: The Intermediate Code Quarantine (Stage 4)

- **Files Responsible:** `src/link_check.cpp`, `src/library_registry.cpp`, `src/main.cpp`
- **Mechanism:** If an unimported type (`word`), unknown library (`import banana;`), or undefined function is used, the pipeline immediately stops with:
  ```
  	flowchart TDPipeline stopped before intermediate code.	flowchart TD
  ```
- 
- 

    end



    SRC -->|Character Stream| LEX

    LEX -->|Lexical Error| ERR_LEX["❌ Halt Exit 1<br/>(Unrecognized Token / Unclosed String)"]:::errStyle



    %% Stage 2: Parser

    subgraph STAGE_2 ["Stage 2: Parser (src/parser.cpp)"]

        PARSE["Recursive Descent Parser<br/>Pratt Operator Precedence<br/>Top-Level Import Invariant"]

    end



    LEX -->|Token Stream| PARSE

    PARSE -->|Syntax Error| ERR_PARSE["❌ Halt Exit 1<br/>(Missing Brace / Misplaced Import)"]:::errStyle



    %% Stage 3: Semantic Analyzer

    subgraph STAGE_3 ["Stage 3: Semantic Analyzer (src/sema.cpp)"]

        SEMA["Scope Resolution<br/>Type Checking & Immutability<br/>Populate Symbol Table"]

        LIBUSE_REC[("Side-Table: LibUse<br/>(Symbol, Kind, Provider, Pos)")]:::sideTable

    end



    PARSE -->|Abstract Syntax Tree (AST)| SEMA

    SEMA -.->|Record Features| LIBUSE_REC

    SEMA -->|Semantic Error| ERR_SEMA["❌ Halt Exit 1<br/>(Type Mismatch / Undeclared Var)"]:::errStyle



    %% Stage 4: Load & Link Verifier

    subgraph STAGE_4 ["Stage 4: Load & Link Verifier (src/link_check.cpp)"]

        REG[("Library Registry<br/>text | math | vector")]:::sideTable

        LOAD_PASS["1. LOAD Pass: Check Imports vs Registry<br/>(Detect LV13 Unknown, LV15 Duplicate)"]

        LINK_PASS["2. LINK Pass: Check LibUse vs Loaded Libs<br/>(Detect LV12 Unimported, LV16 Unused)"]

        FUZZY["3. Levenshtein Typo Matcher<br/>(D ≤ 2 Suggestion for LV14)"]

    end



    SEMA -->|Decorated AST| LOAD_PASS

    REG -.->|Query Modules| LOAD_PASS

    LIBUSE_REC -.->|Audit Uses| LINK_PASS

    LOAD_PASS --> LINK_PASS --> FUZZY



    FUZZY -->|Fatal LV12, LV13, LV14| ERR_QUARANTINE["🛑 QUARANTINE HALT (Exit 1)<br/>'Pipeline stopped before intermediate code'"]:::quarantineStyle



    %% Stage 5: TAC Generator

    subgraph STAGE_5 ["Stage 5: TAC Generator (src/tac_gen.cpp)"]

        TAC_GEN["AST-to-IR Lowering<br/>Temp Register Alloc (t0, t1...)<br/>Linearized Quadruples"]

    end



    FUZZY -->|Module Linkage Clean| TAC_GEN



    %% Stage 6: CFG Builder

    subgraph STAGE_6 ["Stage 6: CFG Builder (src/cfg.cpp)"]

        CFG_BUILD["Leader Identification<br/>Basic Block Partitioning<br/>Edge & Loop Back-Edge Wiring"]

    end



    TAC_GEN -->|Linearized TAC Program| CFG_BUILD



    %% Stage 7: Logic Verifier

    subgraph STAGE_7 ["Stage 7: Logic Verifier (src/verifier.cpp)"]

        INTERVAL["Interval Abstract Domain D_int<br/>[lo, hi] Bounds Lattice"]

        EDGE_REF["Path-Sensitive Edge Refinement<br/>True: σ ⊓ cond | False: σ ⊓ ¬cond"]

        FIXED_PT["Chaotic Loop Fixed-Point Iteration<br/>Widening (∇) & Narrowing (Δ)"]

        FLAW_DET["Flaw Detection & Proof Generator<br/>(LV01-LV04 Condition, LV05-LV09 Loops, LV10-LV11 Div-0)"]

    end



    CFG_BUILD -->|Control Flow Graph| INTERVAL

    INTERVAL --> EDGE_REF --> FIXED_PT --> FLAW_DET



    FLAW_DET -->|Fatal Errors LV06, LV07, LV08, LV09, LV10| ERR_LOGIC["❌ Halt Exit 1<br/>(Loop Overshoot / Div-by-Zero)"]:::errStyle



    %% Stage 8: Optimizer

    subgraph STAGE_8 ["Stage 8: IR Optimizer (src/optimizer.cpp)"]

        OPT["Constant Folding & Propagation<br/>Algebraic Identities (x * 0 → 0)<br/>Dead Code Elimination (DCE)<br/>Branch Folding & Jump Threading"]

    end



    FLAW_DET -->|Passed / Warnings Only| OPT



    %% Stage 9: Native Execution

    subgraph STAGE_9 ["Stage 9: TAC Interpreter (src/tac_interp.cpp)"]

        VM["Virtual Machine Bytecode Dispatch<br/>Built-in Intrinsics Execution<br/>Standard I/O Streams"]

    end



    OPT -->|Optimized Canonical TAC| VM

    STDIN[("⌨️ stdin Inputs")]:::inputStyle -.-> VM

    VM --> OUT(["🖥️ Program Execution Output (stdout)<br/>Exit Code 0"]):::outputStyle



    %% Node Styles

    classDef inputStyle fill:#F1F5F9,stroke:#0F172A,stroke-width:2px,color:#0F172A,font-weight:bold;

    classDef outputStyle fill:#DCFCE7,stroke:#16A34A,stroke-width:2px,color:#14532D,font-weight:bold;

    classDef errStyle fill:#FEE2E2,stroke:#DC2626,stroke-width:1.5px,color:#7F1D1D;

    classDef quarantineStyle fill:#111827,stroke:#DC2626,stroke-width:2px,color:#FFFFFF,font-weight:bold;

    classDef sideTable fill:#FEF3C7,stroke:#D97706,stroke-width:1.5px,color:#78350F;flowchart TD

    %% Source Code Input

    SRC(["📄 Source Code (.jmm)"]):::inputStyle



    %% Stage 1: Lexer

    subgraph STAGE_1 ["Stage 1: Lexer (src/lexer.cpp)"]

        LEX["Character DFA Scanner<br/>Keyword Map (25 Keywords)<br/>Regex Tokenizer"]

    end



    SRC -->|Character Stream| LEX

    LEX -->|Lexical Error| ERR_LEX["❌ Halt Exit 1<br/>(Unrecognized Token / Unclosed String)"]:::errStyle



    %% Stage 2: Parser

    subgraph STAGE_2 ["Stage 2: Parser (src/parser.cpp)"]

        PARSE["Recursive Descent Parser<br/>Pratt Operator Precedence<br/>Top-Level Import Invariant"]

    end



    LEX -->|Token Stream| PARSE

    PARSE -->|Syntax Error| ERR_PARSE["❌ Halt Exit 1<br/>(Missing Brace / Misplaced Import)"]:::errStyle



    %% Stage 3: Semantic Analyzer

    subgraph STAGE_3 ["Stage 3: Semantic Analyzer (src/sema.cpp)"]

        SEMA["Scope Resolution<br/>Type Checking & Immutability<br/>Populate Symbol Table"]

        LIBUSE_REC[("Side-Table: LibUse<br/>(Symbol, Kind, Provider, Pos)")]:::sideTable

    end



    PARSE -->|Abstract Syntax Tree (AST)| SEMA

    SEMA -.->|Record Features| LIBUSE_REC

    SEMA -->|Semantic Error| ERR_SEMA["❌ Halt Exit 1<br/>(Type Mismatch / Undeclared Var)"]:::errStyle



    %% Stage 4: Load & Link Verifier

    subgraph STAGE_4 ["Stage 4: Load & Link Verifier (src/link_check.cpp)"]

        REG[("Library Registry<br/>text | math | vector")]:::sideTable

        LOAD_PASS["1. LOAD Pass: Check Imports vs Registry<br/>(Detect LV13 Unknown, LV15 Duplicate)"]

        LINK_PASS["2. LINK Pass: Check LibUse vs Loaded Libs<br/>(Detect LV12 Unimported, LV16 Unused)"]

        FUZZY["3. Levenshtein Typo Matcher<br/>(D ≤ 2 Suggestion for LV14)"]

    end



    SEMA -->|Decorated AST| LOAD_PASS

    REG -.->|Query Modules| LOAD_PASS

    LIBUSE_REC -.->|Audit Uses| LINK_PASS

    LOAD_PASS --> LINK_PASS --> FUZZY



    FUZZY -->|Fatal LV12, LV13, LV14| ERR_QUARANTINE["🛑 QUARANTINE HALT (Exit 1)<br/>'Pipeline stopped before intermediate code'"]:::quarantineStyle



    %% Stage 5: TAC Generator

    subgraph STAGE_5 ["Stage 5: TAC Generator (src/tac_gen.cpp)"]

        TAC_GEN["AST-to-IR Lowering<br/>Temp Register Alloc (t0, t1...)<br/>Linearized Quadruples"]

    end



    FUZZY -->|Module Linkage Clean| TAC_GEN



    %% Stage 6: CFG Builder

    subgraph STAGE_6 ["Stage 6: CFG Builder (src/cfg.cpp)"]

        CFG_BUILD["Leader Identification<br/>Basic Block Partitioning<br/>Edge & Loop Back-Edge Wiring"]

    end



    TAC_GEN -->|Linearized TAC Program| CFG_BUILD



    %% Stage 7: Logic Verifier

    subgraph STAGE_7 ["Stage 7: Logic Verifier (src/verifier.cpp)"]

        INTERVAL["Interval Abstract Domain D_int<br/>[lo, hi] Bounds Lattice"]

        EDGE_REF["Path-Sensitive Edge Refinement<br/>True: σ ⊓ cond | False: σ ⊓ ¬cond"]

        FIXED_PT["Chaotic Loop Fixed-Point Iteration<br/>Widening (∇) & Narrowing (Δ)"]

        FLAW_DET["Flaw Detection & Proof Generator<br/>(LV01-LV04 Condition, LV05-LV09 Loops, LV10-LV11 Div-0)"]

    end



    CFG_BUILD -->|Control Flow Graph| INTERVAL

    INTERVAL --> EDGE_REF --> FIXED_PT --> FLAW_DET



    FLAW_DET -->|Fatal Errors LV06, LV07, LV08, LV09, LV10| ERR_LOGIC["❌ Halt Exit 1<br/>(Loop Overshoot / Div-by-Zero)"]:::errStyle



    %% Stage 8: Optimizer

    subgraph STAGE_8 ["Stage 8: IR Optimizer (src/optimizer.cpp)"]

        OPT["Constant Folding & Propagation<br/>Algebraic Identities (x * 0 → 0)<br/>Dead Code Elimination (DCE)<br/>Branch Folding & Jump Threading"]

    end



    FLAW_DET -->|Passed / Warnings Only| OPT



    %% Stage 9: Native Execution

    subgraph STAGE_9 ["Stage 9: TAC Interpreter (src/tac_interp.cpp)"]

        VM["Virtual Machine Bytecode Dispatch<br/>Built-in Intrinsics Execution<br/>Standard I/O Streams"]

    end



    OPT -->|Optimized Canonical TAC| VM

    STDIN[("⌨️ stdin Inputs")]:::inputStyle -.-> VM

    VM --> OUT(["🖥️ Program Execution Output (stdout)<br/>Exit Code 0"]):::outputStyle



    %% Node Styles

    classDef inputStyle fill:#F1F5F9,stroke:#0F172A,stroke-width:2px,color:#0F172A,font-weight:bold;

    classDef outputStyle fill:#DCFCE7,stroke:#16A34A,stroke-width:2px,color:#14532D,font-weight:bold;

    classDef errStyle fill:#FEE2E2,stroke:#DC2626,stroke-width:1.5px,color:#7F1D1D;

    classDef quarantineStyle fill:#111827,stroke:#DC2626,stroke-width:2px,color:#FFFFFF,font-weight:bold;

    classDef sideTable fill:#FEF3C7,stroke:#D97706,stroke-width:1.5px,color:#78350F;flowchart TD

    %% Source Code Input

    SRC(["📄 Source Code (.jmm)"]):::inputStyle



    %% Stage 1: Lexer

    subgraph STAGE_1 ["Stage 1: Lexer (src/lexer.cpp)"]

        LEX["Character DFA Scanner<br/>Keyword Map (25 Keywords)<br/>Regex Tokenizer"]

    end



    SRC -->|Character Stream| LEX

    LEX -->|Lexical Error| ERR_LEX["❌ Halt Exit 1<br/>(Unrecognized Token / Unclosed String)"]:::errStyle



    %% Stage 2: Parser

    subgraph STAGE_2 ["Stage 2: Parser (src/parser.cpp)"]

        PARSE["Recursive Descent Parser<br/>Pratt Operator Precedence<br/>Top-Level Import Invariant"]

    end



    LEX -->|Token Stream| PARSE

    PARSE -->|Syntax Error| ERR_PARSE["❌ Halt Exit 1<br/>(Missing Brace / Misplaced Import)"]:::errStyle



    %% Stage 3: Semantic Analyzer

    subgraph STAGE_3 ["Stage 3: Semantic Analyzer (src/sema.cpp)"]

        SEMA["Scope Resolution<br/>Type Checking & Immutability<br/>Populate Symbol Table"]

        LIBUSE_REC[("Side-Table: LibUse<br/>(Symbol, Kind, Provider, Pos)")]:::sideTable

    end



    PARSE -->|Abstract Syntax Tree (AST)| SEMA

    SEMA -.->|Record Features| LIBUSE_REC

    SEMA -->|Semantic Error| ERR_SEMA["❌ Halt Exit 1<br/>(Type Mismatch / Undeclared Var)"]:::errStyle



    %% Stage 4: Load & Link Verifier

    subgraph STAGE_4 ["Stage 4: Load & Link Verifier (src/link_check.cpp)"]

        REG[("Library Registry<br/>text | math | vector")]:::sideTable

        LOAD_PASS["1. LOAD Pass: Check Imports vs Registry<br/>(Detect LV13 Unknown, LV15 Duplicate)"]

        LINK_PASS["2. LINK Pass: Check LibUse vs Loaded Libs<br/>(Detect LV12 Unimported, LV16 Unused)"]

        FUZZY["3. Levenshtein Typo Matcher<br/>(D ≤ 2 Suggestion for LV14)"]

    end



    SEMA -->|Decorated AST| LOAD_PASS

    REG -.->|Query Modules| LOAD_PASS

    LIBUSE_REC -.->|Audit Uses| LINK_PASS

    LOAD_PASS --> LINK_PASS --> FUZZY



    FUZZY -->|Fatal LV12, LV13, LV14| ERR_QUARANTINE["🛑 QUARANTINE HALT (Exit 1)<br/>'Pipeline stopped before intermediate code'"]:::quarantineStyle



    %% Stage 5: TAC Generator

    subgraph STAGE_5 ["Stage 5: TAC Generator (src/tac_gen.cpp)"]

        TAC_GEN["AST-to-IR Lowering<br/>Temp Register Alloc (t0, t1...)<br/>Linearized Quadruples"]

    end



    FUZZY -->|Module Linkage Clean| TAC_GEN



    %% Stage 6: CFG Builder

    subgraph STAGE_6 ["Stage 6: CFG Builder (src/cfg.cpp)"]

        CFG_BUILD["Leader Identification<br/>Basic Block Partitioning<br/>Edge & Loop Back-Edge Wiring"]

    end



    TAC_GEN -->|Linearized TAC Program| CFG_BUILD



    %% Stage 7: Logic Verifier

    subgraph STAGE_7 ["Stage 7: Logic Verifier (src/verifier.cpp)"]

        INTERVAL["Interval Abstract Domain D_int<br/>[lo, hi] Bounds Lattice"]

        EDGE_REF["Path-Sensitive Edge Refinement<br/>True: σ ⊓ cond | False: σ ⊓ ¬cond"]

        FIXED_PT["Chaotic Loop Fixed-Point Iteration<br/>Widening (∇) & Narrowing (Δ)"]

        FLAW_DET["Flaw Detection & Proof Generator<br/>(LV01-LV04 Condition, LV05-LV09 Loops, LV10-LV11 Div-0)"]

    end



    CFG_BUILD -->|Control Flow Graph| INTERVAL

    INTERVAL --> EDGE_REF --> FIXED_PT --> FLAW_DET



    FLAW_DET -->|Fatal Errors LV06, LV07, LV08, LV09, LV10| ERR_LOGIC["❌ Halt Exit 1<br/>(Loop Overshoot / Div-by-Zero)"]:::errStyle



    %% Stage 8: Optimizer

    subgraph STAGE_8 ["Stage 8: IR Optimizer (src/optimizer.cpp)"]

        OPT["Constant Folding & Propagation<br/>Algebraic Identities (x * 0 → 0)<br/>Dead Code Elimination (DCE)<br/>Branch Folding & Jump Threading"]

    end



    FLAW_DET -->|Passed / Warnings Only| OPT



    %% Stage 9: Native Execution

    subgraph STAGE_9 ["Stage 9: TAC Interpreter (src/tac_interp.cpp)"]

        VM["Virtual Machine Bytecode Dispatch<br/>Built-in Intrinsics Execution<br/>Standard I/O Streams"]

    end



    OPT -->|Optimized Canonical TAC| VM

    STDIN[("⌨️ stdin Inputs")]:::inputStyle -.-> VM

    VM --> OUT(["🖥️ Program Execution Output (stdout)<br/>Exit Code 0"]):::outputStyle



    %% Node Styles

    classDef inputStyle fill:#F1F5F9,stroke:#0F172A,stroke-width:2px,color:#0F172A,font-weight:bold;

    classDef outputStyle fill:#DCFCE7,stroke:#16A34A,stroke-width:2px,color:#14532D,font-weight:bold;

    classDef errStyle fill:#FEE2E2,stroke:#DC2626,stroke-width:1.5px,color:#7F1D1D;

    classDef quarantineStyle fill:#111827,stroke:#DC2626,stroke-width:2px,color:#FFFFFF,font-weight:bold;

    classDef sideTable fill:#FEF3C7,stroke:#D97706,stroke-width:1.5px,color:#78350F;

- **Guaranteed Invariant:** No invalid function pointers, missing symbols, or unverified types can ever enter Three-Address Code generation or CFG construction.

### Gate 2: The Logic Verifier Pre-Optimization Placement (Stage 7)

- **Files Responsible:** `src/verifier.cpp`, `src/interval.cpp`, `src/abstract_state.h`, `src/cfg.cpp`
- **Mechanism:** Operates on the Control Flow Graph before the optimizer.
- **The Optimizer Paradox Solved:** Classical compilers let Dead Code Elimination (DCE) silently delete unreachable or contradictory blocks (e.g., `x > 50 and x < 20`). By executing the Logic Verifier *before* the optimizer, J-- catches and proves bugs to the developer with exact invariant proofs before the optimizer cleans up dead code.
