# Custom Programming Language — Consolidated Keyword Specification

Update the existing compiler to support the following custom language keywords and operators. First inspect the current codebase, identify the lexer, parser, AST, and any existing semantic analysis or code generation, then make the necessary changes while preserving working functionality.

## 1. Data Types and Declarations

| C Equivalent       | Custom Keyword |
| ------------------ | -------------- |
| `int`              | `num`          |
| `float` / `double` | `decimal`      |
| `string`           | `word`         |
| `char`             | `letter`       |
| `bool`             | `boolean`      |
| Array type         | `list`         |

The language must use **explicit static typing**. Variables must have declared types; do not infer their types from assigned values.

Examples:

- `num age = 20;`
- `decimal price = 19.99;`
- `word name = "Alex";`
- `letter grade = 'A';`
- `boolean valid = true;`
- `list marks;`

## 2. Control Flow

| C Equivalent | Custom Keyword |
| ------------ | -------------- |
| `if`         | `check`        |
| `else`       | `otherwise`    |
| `while`      | until          |
| `for`        | foreach        |
| `do...while` | `do_until`     |
| `break`      | `stop`         |
| `continue`   | `skip`         |

Use these keywords as replacements for their corresponding C constructs. Preserve conventional parentheses and braces unless the existing language specification requires otherwise.

Examples:

- `check (age > 18) { ... }`
- `otherwise { ... }`
- `until (age < 30) { ... }`

## 3. Input and Output

| C Equivalent | Custom Keyword |
| ------------ | -------------- |
| `printf()`   | `show()`       |
| `scanf()`    | `input()`      |
| `return`     | `give`         |

Treat `show()` and `input()` as built-in operations in the language. Do not require C-style format specifiers such as `%d` or `%s`.

Examples:

- `show(age);`
- `word name = input();`
- `give 0;`

## 4. Operators

Preserve conventional arithmetic, assignment, comparison, and increment/decrement operators. Replace C's logical operators with the custom word-based equivalents.

| Purpose                  | Operator  |
| ------------------------ | --------- |
| Addition                 | `+`       |
| Subtraction              | `-`       |
| Multiplication           | `*`       |
| Division                 | `/`       |
| Modulus                  | `%`       |
| Assignment               | `=`       |
| Equal to                 | equals    |
| Not equal                | notequals |
| Greater than             | `>`       |
| Less than                | `<`       |
| Greater than or equal to | `>=`      |
| Less than or equal to    | `<=`      |
| Logical AND              | `and`     |
| Logical OR               | `or`      |
| Logical NOT              | `not`     |
| Increment                | ++        |
| Decrement                | --        |

The lexer must recognize multi-character operators correctly, such as `==`, `!=`, `>=`, `<=`, `++`, and `--`, before their single-character counterparts where applicable.

## 5. Boolean Values and Constants

| C Equivalent | Custom Keyword |
| ------------ | -------------- |
| `true`       | `true`         |
| `false`      | `false`        |
| `const`      | `fixed`        |

`fixed` is intended to declare a constant whose value cannot be reassigned. Implement it only if it can be integrated cleanly into the existing compiler.

## 6. Syntax and Symbols

Keep these conventional symbols:

- / — statement terminator
- `,` — separator
- `()` — expressions, conditions, and arguments
- `{}` — statement blocks
- `[]` — list declarations and indexing
- # # -single-line comments
- `$ ... $` — multi-line comments

## 7. Implementation Requirements

1. Inspect the existing implementation before modifying files.
2. Update the lexer to recognize the custom keywords and logical operators.
3. Update the parser and AST to support the corresponding syntax.
4. Ensure the semantic analyzer enforces explicit typing, valid declarations, compatible assignments, and valid conditions.
5. Update intermediate code generation where supported features require changes.
6. Preserve existing working features and avoid unnecessary rewrites.
7. Add tests for valid programs, invalid keywords, syntax errors, undeclared variables, and type mismatches.
8. Do not claim a feature is implemented unless it passes tests.
9. If a listed feature requires substantial new work, identify it separately rather than silently introducing incomplete support.
10. After implementation, summarize changed files, implemented features, unsupported features, test results, and the next recommended steps.

**Priority:** Maintain a small, understandable compiler. Fully working lexer, parser, semantic analysis, and three-address code generation are more important than implementing every possible language feature immediately.
