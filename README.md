# Hydro

Hydro is a small experimental programming language and compiler written in **C++**.  
The project demonstrates the fundamental stages of compiler construction including **tokenization, parsing, AST construction, and assembly code generation**.

The compiler translates `.hy` source files into **x86-64 assembly**, which is then assembled and linked into a runnable Linux executable using **NASM** and **LD**.

---

# Example Program

`test.hy`

```hydro
let x = 5 + 6;
exit(x);
```

Running this program produces exit code:

```
11
```

---

# Compiler Pipeline

Hydro follows a traditional compiler architecture:

```
Source Code (.hy)
        │
        ▼
Tokenizer (Lexer)
        │
        ▼
Parser
        │
        ▼
Abstract Syntax Tree (AST)
        │
        ▼
Code Generator
        │
        ▼
x86-64 Assembly
        │
        ▼
NASM
        │
        ▼
Object File
        │
        ▼
LD
        │
        ▼
Executable
```

---

# Project Structure

```
hydro/
│
├── main.cpp
├── tokenization.hpp
├── parser.hpp
├── generation.hpp
├── arena.hpp
│
├── test.hy
│
└── out.asm / out.o / out
```

### Key Components

| File | Description |
|-----|-------------|
| `main.cpp` | Entry point of the compiler |
| `tokenization.hpp` | Lexer that converts source code into tokens |
| `parser.hpp` | Recursive descent parser that builds the AST |
| `generation.hpp` | Code generator that emits x86-64 assembly |
| `arena.hpp` | Custom arena allocator for fast AST node allocation |

---

# Language Grammar

```
{prog}
    → { {stmt} }

{stmt}
    → exit ( {expr} ) ;
    | let ident = {expr} ;

{expr}
    → {term}
    | {binExpr}

{term}
    → int_lit
    | ident

{binExpr}
    → {expr} * {expr}   (precedence 1)
    | {expr} + {expr}   (precedence 0)
```

---

# Abstract Syntax Tree (AST)

Example program:

```
let x = 5 + 6;
exit(x);
```

AST representation:

```
Program
 ├── LetStmt
 │    ├── name: x
 │    └── BinaryExpr(+)
 │         ├── IntLiteral(5)
 │         └── IntLiteral(6)
 │
 └── ExitStmt
      └── Identifier(x)
```

---

# Memory Management

The compiler uses a **custom Arena Allocator** to allocate AST nodes.

Advantages:

- Fast allocation
- Reduced heap fragmentation
- Single bulk deallocation when compilation finishes

Example usage:

```cpp
NodeExprIntLit* node = arena.alloc<NodeExprIntLit>();
```

This avoids repeated `new` allocations during parsing.

---

# Building the Compiler

### Requirements

- `g++` (C++17)
- `nasm`
- `ld`
- Linux environment

### Compile the compiler

```bash
g++ -std=c++17 main.cpp -o hydro
```

---

# Running the Compiler

Compile a Hydro program:

```bash
./hydro test.hy
```

This produces:

```
out.asm
out.o
out
```

---

# Running the Generated Program

```bash
./out
```

Check the exit code:

```bash
echo $?
```

Expected output:

```
11
```

---

# Features

Current language capabilities:

- Integer literals
- Variables (`let`)
- Binary expressions (`+`, `*`)
- Exit statements
- AST generation
- x86-64 assembly generation

---

# Future Improvements

Planned extensions:

- subtraction and division operators
- variable scopes
- conditionals (`if`)
- loops (`while`)
- functions
- improved error reporting
- type checking

---

# Educational Purpose

This project was created to explore:

- compiler architecture
- parsing techniques
- AST design
- low-level code generation
- custom memory allocators

---

# License

MIT License
