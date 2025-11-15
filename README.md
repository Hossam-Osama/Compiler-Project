# Lexical Analyzer Generator – Phase 1 (Compiler Project)

**Alexandria University – CSE – Programming Languages & Compilers**

a suitable Syntax Directed Translation Scheme to convert Java code to Java bytecode, performing necessary lexical, syntax and static semantic analysis (such as type checking and Expressions Evaluation).

---

## Project Overview

This project implements **Phase 1** of a compiler that ultimately translates **Java code to Java Bytecode**. In this phase, we build a **Lexical Analyzer Generator** using C++.

The generator reads regular definitions and regular expressions, constructs:

- Non-deterministic Finite Automata (NFA)
- Combined NFA
- Deterministic Finite Automata (DFA)
- Minimized DFA
- Transition Table
- A generated lexical analyzer capable of tokenizing input programs

This lexical analyzer will integrate with a parser in **Phase 2**.

---

## Project Structure

```
compiler-project/
│
├── CMakeLists.txt
├── README.md
│
├── src/
│   ├── main.cpp
│   ├── lexer/
│   │   ├── RegexParser.cpp
│   │   ├── Thompson.cpp
│   │   ├── NFA.cpp
│   │   ├── DFA.cpp
│   │   ├── Minimize.cpp
│   │   ├── Generator.cpp
│   │   └── ...
│   │
│   └── util/
│       ├── File.cpp
│       └── Helpers.cpp
│
├── include/
│   ├── RegexParser.h
│   ├── Thompson.h
│   ├── NFA.h
│   ├── DFA.h
│   ├── Minimize.h
│   ├── Generator.h
│   └── ...
│
├── tests/
│   ├── lexical_rules.txt
│   ├── sample_program.java
│
└── output/
    ├── transition_table.txt
    └── generated_lexer.cpp
```

---

## Build Instructions

### Prerequisites

Ensure the following are installed:

- **CMake ≥ 3.16**
- **C++ Compiler** (MSVC or MinGW or Clang)
- **Ninja** (optional but recommended)

### Build Steps

```
mkdir build
cd build
cmake ..
cmake --build .
```

The final executable will be:

```
compiler_project.exe (Windows)
./compiler_project   (Linux/Mac)
```

---

## Running the Lexical Analyzer Generator

Example:

```
./compiler_project ../tests/lexical_rules.txt
```

This will:

1. Parse lexical rules
2. Build NFA → DFA → Minimized DFA
3. Output transition table (output/transition_table.txt)
4. Generate lexical analyzer (output/generated_lexer.cpp)

---

## Testing the Generated Lexer

```
g++ output/generated_lexer.cpp -o lexer
./lexer ../tests/sample_program.java
```

You should get a token stream such as:

```
int
id
,
id
num
while
(
id
relop
num
)
...
```

---

## Lexical Rules Input Format

Supports:

- Regular definitions: `LHS = RHS`
- Regular expressions: `LHS: RHS`
- Keywords: `{ }`
- Punctuation: `[ ]`
- Lambda: `\L`
- Operators: `| + * ( )`
- Escaped characters: `\`

Example:

```
letter = a-z | A-Z
digit = 0-9
id: letter (letter|digit)*
digits = digit+
{ boolean int float }
num: digit+ | digit+ . digits ( \L | E digits )
relop: \=\= | !\= | > | >\= | < | <\=
assign: =
[ ; , \( \) { } ]
```

---

## Phase 1 Deliverables

Your submission must include:

1. Executable + Source Code
2. Project Report containing:
   - Data structures used
   - Explanation of algorithms
   - Minimized DFA transition table
   - Token stream of test program
   - Assumptions and justification

---

## Team Members

(Add names here)

---

## Notes

This lexical analyzer will be used directly by the parser in **Phase 2**, so consistency in token naming is important.

Good luck!
