import fs from 'node:fs';
import path from 'node:path';

const casesDir = path.resolve('cases');
fs.mkdirSync(casesDir, { recursive: true });

const cases = [
  {
    file: 'err_missing_brace.cmm',
    content: `# Syntax error: missing closing brace
check (true) {
    show(1);
`
  },
  {
    file: 'sema_undeclared.cmm',
    content: `# Semantic error: undeclared variable
num x = y + 1;
show(x);
`
  },
  {
    file: 'sema_type_mismatch.cmm',
    content: `# Semantic error: type mismatch
import text;
num age = "twenty";
show(age);
`
  },
  {
    file: 'lv01_contradiction.cmm',
    content: `# LV01: Always-false condition (contradiction)
num x = input();
check (x > 10 and x < 5) {
    show(1);
}
`
  },
  {
    file: 'lv02_tautology.cmm',
    content: `# LV02: Always-true condition (tautology)
num x = 5;
check (x equals 5) {
    show(1);
} otherwise {
    show(2);
}
`
  },
  {
    file: 'lv03_self.cmm',
    content: `# LV03: Self-comparison
num x = input();
check (x equals x) {
    show(1);
}
`
  },
  {
    file: 'lv04_redundant.cmm',
    content: `# LV04: Redundant condition part
num x = input();
check (x > 10 and x > 5) {
    show(1);
}
`
  },
  {
    file: 'lv05_loop_never_runs.cmm',
    content: `# LV05: Loop body never runs
num i = 10;
until (i equals 10) {
    show(i);
    i++;
}
`
  },
  {
    file: 'lv06_loop_const.cmm',
    content: `# LV06: Loop never terminates
until (false) {
    show(1);
}
`
  },
  {
    file: 'lv07_loop_overshoot.cmm',
    content: `# LV07: Step skips the loop exit
num i = 0;
until (i equals 5) {
    show(i);
    i = i + 2;
}
`
  },
  {
    file: 'lv08_loop_direction.cmm',
    content: `# LV08: Loop moves in wrong direction
num x = 1;
until (x equals 10) {
    x--;
}
`
  },
  {
    file: 'lv09_after_loop.cmm',
    content: `# LV09: Code after an endless loop
until (false) {
    show(1);
}
show(999);
`
  },
  {
    file: 'lv10_div_zero.cmm',
    content: `# LV10: Definite division by zero
num a = 10;
num b = 0;
num c = a / b;
show(c);
`
  },
  {
    file: 'lv11_div_input.cmm',
    content: `# LV11: Possible division by zero on unvalidated input
num divisor = input();
num result = 100 / divisor;
show(result);
`
  },
  {
    file: 'lv_div_guarded.cmm',
    content: `# No false alarm: guarded division
num n = input();
check (n > 0) {
    num r = 100 / n;
    show(r);
}
`
  },
  {
    file: 'lv_loop_stop_ok.cmm',
    content: `# No false alarm: loop with reachable stop
num x = 0;
until (false) {
    x++;
    check (x > 5) {
        stop;
    }
}
show(x);
`
  },
  {
    file: 'lv12_missing_word.cmm',
    content: `# LV12: Missing import for type 'word'
word name = "tester";
show(name);
`
  },
  {
    file: 'lv12_missing_func.cmm',
    content: `# LV12: Missing import for function 'maximum'
num x = maximum(10, 20);
show(x);
`
  },
  {
    file: 'lv13_unknown_lib.cmm',
    content: `# LV13: Unknown library
import banana;
show(1);
`
  },
  {
    file: 'lv14_undefined_func.cmm',
    content: `# LV14: Undefined function in library
import text;
word s = "test";
num l = lengt(s);
show(l);
`
  },
  {
    file: 'lv15_duplicate_import.cmm',
    content: `# LV15: Duplicate import
import text;
import text;
word w = "hello";
show(w);
`
  },
  {
    file: 'lv16_unused_import.cmm',
    content: `# LV16: Unused import
import math;
num x = 42;
show(x);
`
  },
  {
    file: 'link_blocks_logic.cmm',
    content: `# Link error blocks logic verifier from running
word w = "unimported";
num z = 10 / 0;
show(z);
`
  },
  {
    file: 'valid_countdown.cmm',
    content: `# Valid program: countdown loop
num count = 5;
until (count equals 0) {
    show(count);
    count--;
}
show(0);
`
  },
  {
    file: 'valid_import_text.cmm',
    content: `# Valid program: uses imported libraries text and math
import text;
import math;
word message = "Compiler";
show(length(message));
show(absolute(-99));
`
  },
  {
    file: 'valid_input.cmm',
    content: `# Valid program: takes input and processes it safely
import text;
word name = input();
num age;
show(name);
show(age);
`
  }
];

for (const c of cases) {
  const filePath = path.join(casesDir, c.file);
  fs.writeFileSync(filePath, c.content, 'utf8');
}
console.log(`Updated ${cases.length} case files.`);

