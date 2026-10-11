# 📘 C-- (CMM) Language Reference & Developer Guide

Welcome to **C--** (file extension `.cmm`), a statically-typed programming language designed with clean word-based syntax, custom operators, and explicit type safety.

---

## 1. Language Overview & Design Principles

* **File Extension**: `.cmm`
* **Paradigm**: Imperative, Statically-Typed
* **Compiler Frontend**: Written in C++20 with a hand-written Lexer and Recursive-Descent Parser.
* **Statement Terminators**: Semicolon (`;`) or **Newline** (blank lines are automatically collapsed).
* **Comments**:
  * Single-line: `# Single-line comment here`
  * Multi-line: `$ Multi-line comment here $`

---

## 2. Complete Keyword & Type Reference

### Data Types
| Keyword | C Equivalent | Example | Description |
| :--- | :--- | :--- | :--- |
| `num` | `int` | `num score = 100;` | Whole integer numbers |
| `decimal` | `float` / `double` | `decimal pi = 3.1415;` | Floating-point decimal numbers |
| `word` | `string` / `char*` | `word name = "Alex";` | Text strings enclosed in double quotes `"` |
| `letter` | `char` | `letter grade = 'A';` | Single characters enclosed in single quotes `'` |
| `boolean` | `bool` | `boolean active = true;` | Boolean values: `true` or `false` |
| `list` | array | `list marks;` | Array/list type declaration |

### Modifiers & Constants
| Keyword | Meaning | Example |
| :--- | :--- | :--- |
| `fixed` | Constant (`const`), value cannot be reassigned | `fixed num MAX_USERS = 500;` |

### Control Flow
| Keyword | C Equivalent | Syntax Example | Notes |
| :--- | :--- | :--- | :--- |
| `check` | `if` | `check (score >= 50) { ... }` | Parentheses are optional: `check score >= 50 { ... }` |
| `otherwise` | `else` | `otherwise { ... }` | Attaches to previous `check` block |
| `until` | `while` | `until (lives equals 0) { ... }` | Executes block until condition becomes true |
| `stop` | `break` | `stop;` | Exits the innermost loop |
| `skip` | `continue` | `skip;` | Skips to the next loop iteration |

### Input & Output
| Keyword | C Equivalent | Syntax Example | Description |
| :--- | :--- | :--- | :--- |
| `show(...)` | `printf(...)` / `cout` | `show("Hello");` or `show(score);` | Prints expression to standard output |
| `input()` | `scanf(...)` / `cin` | `word username = input();` | Reads input from standard input |
| `give` | `return` | `give 0;` | Exits / returns from current routine |

---

## 3. Operator Reference

| Operator | Purpose | Example |
| :--- | :--- | :--- |
| `+` | Addition | `num a = 5 + 3;` |
| `-` | Subtraction / Unary Negation | `num b = -10;` |
| `*` | Multiplication | `num c = a * 2;` |
| `/` | Division | `num d = c / 4;` |
| `%` | Modulo / Remainder | `num rem = a % 2;` |
| `=` | Assignment | `a = 20;` |
| `++` | Post-increment by 1 | `a++;` |
| `--` | Post-decrement by 1 | `a--;` |
| `equals` (or `==`) | Equality test | `check (a equals 10) { ... }` |
| `notequals` (or `!=`) | Inequality test | `check (a notequals 10) { ... }` |
| `<` | Less than | `check (a < 10) { ... }` |
| `>` | Greater than | `check (a > 10) { ... }` |
| `<=` | Less than or equal to | `check (a <= 10) { ... }` |
| `>=` | Greater than or equal to | `check (a >= 10) { ... }` |
| `and` | Logical AND (`&&`) | `check (a > 0 and b > 0) { ... }` |
| `or` | Logical OR (`\|\|`) | `check (a > 0 or b > 0) { ... }` |
| `not` | Logical NOT (`!`) | `check (not isFinished) { ... }` |

---

## 4. Code Examples

### Example 1: `game_combat.cmm`
```cmm
# Game Combat & Shield Simulation
fixed word TITLE = "Space Arena";
word player = "Aria";
num level = 5;
decimal health = 100.0;
boolean hasShield = true;

show("--- Welcome to: ---");
show(TITLE);

# Combat calculation
num enemyDamage = 35;

check (hasShield and health > 50.0) {
    show("Shield absorbed the damage!");
    health = health - 10.0;
} otherwise {
    show("Direct hit taken!");
    health = health - enemyDamage;
}

show("Remaining Health:");
show(health);

give 0;
```

---

### Example 2: `countdown.cmm`
```cmm
# Rocket Launch Loop
num countdown = 5;
fixed num LIFTOFF = 0;

show("Initiating launch sequence...");

until (countdown equals LIFTOFF) {
    show(countdown);
    countdown--;
}

show("Liftoff!");
```

---

### Example 3: `student_grades.cmm`
```cmm
# Grade Evaluation Matrix
letter grade = 'B';
num attendance = 90;
boolean honors = false;

check ((grade equals 'A' or grade equals 'B') and not (attendance < 75)) {
    honors = true;
    show("Awarding Honors Citation!");
} otherwise {
    show("Requirements not fulfilled.");
}
```

---

## 5. Using the Compiler CLI

To compile, inspect, and test your C-- programs:

```powershell
# Inspect the token stream (Lexer debug output)
.\build\c--.exe myfile.cmm --tokens

# Inspect the Abstract Syntax Tree (Parser debug output)
.\build\c--.exe myfile.cmm --ast

# Check for syntax errors without printing the AST
.\build\c--.exe myfile.cmm

# Run the complete test suite
.\build\cmm_test.exe
```

