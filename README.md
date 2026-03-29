# Multilingual Programming Language Compiler

A compiler built from scratch in C++ that demonstrates how programming 
languages can be designed around regional languages instead of English. 
The compiler accepts source code written with Hindi-transliterated keywords 
and processes it through a complete 6-phase compilation pipeline, producing 
Three-Address Intermediate Representation as output.

The project is designed so that the entire keyword set can be swapped to 
any language — Devanagari Hindi, Tamil, Telugu, Gujarati, or any other — 
by changing a single configuration file. The current implementation uses 
Hindi keywords written in Latin script as a starting point, with full 
Devanagari and other script support planned as the next step once 
appropriate input tooling is available.

---

## Why This Project

Most programming languages require knowledge of English to read and write 
code. This creates an unnecessary barrier for first-generation programmers 
who think and reason in regional languages. This compiler is a working 
proof of concept that a complete, correct compiler pipeline can be built 
around any human language — not just English.

---

## Current Language — Hindi (Latin Transliteration)

| Keyword | English Equivalent | Category |
|---------|-------------------|----------|
| sankhya | int | Data Type |
| dasha | float | Data Type |
| satya | bool | Data Type |
| shunya | void | Data Type |
| agar | if | Control Flow |
| nahi_to | else | Control Flow |
| jabtak | while | Control Flow |
| baar_baar | for | Control Flow |
| ruko | break | Control Flow |
| kaam | function | Function |
| wapas | return | Function |
| dikhao | print | I/O |
| sach | true | Boolean |
| jhooth | false | Boolean |

---

## Switching to Another Language

All keywords are defined in a single file — `language.h`. To switch the 
entire compiler to a different language, only the values in that file need 
to change. The rest of the compiler — the parser, type checker, IR generator 
— works entirely with internal token types and has no knowledge of what 
language the keywords came from.

For example, switching `sankhya` to `संख्या` in `language.h` immediately 
makes the compiler accept Devanagari Hindi source files. No other file 
needs to be touched.

---

## Compiler Pipeline
```
Source File (.hin)
      |
      v
  [ Lexer ]          — Tokenises source, recognises keywords
      |
      v
  [ Parser ]         — Builds Abstract Syntax Tree (Recursive Descent)
      |
      v
  [ Type Checker ]   — Validates types, scope, and symbol table
      |
      v
  [ IR Generator ]   — Produces Three-Address Intermediate Representation
      |
      v
  [ Optimiser ]      — Constant folding + Dead code elimination
      |
      v
  Output (.ir file)
```

---

## Project Structure

| File | Purpose |
|------|---------|
| `language.h` | Single configuration file — change this to switch languages |
| `token.h` | All token type definitions |
| `lexer.h / lexer.cpp` | Lexical analyser — reads source, produces token stream |
| `ast.h` | All Abstract Syntax Tree node class definitions |
| `parser.h / parser.cpp` | Recursive descent parser — builds AST from tokens |
| `typechecker.h / typechecker.cpp` | Type checker, scope manager, symbol table |
| `ir.h / ir.cpp` | IR generator with constant folding and dead code elimination |
| `main.cpp` | Pipeline orchestrator |
| `input.hin` | Sample Hindi source program |

---

## How to Compile
```bash
g++ -std=c++17 main.cpp lexer.cpp parser.cpp typechecker.cpp ir.cpp -o compiler
```

## How to Run
```bash
./compiler input.hin output.ir
```

---

## Sample Program (input.hin)
```
sankhya x = 10;
sankhya y = 20;
sankhya sum = x + y;

agar (x > 0) {
    dikhao(x);
} nahi_to {
    dikhao(y);
}

jabtak (x > 0) {
    dikhao(x);
    x += -1;
}

kaam sankhya jodo(sankhya a, sankhya b) {
    sankhya hasil = a + b;
    wapas hasil;
}

sankhya answer = jodo(x, y);
dikhao(answer);
```

---

## Technical Highlights

- Recursive Descent Parser with full operator precedence (8 levels)
- Symbol table with nested scope management
- Type inference engine supporting int, float, bool, string, and void
- Three-Address Code IR generation for all control flow constructs
- Constant folding with propagation across the entire IR
- Dead code elimination pass
- Precise error reporting with line and column numbers across all phases
- Single-file language configuration enabling full language swapping

---

## Built With

- C++17
- Standard Library only — no external dependencies
