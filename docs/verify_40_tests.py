import subprocess
import tempfile
import os

compiler = r"C:\Users\thaku\.gemini\antigravity\scratch\C--\build\c--.exe"

tests = [
    # Category 1: Syntax
    ("syn01", "num x = 50;\ncheck (x > 20) {\nshow(x);\nnum y = 10;\nshow(y);\n", 1, "Missing brace"),
    ("syn02", "num a = 10;\nshow(a);\nimport text;\nword s = \"hello\";\nshow(s);\n", 1, "Misplaced import"),
    ("syn03", "num p = 25;\nnum q = p ++* 5;\nshow(q);\n", 1, "Malformed operator"),
    ("syn04", "import ;\nnum z = 42;\nshow(z);\n", 1, "Missing import name"),
    ("syn05", "num price = 100;\nnum tax = ;\nshow(price);\n", 1, "Missing expr after ="),
    
    # Category 2: Semantic
    ("sem01", "num total = 100;\nnum finalScore = total + uncalculatedBonus;\nshow(finalScore);\n", 1, "Undeclared var"),
    ("sem02", "decimal pi = 3.14159;\nnum count = pi;\nshow(count);\n", 1, "Type mismatch num vs decimal"),
    ("sem03", "fixed num MAX_USERS = 500;\nshow(MAX_USERS);\nMAX_USERS = 600;\nshow(MAX_USERS);\n", 1, "Fixed reassignment"),
    ("sem04", "num base = 40;\nboolean flag = true;\nnum result = base + flag;\nshow(result);\n", 1, "Type violation"),
    ("sem05", "check (true) {\nnum secretToken = 999;\nshow(secretToken);\n}\nshow(secretToken);\n", 1, "Scope leak"),
    
    # Category 3: Conditions
    ("lv01", "num val = 15;\ncheck (val > 50 and val < 20) {\nshow(1);\n}\nshow(val);\n", 0, "LV01 contradiction"),
    ("lv02", "num score = 85;\ncheck (score >= 0 or score < 100) {\nshow(score);\n}\nshow(0);\n", 0, "LV02 tautology"),
    ("lv03", "num temperature = 32;\ncheck (temperature equals temperature) {\nshow(1);\n}\nshow(temperature);\n", 0, "LV03 self-cmp"),
    ("lv04", "num age = 25;\ncheck (age > 20 and age > 10) {\nshow(age);\n}\nshow(1);\n", 0, "LV04 redundant"),
    ("lv01_cascade", "num level = 5;\ncheck (level equals 10 and level equals 20) {\nshow(999);\n}\notherwise {\nshow(level);\n}\n", 0, "LV01 cascade"),
    
    # Category 4: Loops
    ("lv05", "num counter = 0;\nuntil (counter equals 0) {\nshow(counter);\ncounter++;\n}\nshow(100);\n", 0, "LV05 never runs"),
    ("lv06", "num idx = 0;\nuntil (idx equals 10) {\nshow(idx);\n}\nshow(1);\n", 1, "LV06 infinite loop"),
    ("lv07", "num i = 0;\nuntil (i equals 7) {\nshow(i);\ni = i + 2;\n}\nshow(i);\n", 1, "LV07 overshoot"),
    ("lv08", "num x = 0;\nuntil (x equals 10) {\nshow(x);\nx--;\n}\nshow(x);\n", 1, "LV08 wrong direction"),
    ("lv09", "num k = 1;\nuntil (k equals 0) {\nshow(k);\nk++;\n}\nshow(999);\n", 1, "LV09 after endless"),
    
    # Category 5: Arithmetic
    ("lv10_const", "num numerator = 500;\nnum denominator = 0;\nnum quotient = numerator / denominator;\nshow(quotient);\n", 1, "LV10 const div-0"),
    ("lv10_expr", "num a = 12;\nnum b = 12;\nnum diff = a - b;\nnum result = 100 / diff;\nshow(result);\n", 1, "LV10 expr div-0"),
    ("lv11_input", "num divisor = input();\nnum ratio = 1000 / divisor;\nshow(ratio);\n", 0, "LV11 input div-0"),
    ("lv10_calc", "num x = 5;\nnum y = 5;\nnum z = 50 / (x - y);\nshow(z);\n", 1, "LV10 computed div-0"),
    ("lv11_mod", "num modulus = input();\nnum rem = 45 % modulus;\nshow(rem);\n", 0, "LV11 input mod-0"),
    
    # Category 6: Guarded Safety
    ("grd01", "num divisor = input();\ncheck (divisor != 0) {\nnum safeQuotient = 1000 / divisor;\nshow(safeQuotient);\n}\notherwise {\nshow(0);\n}\n", 0, "Guarded div != 0"),
    ("grd02", "num den = input();\ncheck (den > 0) {\nnum safeVal = 500 / den;\nshow(safeVal);\n}\nshow(1);\n", 0, "Guarded div > 0"),
    ("grd03", "num i = 1;\nuntil (i equals 9) {\nshow(i);\ni = i + 2;\n}\nshow(i);\n", 0, "Aligned step loop"),
    ("grd04", "num count = 10;\nuntil (count equals 0) {\nshow(count);\ncount--;\n}\nshow(0);\n", 0, "Countdown loop"),
    ("grd05", "num selector = input();\nnum offset = 0;\ncheck (selector > 10) {\noffset = 5;\n}\notherwise {\noffset = 2;\n}\nnum outcome = 20 / offset;\nshow(outcome);\n", 0, "Guarded branch join"),
    
    # Category 7: Load/Link
    ("lv12_word", "word greeting = \"hello world\";\nshow(greeting);\n", 1, "LV12 missing word"),
    ("lv12_func", "num neg = -45;\nnum pos = absolute(neg);\nshow(pos);\n", 1, "LV12 missing func"),
    ("lv13_unknown", "import graphics;\nnum x = 10;\nshow(x);\n", 1, "LV13 unknown lib"),
    ("lv14_typo", "import text;\nword str = \"analyzer\";\nnum len = lenght(str);\nshow(len);\n", 1, "LV14 typo"),
    ("lv15_16_dup", "import text;\nimport text;\nimport math;\nword msg = \"systems\";\nshow(msg);\n", 0, "LV15 & LV16"),
    
    # Category 8: Valid
    ("val01", "import text;\nword title = \"LogicVerifier\";\nnum len = length(title);\nboolean empty = isempty(title);\nshow(len);\nshow(empty);\n", 0, "Valid text stdlib"),
    ("val02", "import math;\nnum a = -75;\nnum b = 120;\nnum absA = absolute(a);\nnum maxVal = maximum(absA, b);\nnum minVal = minimum(absA, b);\nshow(absA);\nshow(maxVal);\nshow(minVal);\n", 0, "Valid math stdlib"),
    ("val03", "num celsius = 30;\nnum fahrenheit = (celsius * 9) / 5 + 32;\nshow(fahrenheit);\ncheck (fahrenheit > 80) {\nshow(1);\n}\notherwise {\nshow(0);\n}\n", 0, "Valid temp conversion"),
    ("val04", "num n = 7;\nnum prev1 = 1;\nnum prev2 = 0;\nnum current = 0;\nnum step = 1;\nuntil (step equals n) {\ncurrent = prev1 + prev2;\nprev2 = prev1;\nprev1 = current;\nstep++;\n}\nshow(current);\n", 0, "Valid fibonacci"),
    ("val05", "num role = 2;\nnum accessLevel = 0;\ncheck (role equals 1) {\naccessLevel = 100;\n}\notherwise {\ncheck (role equals 2) {\naccessLevel = 50;\n}\notherwise {\naccessLevel = 10;\n}\n}\nshow(accessLevel);\n", 0, "Valid decision matrix"),
]

passed = 0
for test_id, code, exp_exit, desc in tests:
    with tempfile.NamedTemporaryFile(suffix=".jmm", mode="w", delete=False) as f:
        f.write(code)
        fpath = f.name
    try:
        res = subprocess.run([compiler, fpath, "--verify"], capture_output=True, text=True)
        ok = (res.returncode == exp_exit)
        diag = (res.stderr.strip() or res.stdout.strip()).replace("\n", " | ")
        if ok:
            passed += 1
            print(f"PASS [{test_id}] {desc:25} -> Exit {res.returncode}")
        else:
            print(f"FAIL [{test_id}] {desc:25} -> Expected {exp_exit}, Got {res.returncode}")
            print(f"   Details: {diag[:120]}")
    finally:
        os.remove(fpath)

print(f"\n=======================================================")
print(f"Summary: {passed}/{len(tests)} Tests Passed Perfectly!")
print(f"=======================================================")
