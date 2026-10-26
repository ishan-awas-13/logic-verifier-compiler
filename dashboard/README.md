# C-- Logic Verifier Projector Demonstration Dashboard

A local, high-contrast, strictly monochrome web dashboard designed specifically to demonstrate the **C-- Compiler & Logic Verifier** pipeline on a projector.

The dashboard runs 100% offline, operates without external CDNs or remote APIs, and communicates with the C-- compiler strictly through its command-line interface.

---

## 1. Quick Start

### Option A: Single Command Start
```bash
npm run start
```
Starts the Node.js server binding to `127.0.0.1:4000` and serves the production-bundled frontend.

### Option B: Windows One-Click Batch Script
Double-click `start.bat` (or execute from PowerShell/CMD):
```cmd
start.bat
```
Checks dependencies, builds the frontend if needed, launches the server, and automatically opens your browser at `http://127.0.0.1:4000`.

### Option C: Development Mode
```bash
npm run dev
```
Starts the Vite development server with hot-module reloading on `http://localhost:3000` (proxies `/api` requests to backend at port 4000).

---

## 2. Step 0: Compiler Inspection Findings

| Stage / Concern | CLI Flag(s) | Output / Behavior | Status |
| :--- | :--- | :--- | :--- |
| **Lexer** | `--tokens` | Formatted table: `Line`, `Col`, `Type`, `Lexeme` | Supported |
| **Parser** | `--ast` | Hierarchical syntax tree printout | Supported |
| **Semantic Analysis** | `--sema` | Type-checks program, outputs Symbol Table | Supported |
| **Load and Link** | `--link` | Verifies library imports (`text`, `vector`, `math`), outputs link map table | Supported |
| **Library Registry** | `--libs` | Dumps registered built-in libraries, types, and function signatures | Supported |
| **TAC Generation** | `--tac` | Emits Three-Address Code quadruples | Supported |
| **Control Flow Graph** | `--cfg`, `--dot` | Emits basic blocks with predecessors, successors, and DOT graph | Supported |
| **Logic Verifier** | `--verify` | Runs abstract interpretation on CFG, emits `LV01`–`LV16` findings | Supported |
| **Strict Verifier** | `--strict` | Escalates Logic Verifier warnings into fatal errors (exit code 1) | Supported |
| **Optimizer** | `--opt`, `--stats` | Multi-pass IR optimizer (constant folding, dead code elimination, etc.) | Supported |
| **Equivalence Check** | `--optcheck` | Proves unoptimized and optimized TAC produce identical character-by-character output | Supported |
| **Execution Backend** | `--run-tac` | Interprets TAC quadruples via TAC interpreter | Supported |
| **Runtime Inputs** | `--inputs <csv>` | Supplies comma-separated values to `input()` calls | Supported |

---

## 3. Configuration: `dashboard.config.json`

The dashboard behavior is entirely config-driven:

```jsonc
{
  // Display name of the language shown in headers
  "languageName": "C--",

  // Source code file extension
  "fileExtension": ".cmm",

  // Path to the compiler binary relative to the dashboard directory
  // Overridable via the COMPILER_PATH environment variable
  "compilerPath": "../C--/build/c--.exe",

  // Process execution timeout in milliseconds (kills hung processes)
  "timeoutMs": 10000,

  // Full sequential pipeline phases executed by /api/pipeline
  "pipeline": [
    { "id": "lexer", "label": "Lexer", "flags": ["--tokens"] },
    { "id": "parser", "label": "Parser", "flags": ["--ast"] },
    { "id": "semantic", "label": "Semantic", "flags": ["--sema"] },
    { "id": "link", "label": "Load and Link", "flags": ["--link"] },
    { "id": "logic", "label": "Logic Verifier", "flags": ["--verify"] },
    { "id": "tac", "label": "TAC Gen", "flags": ["--tac"] },
    { "id": "cfg", "label": "CFG", "flags": ["--cfg"] },
    { "id": "optimizer", "label": "Optimizer", "flags": ["--opt", "--stats"] },
    { "id": "run", "label": "Run", "flags": ["--run-tac"] }
  ],

  // Direct quick-execution modes available in the dashboard dropdown
  "quickModes": [
    { "id": "verify", "label": "Verify", "flags": ["--verify"] },
    { "id": "verify-strict", "label": "Verify (Strict)", "flags": ["--verify", "--strict"] },
    { "id": "link-only", "label": "Link Map", "flags": ["--link"] },
    { "id": "run", "label": "Run TAC", "flags": ["--run-tac"] },
    { "id": "optcheck", "label": "Optimizer Equivalence", "flags": ["--optcheck"] },
    { "id": "libs", "label": "Library Registry", "flags": ["--libs"] },
    { "id": "ast", "label": "AST Tree", "flags": ["--ast"] },
    { "id": "tokens", "label": "Tokens", "flags": ["--tokens"] },
    { "id": "tac", "label": "TAC IR", "flags": ["--tac"] },
    { "id": "cfg", "label": "CFG Blocks", "flags": ["--cfg"] }
  ]
}
```

---

## 4. Verification and Testing

### Running the Case Verification Suite (`npm run verify-cases`)
Executes all 26 cases in `cases/manifest.json` against the real `c--.exe` compiler and verifies that exit codes and reported logic verifier codes (`[LVnn]`) match 100%:

```bash
npm run verify-cases
```

Sample output:
```
Case ID                   | Expected LV    | Actual LV      | Exp Exit | Act Exit | Status   | Duration  
---------------------------------------------------------------------------------------------------------
err_missing_brace         | none           | none           | 1        | 1        | PASS     | 55ms      
sema_undeclared           | none           | none           | 1        | 1        | PASS     | 55ms      
sema_type_mismatch        | none           | none           | 1        | 1        | PASS     | 56ms      
lv01_contradiction        | LV01           | LV01           | 0        | 0        | PASS     | 52ms      
lv02_tautology            | LV02           | LV02           | 0        | 0        | PASS     | 45ms      
lv03_self                 | LV03           | LV03           | 0        | 0        | PASS     | 43ms      
lv04_redundant            | LV04           | LV04           | 0        | 0        | PASS     | 61ms      
lv05_loop_never_runs      | LV05           | LV05           | 0        | 0        | PASS     | 61ms      
lv06_loop_const           | LV06           | LV06           | 1        | 1        | PASS     | 54ms      
lv07_loop_overshoot       | LV07           | LV07           | 1        | 1        | PASS     | 49ms      
lv08_loop_direction       | LV08           | LV08           | 1        | 1        | PASS     | 60ms      
lv09_after_loop           | LV06,LV09      | LV06,LV09      | 1        | 1        | PASS     | 71ms      
lv10_div_zero             | LV10           | LV10           | 1        | 1        | PASS     | 77ms      
lv11_div_input            | LV11           | LV11           | 0        | 0        | PASS     | 74ms      
lv_div_guarded            | none           | none           | 0        | 0        | PASS     | 70ms      
lv_loop_stop_ok           | none           | none           | 0        | 0        | PASS     | 69ms      
lv12_missing_word         | LV12           | LV12           | 1        | 1        | PASS     | 57ms      
lv12_missing_func         | LV12           | LV12           | 1        | 1        | PASS     | 62ms      
lv13_unknown_lib          | LV13           | LV13           | 1        | 1        | PASS     | 70ms      
lv14_undefined_func       | LV14           | LV14           | 1        | 1        | PASS     | 62ms      
lv15_duplicate_import     | LV15           | LV15           | 0        | 0        | PASS     | 62ms      
lv16_unused_import        | LV16           | LV16           | 0        | 0        | PASS     | 61ms      
link_blocks_logic         | LV12           | LV12           | 1        | 1        | PASS     | 58ms      
valid_countdown           | none           | none           | 0        | 0        | PASS     | 61ms      
valid_import_text         | none           | none           | 0        | 0        | PASS     | 103ms     
valid_input               | none           | none           | 0        | 0        | PASS     | 113ms     
---------------------------------------------------------------------------------------------------------
Total cases: 26 | Passed: 26 | Failed: 0
```

### Running Backend Integration Tests (`npm test`)
```bash
npm test
```
Runs 7 automated integration tests with `node:test`:
- `GET /api/health`: Verifies compiler detection and phase registration.
- `GET /api/cases`: Confirms all 26 cases load with source code.
- `POST /api/pipeline` (valid): Confirms all 9 phases pass sequentially.
- `POST /api/pipeline` (LV10): Proves pipeline stops at logic verifier and flags later phases as `not_reached`.
- `POST /api/pipeline` (syntax): Proves parsing errors halt subsequent phases.
- `POST /api/run` (oversized source): Validates rejection of requests > 200 KB.
- Timeout handling: Validates automatic SIGKILL termination and `timedOut: true` flag.

---

## 5. Adding a New Case in One Step

1. Add your `.cmm` program file to `cases/` (e.g. `cases/my_new_case.cmm`).
2. Add an entry to `cases/manifest.json`:
```json
{
  "id": "my_new_case",
  "category": "Logic Flaws",
  "title": "Custom Test Case Title",
  "code": "LV01",
  "description": "Short explanation of the flaw and why it matters.",
  "file": "cases/my_new_case.cmm",
  "expectedCodes": ["LV01"],
  "expectedExit": 0,
  "inputs": "optional\nstdin\ninputs"
}
```
3. Run `npm run verify-cases` to verify with the compiler!

---

## 6. Projector & Examiner Presentation Features

1. **Strictly Monochrome Design:**
   - Pure black `#000000` background and high-contrast white `#FFFFFF` text.
   - Zero color confusion on miscalibrated projectors.
   - Severity is conveyed purely through shapes:
     - `■ ERROR`: Solid white pill with black text and filled square.
     - `△ WARNING`: Outlined white pill with white text and outlined triangle.
     - `✓ MATCH`: Outlined pill with checkmark.
2. **Typography Hierarchy:**
   - Headers: Cursive script (`Great Vibes`, 30px+) for elegance.
   - UI Labels & Tabs: All-caps sans-serif (`Inter`, letter-spaced).
   - Code & Terminal Output: Clean monospace (`JetBrains Mono`).
   - Fonts are 100% bundled locally through `@fontsource` packages.
3. **Presentation Mode (Toggle with `F11` or button):**
   - Collapses the left rail and expands editor and output to maximum projector dimensions.
   - Quick font scaling with `[A-]` and `[A+]` controls.
4. **Interactive Diagnostic Jumping:**
   - Clicking any diagnostic card instantly scrolls the CodeMirror editor to that line and column and flashes the line with high-contrast animation.
5. **Keyboard Shortcuts:**
   - `Ctrl+Enter`: Run entire compiler pipeline.
   - `Ctrl+S`: Save edits to browser `localStorage`.
   - `Ctrl+/`: Toggle left case library rail.
   - `Esc`: Exit presentation mode or close verification matrix modal.

---

## 7. Manual Test Checklist

- [x] **Category Walkthrough:** Click at least one case in each category (Earlier Phases, Logic Flaws, Load and Link, Valid Programs) and click `Run pipeline`.
- [x] **Diagnostic Jump:** In `lv10_div_zero`, click the diagnostic card; verify line 3 flashes in the editor.
- [x] **Presentation Mode:** Click `PRESENT` or press `F11`; verify rail hides, editor fills screen. Press `Esc` to exit.
- [x] **Pre-Demo Verification Matrix:** Click `ALL CASES` in header; click `EXECUTE ALL CASES`; verify all 26 cases show `MATCH ✓`.
- [x] **Offline Verification:** Disconnect Wi-Fi / network; reload page; verify all fonts render properly and all compiler runs function normally.
- [x] **Responsive Scaling:** Resize window to 1366x768; verify editor, pipeline strip, and output tabs remain fully readable and operable.

