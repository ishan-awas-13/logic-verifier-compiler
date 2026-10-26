# ⚡ J-- (JMM): A Modern Statically-Typed Programming Language & Provably-Verified Compiler

[![Language: J--](https://img.shields.io/badge/Language-J----0052CC?style=for-the-badge&logo=codeigniter&logoColor=white)](https://github.com)
[![Standard: C++20](https://img.shields.io/badge/Standard-C%2B%2B20-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)](https://en.cppreference.com/w/cpp/20)
[![Verification: Abstract Interpretation](<https://img.shields.io/badge/Verifier-Abstract%20Interpretation-8A2BE2?style=for-the-badge>)](https://en.wikipedia.org/wiki/Abstract_interpretation)
[![Status: Production Ready](https://img.shields.io/badge/Status-Complete-success?style=for-the-badge)](#)

---

## 👨‍💻 Language Creators & System Architects

We designed and implemented **J--** from scratch — our own custom programming language featuring an end-to-end industrial compiler toolchain, native intermediate representation (TAC), abstract interpretation verification engine, multi-pass canonical optimizer, and interactive live projector dashboard.

* **Utkarsh Vikram Singh** (`24BCE2611`) — *Compiler Frontend, Intermediate Code Generation (TAC), Load & Link Quarantine Subsystem, and Execution Virtual Machine.*
* **Ishan Awasthi** (`24BAI0207`) — *Control Flow Graph (CFG) Construction, Interval Abstract Interpretation Domain, Logic Verifier Engine (LV01–LV16), and Multi-Pass Canonical Optimizer.*

---

## 📖 Table of Contents

1. [The Big Idea &amp; Philosophy](#1-the-big-idea--philosophy)
2. [End-to-End Compiler Architecture (Mermaid &amp; Dataflow)](#2-end-to-end-compiler-architecture)
3. [The 9 Compilation Phases in Detail](#3-the-9-compilation-phases-in-detail)
4. [The Load &amp; Link Quarantine Subsystem](#4-the-load--link-quarantine-subsystem)
5. [The Logic Verifier Engine (LV01–LV16 Rules)](#5-the-logic-verifier-engine-lv01lv16-rules)
6. [How to Start This Project From 0 (Setup &amp; Build Guide)](#6-how-to-start-this-project-from-0-setup--build-guide)
7. [Language Syntax &amp; Grammar Cheatsheet](#7-language-syntax--grammar-cheatsheet)
8. [Comprehensive Test Suite &amp; Sample Programs](#8-comprehensive-test-suite--sample-programs)
   - [A. Early Frontend Syntax &amp; Semantic Errors](#a-early-frontend-syntax--semantic-errors)
   - [B. Load &amp; Link Checks (LV12–LV16)](#b-load--link-checks-lv12lv16)
   - [C. Logic Verifier Checks (LV01–LV11)](#c-logic-verifier-checks-lv01lv11)
   - [D. Guarded Code (Zero False Positives)](#d-guarded-code-zero-false-positives)
   - [E. Valid Showcase Applications](#e-valid-showcase-applications)

---

## 1. The Big Idea & Philosophy

### Why J--?

Most programming languages force developers into a false dichotomy:

1. **Low-Level Languages (C/C++):** Offer raw performance and control, but are plagued by cryptic syntax, dangerous undefined behaviors, unconstrained pointer arithmetic, and silent arithmetic flaws (division by zero, integer overflows, out-of-bounds access).
2. **High-Level Languages (Python/JavaScript):** Offer human-readable keywords and rapid prototyping, but defer critical errors to dynamic runtime exceptions, leading to catastrophic runtime crashes in production.

**J--** was created to bridge this divide. It introduces:

* **Clean, Semantic English Keywords:** Replacing cryptic punctuation with intent-revealing keywords (`num`, `word`, `decimal`, `letter`, `boolean`, `check`, `otherwise`, `until`, `fixed`, `show`, `give`).
* **The "Zero-Tolerance" Soundness Invariant:** Catching arithmetic traps, infinite loops, unreachable states, and linkage inconsistencies **at compile-time** before machine execution.

### The Optimizer Paradox

In traditional optimizing compilers (such as GCC or Clang), optimization passes run on unverified intermediate representations. When an optimizer encounters undefined behaviors (e.g., division by zero or infinite loops), it frequently invokes the standard's permission to optimize aggressively:

* Dead branches containing logical flaws are silently pruned.
* Infinite loops without side-effects are eliminated.
* Division-by-zero operations are assumed to be unreachable, distorting program logic.

This phenomenon is known as the **Optimizer Paradox**: *The smarter the optimizer is, the more effectively it conceals developer errors.*

### The J-- Architectural Solution

J-- fundamentally solves this by enforcing an architectural invariant:

$$
\text{Source Code} \longrightarrow \text{Load/Link Quarantine} \longrightarrow \text{CFG & Logic Verification} \longrightarrow \text{Optimization} \longrightarrow \text{Execution}
$$

1. **Gate 1 (Stage 4 Quarantine):** Unresolved modules, unimported library symbols, or typoed library identifiers trigger an immediate quarantine halt before Three-Address Code (TAC) can ever be emitted.
2. **Gate 2 (Stage 7 Logic Verifier):** Uses **Abstract Interpretation over an Interval Lattice** to formally prove bounds, detecting division-by-zero, out-of-bounds indexing, loop overshoots, and infinite loops *strictly prior* to the Optimizer.
3. **Stage 8 Canonical Optimizer:** Only receives a verified, provably clean control-flow graph where optimization is both safe and semantics-preserving.

---

## 2. End-to-End Compiler Architecture

The diagram below illustrates the complete 9-stage pipeline, intermediate data structures, error gates, and the intermediate code quarantine mechanism:

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

## 3. The 9 Compilation Phases in Detail

| Stage                       | Responsible Source Files                             | Input Artifact                         | Core Operations & Algorithms                                                                                                      | Output Artifact                                    | Failure Exit Gate                                                                           |
| :-------------------------- | :--------------------------------------------------- | :------------------------------------- | :-------------------------------------------------------------------------------------------------------------------------------- | :------------------------------------------------- | :------------------------------------------------------------------------------------------ |
| **1. Lexer**          | `src/lexer.cpp`, `src/lexer.h`                   | Raw `.jmm` source text               | DFA character scanning, comment stripping (`#` line, `$ ... $` block), keyword resolution                                     | Token Stream `std::vector<Token>`                | Halts with Exit 1 on unrecognized characters or unclosed strings.                           |
| **2. Parser**         | `src/parser.cpp`, `src/parser.h`                 | Token Stream                           | Recursive descent with Pratt operator precedence parsing; enforces top-level import ordering                                      | Abstract Syntax Tree (`Program` AST root)        | Halts with Exit 1 on syntax errors or misplaced imports.                                    |
| **3. Semantic**       | `src/sema.cpp`, `src/sema.h`                     | Raw AST                                | Lexical scoping, type checking, constant immutability checks (`fixed`), generates symbol table                                  | Decorated AST + `LibUse` Side-Table              | Halts with Exit 1 on undeclared variables, type collisions, or illegal reassignments.       |
| **4. Load & Link**    | `src/link_check.cpp`, `src/library_registry.cpp` | `Program.imports` + `LibUse` table | Validates library registry, audits used features against loaded modules, computes Levenshtein distance                            | Verified Module Linkage                            | **Quarantine Halt:** Stops compiler before TAC emission if LV12, LV13, or LV14 occur. |
| **5. TAC Gen**        | `src/tac_gen.cpp`, `src/tac.h`                   | Verified AST                           | Lowers high-level constructs into linearized Three-Address Code quadruples `(op, arg1, arg2, res)`                              | Linearized TAC Instructions                        | N/A (Guaranteed well-formed by semantic checks).                                            |
| **6. CFG Builder**    | `src/cfg.cpp`, `src/cfg.h`                       | Linearized TAC                         | Identifies basic block leaders (labels, targets, branch continuations), constructs directed CFG edges                             | Control Flow Graph (Nodes & Edges)                 | N/A.                                                                                        |
| **7. Logic Verifier** | `src/verifier.cpp`, `src/verifier.h`             | Control Flow Graph                     | Interval abstract interpretation, path-sensitive edge transfer functions, chaotic fixed-point iteration                           | Proved Invariants & Diagnostic Proofs (LV01–LV11) | Halts with Exit 1 on fatal loop overshoots, infinite loops, or definite division-by-zero.   |
| **8. Optimizer**      | `src/optimizer.cpp`, `src/optimizer.h`           | Verified CFG                           | Multi-pass canonical optimizations: Constant folding/propagation, algebraic simplification, dead-code elimination, jump threading | Optimized Canonical TAC                            | Optional `--optcheck` verifies operational equivalence.                                   |
| **9. Runtime VM**     | `src/tac_interp.cpp`, `src/tac_interp.h`         | Optimized TAC + Inputs                 | Stack-frame virtual machine, dispatch loop, standard library intrinsics execution (`show`, `input`)                           | Program stdout & Return Code                       | Clean Exit 0 upon successful completion.                                                    |

---

## 4. The Load & Link Quarantine Subsystem

### The Problem in Traditional Compilers

In standard C/C++ toolchains, the linker is an entirely decoupled binary (`ld` or `lld`) that executes long after intermediate compilation finishes. When missing libraries or misspelled symbols occur, the backend emits unreadable mangled linker errors (e.g. `undefined reference to _Z14calculate_areaPKc`).

### The J-- Native Solution: Integrated Quarantine Gate

J-- features an integrated Load and Link Verifier that sits directly between the Semantic Analyzer and TAC Generation.

```
Decorated AST ────► [LOAD Pass] ────► [LINK Pass] ────► [Levenshtein Matcher] ────► Clean? 
                          │                   │                     │                  │
                      [LV13/LV15]         [LV12/LV16]            [LV14]           YES: Lower to TAC
                          └───────────────────┴─────────────────────┴────────────► NO:  QUARANTINE HALT!
```

### The Registry & Supported Modules

The internal library registry (`src/library_registry.cpp`) manages built-in standard modules:

* `text`: String processing primitives (`word` data type, `concat`, `length`, `contains`, `substr`).
* `math`: Arithmetic & mathematical primitives (`abs`, `sqrt`, `pow`, `min`, `max`).
* `vector`: Linear algebra & container primitives (`list` expansions, `dot_product`, `magnitude`).

### Quarantine Execution Behavior

If a program triggers a fatal linkage error (LV12, LV13, or LV14), the compiler prints the exact diagnostic and immediately aborts:

```
Pipeline stopped before intermediate code.
```

**No Three-Address Code, No Control Flow Graph, and No VM execution will take place.**

---

## 5. The Logic Verifier Engine (LV01–LV16 Rules)

The J-- Logic Verifier is a static formal analysis engine based on **Abstract Interpretation**.

### Mathematical Foundation: The Interval Lattice

Each numeric variable $x$ is mapped to an interval in the complete lattice $\mathcal{D}_{\text{int}}$:

$$
\mathcal{D}_{\text{int}} = \{ [lo, hi] \mid lo, hi \in \mathbb{Z} \cup \{-\infty, +\infty\}, lo \le hi \} \cup \{\bot\}
$$

* Bottom ($\bot$): Unreachable / contradiction state.
* Top ($\top = [-\infty, +\infty]$): Completely unconstrained state.

### Path-Sensitive Edge Refinement

When control branches on a condition $C$, outgoing edges refine variable states:

$$
\sigma_{\text{true}} = \sigma \sqcap \llbracket C \rrbracket \qquad \sigma_{\text{false}} = \sigma \sqcap \llbracket \neg C \rrbracket
$$

If $\sigma_{\text{true}} = \bot$, the True branch is mathematically unreachable, proving **LV02 (Dead Code Branch)**.
If $\sigma_{\text{false}} = \bot$, the False branch is mathematically unreachable, proving **LV01 (Redundant Condition)**.

### Complete Diagnostic Rule Reference

| Code           | Severity        | Name                              | Formal Detection Criterion                                                                                |
| :------------- | :-------------- | :-------------------------------- | :-------------------------------------------------------------------------------------------------------- |
| **LV01** | `Warning`     | Redundant Condition               | $\sigma_{\text{entry}} \sqcap \neg C = \bot$ (Branch evaluates True under all paths).                   |
| **LV02** | `Warning`     | Dead Code Branch                  | $\sigma_{\text{entry}} \sqcap C = \bot$ (Branch body is mathematically unreachable).                    |
| **LV03** | `Warning`     | Fixed Condition in Loop           | Loop condition variable not modified across any edge in loop body.                                        |
| **LV04** | `Warning`     | Condition Contradiction           | Guard condition contradicts an enclosing dominant branch guard.                                           |
| **LV05** | `Warning`     | Loop Invariant Flaw               | Loop bound invariant violated by abnormal internal re-assignment.                                         |
| **LV06** | `Fatal Error` | Definite Infinite Loop            | Loop back-edge interval never intersects loop termination guard.                                          |
| **LV07** | `Fatal Error` | Loop Increment Direction Mismatch | Loop counter increments away from upper bound or decrements away from lower bound.                        |
| **LV08** | `Fatal Error` | Loop Value Overshoot              | Loop step size $\Delta > 1$ skips exact equality termination guard ($i == N$).                        |
| **LV09** | `Fatal Error` | Array Index Out-of-Bounds         | Proved index interval $[lo_i, hi_i] \cap [0, \text{size}-1] = \emptyset$.                               |
| **LV10** | `Fatal Error` | Definite Division by Zero         | Proved divisor interval $D = [0, 0]$.                                                                   |
| **LV11** | `Warning`     | Potential Division by Zero        | Proved divisor interval $0 \in [lo_D, hi_D]$ where $lo_D \ne hi_D$.                                   |
| **LV12** | `Fatal Error` | Unimported Library Feature        | Program uses symbol $S \in \text{Module } M$, but `import M;` is missing.                             |
| **LV13** | `Fatal Error` | Unknown Module Import             | Source specifies `import M;`, but $M \notin \text{Registry}$.                                         |
| **LV14** | `Fatal Error` | Module Typo with Suggestion       | Module $M \notin \text{Registry}$, but $\min_{R \in \text{Registry}} \text{Levenshtein}(M, R) \le 2$. |
| **LV15** | `Warning`     | Duplicate Module Import           | Module $M$ imported two or more times in source header.                                                 |
| **LV16** | `Warning`     | Unused Module Import              | Module $M$ imported, but zero symbols from $M$ are referenced.                                        |

---

## 6. How to Start This Project From 0 (Setup & Build Guide)

Follow this step-by-step guide to clone, compile, and run J-- from scratch on any operating system.

### Prerequisites

* **C++20 Compiler:** GCC 11+, Clang 13+, or MSVC 2022+
* **Build System:** CMake 3.20+
* **Dashboard Runtime (Optional):** Node.js v18+ and npm

---

### Step 1: Clone the Repository

```bash
git clone https://github.com/your-username/jmm-compiler.git
cd jmm-compiler
```

---

### Step 2: Build the Native Compiler Executable

#### On Linux / macOS:

```bash
mkdir -p build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j $(nproc)
```

#### On Windows (PowerShell with MinGW or MSVC):

```powershell
cmake -B build -G "MinGW Makefiles"
cmake --build build
```

This produces the standalone binary:

* Windows: `build/c--.exe` (or `build/jmm.exe`)
* Linux/macOS: `build/c--`

---

### Step 3: Run the CLI Compiler

Verify the compiler build by compiling and running an example:

```bash
# 1. Run static logic verification on a program:
./build/c-- examples/fibonacci.jmm --verify

# 2. View generated Three-Address Code (TAC):
./build/c-- examples/fibonacci.jmm --tac

# 3. View the multi-pass optimized TAC:
./build/c-- examples/fibonacci.jmm --opt

# 4. Execute the program natively on the VM:
./build/c-- examples/fibonacci.jmm --run-tac
```

#### Compiler CLI Flags Reference

```
Usage: c-- <source.jmm> [OPTIONS]

Options:
  --tokens        Print scanned lexical tokens and positions
  --ast           Dump the Abstract Syntax Tree structure
  --sema          Run semantic analysis and print symbol table
  --link          Run the Load & Link Verifier pass
  --libs          Display all registered built-in standard modules
  --tac           Generate and print Three-Address Code IR
  --cfg           Generate Graphviz DOT control flow graph
  --verify        Run the Interval Logic Verifier (LV01-LV16)
  --opt           Print optimized TAC after constant folding & DCE
  --run-tac       Execute the program on the native bytecode VM
  --inputs <v1,v2> Pass comma-separated stdin values to the VM
  --optcheck      Verify behavioral equivalence between raw and optimized IR
  --strict        Treat all verification warnings as fatal errors
```

---

### Step 4: Launch the Live Projector & Interactive Dashboard

The repository includes a modern, GPU-accelerated web dashboard with real-time compilation, CFG rendering, terminal emulation, and categorized test suites.

```bash
# Navigate to dashboard directory
cd dashboard

# Install frontend and backend dependencies
npm install

# Start both backend compiler bridge (Port 4000) and Vite frontend (Port 5173)
npm start
```

Open your browser to:

```
http://localhost:5173
```

* **Full Display Mode:** Press `F11` for clean, distraction-free classroom presentation.
* **Live Inspector:** Edit `.jmm` code in the Monaco editor and watch the CFG, intervals, and diagnostics update instantaneously.

---

## 7. Language Syntax & Grammar Cheatsheet

### Data Types

| J-- Type    | Description                                 | Example                           |
| :---------- | :------------------------------------------ | :-------------------------------- |
| `num`     | 64-bit signed integer                       | `num counter = 0;`              |
| `decimal` | Double-precision floating point             | `decimal ratio = 3.14159;`      |
| `word`    | String literal (requires `import text;`)  | `word greeting = "Hello, J--";` |
| `letter`  | Single Unicode character                    | `letter grade = 'A';`           |
| `boolean` | Boolean truth values (`true` / `false`) | `boolean active = true;`        |
| `list`    | Linear array container                      | `list values;`                  |
| `fixed`   | Immutable constant modifier                 | `fixed num MAX_RETRIES = 5;`    |

### Control Flow & Keywords

* **Conditionals:**
  ```jmm
  check (score >= 90) {
      show("Grade: A");
  } otherwise {
      show("Grade: B");
  }
  ```
* **Loops:**
  ```jmm
  num i = 0;
  until (i >= 10) {
      show(i);
      i = i + 1;
  }
  ```
* **Keywords:** `check`, `otherwise`, `until`, `stop` (`break`), `skip` (`continue`), `give` (`return`).
* **Comparison & Boolean Operators:** `equals` (`==`), `not_equals` (`!=`), `and` (`&&`), `or` (`||`), `not` (`!`), `<`, `<=`, `>`, `>=`.
* **Standard I/O:** `show(...)` (prints expression), `input()` (reads line from stdin).

---

## 8. Comprehensive Test Suite & Sample Programs

Below are complete, copy-paste ready `.jmm` sample programs illustrating every category of compiler diagnostic and valid execution.

### A. Early Frontend Syntax & Semantic Errors

#### Case 1: Lexical Error — Unrecognized Token

```jmm
# File: test_lexical_error.jmm
num valid_var = 10;
num @corrupted = 20;   # '@' is not a valid character in J--
show(valid_var);
```

* **Compiler Diagnostic:** `[LEXICAL ERROR] Line 3, Col 5: Unrecognized character '@'` (Exit code 1).

#### Case 2: Syntax Error — Misplaced Import Inside Routine

```jmm
# File: test_misplaced_import.jmm
num start = 1;
import text;   # Grammatical Invariant: All imports MUST precede code statements
show(start);
```

* **Compiler Diagnostic:** `[SYNTAX ERROR] Line 3: Imports must appear at the top of the file before any statements.` (Exit code 1).

#### Case 3: Semantic Error — Re-assigning a `fixed` Constant

```jmm
# File: test_fixed_reassign.jmm
fixed num TIMEOUT = 30;
TIMEOUT = 60;   # Semantic Invariant: fixed variables are immutable
show(TIMEOUT);
```

* **Compiler Diagnostic:** `[SEMANTIC ERROR] Line 3: Cannot assign to fixed variable 'TIMEOUT'.` (Exit code 1).

---

### B. Load & Link Checks (LV12–LV16)

#### Case LV12: Unimported Library Type (`word`)

```jmm
# File: test_lv12_unimported.jmm
# Missing 'import text;'
word username = "Utkarsh";
show(username);
```

* **Compiler Diagnostic:**
  ```
  [LOAD/LINK ERROR] (LV12) Type 'word' requires library 'text'.
  Pipeline stopped before intermediate code.
  ```

#### Case LV13: Unknown Module Import

```jmm
# File: test_lv13_unknown.jmm
import cryptography;   # 'cryptography' is not in the internal library registry
num x = 42;
show(x);
```

* **Compiler Diagnostic:**
  ```
  [LOAD/LINK ERROR] (LV13) Unknown library 'cryptography'.
  Pipeline stopped before intermediate code.
  ```

#### Case LV14: Module Typo with Levenshtein Suggestion ($D \le 2$)

```jmm
# File: test_lv14_typo.jmm
import mathe;   # Typo for 'math' (Levenshtein Distance = 1)
num r = 5;
show(r);
```

* **Compiler Diagnostic:**
  ```
  [LOAD/LINK ERROR] (LV14) Unknown library 'mathe'. Did you mean 'math'?
  Pipeline stopped before intermediate code.
  ```

#### Case LV15: Duplicate Module Import

```jmm
# File: test_lv15_duplicate.jmm
import math;
import math;   # Redundant second import
num x = 10;
show(x);
```

* **Compiler Diagnostic:** `[LOAD/LINK WARNING] (LV15) Library 'math' is imported multiple times.` (Compilation proceeds).

#### Case LV16: Unused Library Import

```jmm
# File: test_lv16_unused.jmm
import text;   # Imported, but program only uses integers
num a = 50;
num b = 20;
show(a + b);
```

* **Compiler Diagnostic:** `[LOAD/LINK WARNING] (LV16) Library 'text' is imported but never used.` (Compilation proceeds).

---

### C. Logic Verifier Checks (LV01–LV11)

#### Case LV01 & LV02: Redundant Condition and Dead Code Branch

```jmm
# File: test_lv01_lv02.jmm
num x = 10;
check (x > 5) {
    show("x is greater than 5");   # Always executed (LV01)
} otherwise {
    show("Dead branch");           # Mathematically unreachable (LV02)
}
```

* **Compiler Diagnostic:**
  * `[VERIFIER WARNING] (LV01) Condition '(x > 5)' is always TRUE. Variable 'x' has interval [10, 10].`
  * `[VERIFIER WARNING] (LV02) Code block inside 'otherwise' is unreachable (DEAD CODE).`

#### Case LV06: Definite Infinite Loop (Fatal Error)

```jmm
# File: test_lv06_infinite.jmm
num i = 0;
until (i == 10) {
    show(i);
    # Bug: counter 'i' is never incremented!
}
```

* **Compiler Diagnostic:**
  ```
  [VERIFIER ERROR] (LV06) Definite infinite loop detected.
  Loop guard requires 'i == 10', but variable 'i' interval remains [0, 0].
  ```

#### Case LV07: Increment Direction Mismatch

```jmm
# File: test_lv07_direction.jmm
num count = 10;
until (count == 0) {
    show(count);
    count = count + 1;   # Bug: incrementing away from zero!
}
```

* **Compiler Diagnostic:**
  ```
  [VERIFIER ERROR] (LV07) Loop increment direction mismatch.
  Counter 'count' increments away from termination target 0.
  ```

#### Case LV08: Loop Off-by-One / Value Overshoot

```jmm
# File: test_lv08_overshoot.jmm
num i = 0;
until (i == 9) {
    show(i);
    i = i + 2;   # Steps: 0, 2, 4, 6, 8, 10... skips 9!
}
```

* **Compiler Diagnostic:**
  ```
  [VERIFIER ERROR] (LV08) Loop step size 2 skips exact equality guard 'i == 9'.
  Loop will overshoot and fail to terminate.
  ```

#### Case LV10: Definite Division by Zero (Fatal Error)

```jmm
# File: test_lv10_div_zero.jmm
num numerator = 100;
num divisor = 0;
num result = numerator / divisor;   # Definite zero divisor
show(result);
```

* **Compiler Diagnostic:**
  ```
  [VERIFIER ERROR] (LV10) Definite division by zero detected.
  Divisor 'divisor' has proved interval [0, 0]. Halting compilation.
  ```

#### Case LV11: Potential Division by Zero (Path-Sensitive Warning)

```jmm
# File: test_lv11_potential_div.jmm
num denom = input();   # Unconstrained interval [-inf, +inf]
num answer = 500 / denom;
show(answer);
```

* **Compiler Diagnostic:**
  ```
  [VERIFIER WARNING] (LV11) Potential division by zero.
  Divisor 'denom' interval [-∞, +∞] contains 0 without guarding check.
  ```

---

### D. Guarded Code (Zero False Positives)

The J-- Verifier features path-sensitive edge refinement. When code is correctly guarded by a condition, the verifier proves safety and issues zero false alarms:

```jmm
# File: test_guarded_safe.jmm
num denom = input();

check (denom not_equals 0) {
    # Refinement on TRUE branch: interval of 'denom' becomes (-inf, -1] U [1, +inf]
    num safe_quotient = 1000 / denom;
    show("Safe division result: ");
    show(safe_quotient);
} otherwise {
    show("Error: Divisor cannot be zero.");
}
```

* **Compiler Diagnostic:** `[VERIFIER] 0 Errors, 0 Warnings. Verification Proof Succeeded.`
* **VM Execution:** Safe execution guaranteed.

---

### E. Valid Showcase Applications

#### 1. Fibonacci Sequence Generator

```jmm
# File: examples/fibonacci.jmm
num n = 8;
num first = 0;
num second = 1;
num count = 0;

show("--- Fibonacci Sequence ---");
until (count == n) {
    show(first);
    num next = first + second;
    first = second;
    second = next;
    count = count + 1;
}
```

* **Execution Output:**
  ```
  --- Fibonacci Sequence ---
  0
  1
  1
  2
  3
  5
  8
  13
  ```

#### 2. Standard Library Integration (`text` & `math`)

```jmm
# File: examples/stdlib_demo.jmm
import text;
import math;

word user = "Ishan";
num radius = 7;
num area = (22 * radius * radius) / 7;

show("Welcome developer: ");
show(user);
show("Computed circle area: ");
show(area);
```

* **Execution Output:**
  ```
  Welcome developer: 
  Ishan
  Computed circle area: 
  154
  ```

---

## 🏆 Project Achievements & Summary

1. **Own Language Design:** Created a full-fledged imperative language (J--) with dedicated grammar, lexer, and parser.
2. **Intermediate Representation:** Built a linearized Three-Address Code (TAC) virtual register quadruple system.
3. **Formal Verification:** Implemented an interval-domain Abstract Interpreter catching 16 distinct categories of logic and linkage flaws (LV01–LV16).
4. **Intermediate Code Quarantine:** Engineered a strict Load & Link quarantine gate preventing broken code from reaching IR lowering.
5. **Multi-Pass Optimizer:** Built an AST and TAC canonical optimizer performing constant folding, algebraic reduction, jump threading, and dead code elimination.
6. **Hardware-Accelerated UI:** Created a live web projector dashboard with Monaco editor integration, dynamic CFG graphing, and instant compilation feedback.

---

*Designed and engineered with pride by **Utkarsh Vikram Singh** (`24BCE2611`) & **Ishan Awasthi** (`24BAI0207`).*
