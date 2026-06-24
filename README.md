# Hydrogen - A Low-Level Programming Language Compiler

![Language](https://img.shields.io/badge/Language-C%2B%2B-brightyellow?style=flat-square)
![Standard](https://img.shields.io/badge/Standard-C%2B%2B20-blue?style=flat-square)
![Build System](https://img.shields.io/badge/Build-CMake-green?style=flat-square)
![License](https://img.shields.io/badge/License-MIT-brightgreen?style=flat-square)

**Hydrogen** is a minimalist compiler project that transpiles a simple high-level language into x86_64 assembly code. It demonstrates fundamental compiler concepts including lexical analysis, syntax analysis, and code generation while producing standalone x86_64 executable binaries.

---

## Table of Contents

1. [Quick Start](#quick-start)
2. [Overview](#overview)
3. [Language Grammar & Syntax](#language-grammar--syntax)
4. [Project Architecture](#project-architecture)
5. [Compiler Pipeline](#compiler-pipeline)
6. [Build Instructions](#build-instructions)
7. [Usage Guide](#usage-guide)
8. [Code Structure](#code-structure)
9. [Detailed Component Reference](#detailed-component-reference)
10. [Advanced Topics](#advanced-topics)
11. [Examples](#examples)
12. [Troubleshooting](#troubleshooting)

---

## Quick Start

### Prerequisites
- **C++20 compliant compiler** (GCC 9+, Clang 10+, or MSVC 2019+)
- **CMake** 3.20 or newer
- **NASM** (Netwide Assembler) for assembly compilation
- **GNU ld** (GNU Linker) for linking object files
- Standard build tools (`make` or `ninja`)

### Installation & Build

```bash
# Clone or navigate to the project directory
cd hydrogen

# Configure the CMake build system
cmake -S . -B build

# Compile the Hydrogen compiler
cmake --build build -j

# Verify the build was successful
./build/hydro --help
```

### Hello, World! (Hydrogen Edition)

Create a file named `hello.hy`:

```hydrogen
let x = 42;
exit(x);
```

Compile and run:

```bash
./build/hydro hello.hy
./out
echo $?  # Should print: 42
```

---

## Overview

Hydrogen is an educational compiler project that bridges high-level programming concepts and low-level assembly code. Rather than interpreting code or compiling to bytecode, Hydrogen generates direct x86_64 assembly instructions, demonstrating:

- **Lexical Analysis**: Breaking source code into meaningful tokens
- **Syntax Analysis**: Parsing tokens into an Abstract Syntax Tree (AST)
- **Code Generation**: Converting AST nodes into x86_64 assembly instructions
- **System Integration**: Leveraging external tools (NASM, ld) for assembly and linking

### Key Characteristics

| Feature | Details |
|---------|---------|
| **Input Format** | `.hy` files (Hydrogen source code) |
| **Output Format** | x86_64 executable binaries |
| **Intermediate Format** | NASM-compatible x86_64 assembly |
| **Runtime Model** | Standalone bare-metal binaries |
| **Type System** | Statically inferred (all values are 64-bit integers) |
| **Memory Model** | Stack-based (LIFO allocation) |

---

## Language Grammar & Syntax

The Hydrogen language follows a formal context-free grammar designed for simplicity and clarity.

### Formal Grammar (ABNF-style)

```ebnf
program     :: statement*

statement   :: exit_stmt
            | let_stmt
            | if_stmt

exit_stmt   :: "exit" "(" expression ")" ";"
            | "exit" "(" expression ")"

let_stmt    :: "let" identifier "=" expression ";"
            | "let" identifier "=" expression

if_stmt     :: "if" "(" expression ")" "{" statement* "}"
            | "if" "(" expression ")" "{" statement* "}" "else" "{" statement* "}"

expression  :: term
            | binary_expr

binary_expr :: expression ("+" | "-" | "*" | "/") expression

term        :: integer_literal
            | identifier

integer_literal :: digit+

identifier  :: letter (letter | digit | "_")*
```

### Grammar Precedence Table

The compiler respects operator precedence and left-to-right associativity:

| Precedence Level | Operators | Precedence | Examples |
|------------------|-----------|-----------|----------|
| 1 (Highest) | `*`, `/` | Left-associative | `a * b / c` evaluates as `(a * b) / c` |
| 0 (Lower) | `+`, `-` | Left-associative | `a + b - c` evaluates as `(a + b) - c` |

### Operator Reference

| Operator | Name | Type | Description | Example |
|----------|------|------|-------------|---------|
| `+` | Addition | Binary | Adds two expressions | `let sum = 5 + 3;` |
| `-` | Subtraction | Binary | Subtracts right from left | `let diff = 10 - 4;` |
| `*` | Multiplication | Binary | Multiplies two expressions | `let prod = 6 * 7;` |
| `/` | Division | Binary | Integer division (truncates) | `let quot = 20 / 4;` |
| `=` | Assignment | Binary | Assigns expression to variable | `let x = 100;` |
| `exit()` | Exit Statement | Unary Function | Terminates program with exit code | `exit(42);` |

### Reserved Keywords

| Keyword | Purpose | Usage |
|---------|---------|-------|
| `exit` | Program termination | `exit(exit_code);` |
| `let` | Variable declaration | `let variable_name = value;` |
| `if` | Conditional branching | `if (condition) { ... }` |
| `else` | Conditional alternative | `if (...) { ... } else { ... }` |

### Data Types

The Hydrogen language uses a single unified data type:

- **Integer (i64)**: All values are 64-bit signed integers, following x86_64 conventions

---

## Project Architecture

### High-Level Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                    Source Code (.hy)                         │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│                  TOKENIZATION STAGE                          │
│  (Lexical Analysis: Characters → Tokens)                     │
│               Handled by: Tokenizer                          │
└────────────────────┬────────────────────────────────────────┘
                     │
                     ▼
┌─────────────────────────────────────────────────────────────┐
│    PARSING STAGE                                             │
│    (Syntax Analysis: Tokens → Abstract Syntax Tree)         │
│    Handled by: Parser                                        │
└────────────────────┬────────────────────────────────────────┘
                     │
                     ▼
┌─────────────────────────────────────────────────────────────┐
│    CODE GENERATION STAGE                                    │
│    (AST → x86_64 Assembly)                                  │
│    Handled by: Generator                                    │
└────────────────────┬────────────────────────────────────────┘
                     │
                     ▼
┌─────────────────────────────────────────────────────────────┐
│                    out.asm (Assembly)                        │
└────────────────────┬────────────────────────────────────────┘
                     │
        ┌────────────┴────────────┬─────────────┐
        │                         │             │
        ▼                         ▼             ▼
┌──────────────┐         ┌────────────────┐   │
│ NASM (nasm)  │         │ NASM Assembly  │   │
│ Assembler    │────────▶│ Parser & Linker│   │
└──────────────┘         └────────┬───────┘   │
                                  │           │
                                  ▼           │
                         ┌────────────────┐  │
                         │   out.o        │  │
                         │  (Object File) │  │
                         └────────┬───────┘  │
                                  │          │
                                  ▼          │
                         ┌────────────────┐  │
                         │   ld (linker)  │◀─┘
                         │                │
                         └────────┬───────┘
                                  │
                                  ▼
                         ┌────────────────┐
                         │   out (Binary) │
                         │  (Executable)  │
                         └────────────────┘
```

### Directory Structure

```
hydrogen/
├── CMakeLists.txt              # CMake build configuration
├── README.md                   # This file
├── Instructions.txt            # Quick reference guide
├── src/
│   ├── main.cpp               # Entry point & orchestration
│   ├── tokenization.hpp       # Lexical analysis (tokenizer)
│   ├── parser.hpp             # Syntax analysis (parser)
│   ├── generation.hpp         # Code generation
│   └── arena.hpp              # Memory management utilities
├── build/                      # Build artifacts (generated by CMake)
│   ├── hydro                   # Compiled Hydrogen compiler executable
│   ├── CMakeCache.txt
│   └── CMakeFiles/
├── test.hy                     # Example Hydrogen source file
├── out.asm                     # Generated assembly (example)
└── out                         # Generated executable (example)
```

---

## Compiler Pipeline

The Hydrogen compiler follows a classic three-stage compiler architecture with external tool integration.

### Stage 1: Tokenization (Lexical Analysis)

**Component**: `tokenization.hpp::Tokenizer`

**Purpose**: Convert raw source code characters into a stream of meaningful tokens

**Process**:

```
Source Code (string)
    ↓
[Character scanning with lookahead]
    ↓
[Pattern matching for keywords, numbers, symbols]
    ↓
Token Stream (vector<Token>)
```

**Token Types**:

```cpp
enum class TokenType {
    exit,           // "exit" keyword
    int_lit,        // Integer literal (e.g., "42")
    semi,           // Semicolon ";"
    open_paren,     // Left parenthesis "("
    close_paren,    // Right parenthesis ")"
    ident,          // Identifier (variable name)
    let,            // "let" keyword
    eq,             // Equals sign "="
    plus,           // Addition "+"
    minus,          // Subtraction "-"
    mul,            // Multiplication "*"
    div,            // Division "/"
    open_curly,     // Left brace "{"
    close_curly,    // Right brace "}"
    if_tok,         // "if" keyword
    else_tok        // "else" keyword
};
```

**Example**:

```
Input:  "let x = 42;"
Output: [
    Token{TokenType::let},
    Token{TokenType::ident, "x"},
    Token{TokenType::eq},
    Token{TokenType::int_lit, "42"},
    Token{TokenType::semi}
]
```

### Stage 2: Parsing (Syntax Analysis)

**Component**: `parser.hpp::Parser`

**Purpose**: Verify token sequence follows grammar rules and build an Abstract Syntax Tree (AST)

**Process**:

```
Token Stream
    ↓
[Recursive descent parsing]
    ↓
[Grammar rule validation with backtracking]
    ↓
Abstract Syntax Tree (AST)
```

**AST Node Types**:

- `NodeProg`: Root program node containing all statements
- `NodeStmt`: Base statement type (exit, let, if)
- `NodeExpr`: Expression nodes (terms and binary operations)
- `NodeTerm`: Terminal values (integers, identifiers)
- `NodeBinExpr*`: Binary expression nodes (Add, Sub, Mul, Div)

**Example AST Construction**:

```
Input:  "let x = 5 + 3;"

AST:    NodeProg
        └── NodeStmt (let)
            ├── identifier: "x"
            └── NodeExpr
                └── NodeBinExprAdd
                    ├── lhs: NodeTermIntLit{5}
                    └── rhs: NodeTermIntLit{3}
```

**Parsing Strategy**: Recursive Descent

The parser uses recursive descent with operator precedence climbing to handle expressions:

1. **Statements**: `parse_prog()` → `parse_stmt()` (for each statement)
2. **Expressions**: `parse_expr()` → `parse_bin_expr()` → `parse_term()`
3. **Precedence**: Handled through separate parsing functions for each precedence level

### Stage 3: Code Generation

**Component**: `generation.hpp::Generator`

**Purpose**: Traverse AST and emit x86_64 assembly code

**Process**:

```
Abstract Syntax Tree
    ↓
[AST visitor pattern traversal]
    ↓
[x86_64 instruction emission]
    ↓
Assembly code (NASM syntax)
```

**Key Concepts**:

- **Stack-based Evaluation**: All expressions evaluated using the system stack
- **Register Allocation**: Primarily uses RAX for temporary values
- **Stack Pointer (RSP)**: Managed for local variable storage
- **Scope Management**: Separate scopes track variable lifetimes

**Register Usage**:

| Register | Purpose | Reserved | Purpose |
|----------|---------|----------|---------|
| RAX | Accumulator (temporary expressions) | RSP | Stack Pointer |
| RBX-RDX | (Available) | RBP | Base Pointer (optional) |
| R8-R15 | (Available) | | |

**Assembly Output Example**:

```nasm
; Generated from: let x = 10; exit(x);

mov rax, 10
push rax                ; x is now at [rsp]
mov rax, [rsp]          ; Load x back into RAX
push rax
mov rax, [rsp]          ; Load exit code
pop rcx                 ; Clean up stack
mov rdi, rax            ; Setup exit code in rdi
mov rax, 60             ; syscall: exit
syscall
```

### Stage 4: Assembly & Linking (External Tools)

**Components**: NASM Assembler + GNU ld Linker

**NASM Assembly**:
```bash
nasm -felf64 out.asm -o out.o
```
- Converts x86_64 assembly syntax to ELF64 object format
- Performs encoding of assembly mnemonics to machine code
- Generates per-function relocatable code

**GNU Linking**:
```bash
ld -o out out.o
```
- Resolves symbol relocations
- Combines object files into executable
- Links with standard library if needed

**Output**: Standalone x86_64 Linux executable binary

---

## Build Instructions

### System Requirements

| Component | Version | Purpose |
|-----------|---------|---------|
| CMake | ≥ 3.20 | Build system configuration |
| C++ Compiler | C++20 | Compile Hydrogen compiler |
| NASM | Latest | Assemble generated code |
| GNU ld | Standard | Link object files to executables |
| Make or Ninja | Standard | Execute build commands |

### Building from Source

#### Method 1: Using CMake (Recommended)

```bash
# Navigate to project root
cd /path/to/hydrogen

# Create and configure build directory
cmake -S . -B build

# Build with all available CPU cores
cmake --build build -j

# (Optional) Install to system
cmake --install build --prefix /usr/local
```

#### Method 2: Manual Compilation

If CMake is unavailable (not recommended):

```bash
cd src
g++ -std=c++20 -O2 main.cpp -o ../build/hydro
```

### Build Configuration Options

Customize the build with CMake variables:

```bash
# Debug build (with symbols, no optimization)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# Release build (optimized, stripped)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Verbose output (show all compiler commands)
cmake --build build -- VERBOSE=1
```

### Verifying the Build

```bash
# Check if compiler executable exists and is runnable
file build/hydro

# Show compiler version/help
./build/hydro

# Test with provided example
./build/hydro test.hy
./out
echo $?  # Should display program exit code
```

### Troubleshooting Build Issues

| Error | Cause | Solution |
|-------|-------|----------|
| `CMake not found` | CMake not installed | Install CMake: `apt install cmake` |
| `C++20 not supported` | Old compiler version | Update GCC/Clang or use newer compiler |
| `nasm not found` | NASM not installed | Install NASM: `apt install nasm` |
| `ld not found` | GNU binutils missing | Install binutils: `apt install binutils` |
| File permission denied | Executable not marked | Run: `chmod +x ./build/hydro` |

---

## Usage Guide

### Basic Compiler Usage

#### Compile a Hydrogen file

```bash
# Using default filename (test.hy)
./build/hydro

# Using specific input file
./build/hydro myprogram.hy

# Using absolute path
./build/hydro /path/to/program.hy
```

#### Run the compiled binary

```bash
# Execute the generated binary
./out

# Check exit code
echo $?

# Run and capture exit code
./out
EXIT_CODE=$?
echo "Program exited with code: $EXIT_CODE"
```

### Artifact Generation

The compiler generates three files in the same directory as the source file:

| File | Type | Purpose | Viewable |
|------|------|---------|----------|
| `out.asm` | Assembly | NASM x86_64 assembly code | ✓ (human-readable) |
| `out.o` | Object | Compiled machine code (relocatable) | ✗ (binary) |
| `out` | Binary | Final executable | ✓ (executable) |

### Example: Complete Workflow

```bash
# Create source file
cat > sample.hy <<'EOF'
let a = 100;
let b = 20;
exit(a + b);
EOF

# Compile
./build/hydro sample.hy

# Check generated assembly
cat out.asm

# Run and verify
./out
echo "Exit code: $?"
```

### Command-Line Error Handling

The compiler provides descriptive error messages:

```bash
# File not found error
$ ./build/hydro nonexistent.hy
Failed to open file: nonexistent.hy

# Syntax error
$ ./build/hydro bad_syntax.hy
Invalid program

# Runtime variable reference error (during code generation)
$ ./build/hydro bad_var.hy
Undeclared identifier: undefined_var

# NASM assembly error (if invalid assembly generated)
nasm failed

# Linker error
ld failed
```

---

## Code Structure

### File Organization and Responsibilities

#### [main.cpp](src/main.cpp)

**Responsibility**: Orchestration and I/O handling

**Key Functions**:

```cpp
int main(int argc, char* argv[])
```

**Workflow**:

1. Parse command-line arguments (input filename)
2. Read source file into memory
3. Create `Tokenizer` and generate tokens
4. Create `Parser` and build AST
5. Create `Generator` and emit assembly
6. Write assembly to `out.asm`
7. Execute NASM assembly: `nasm -felf64 out.asm -o out.o`
8. Execute linker: `ld -o out out.o`
9. Return exit status

**Error Handling**: Validates each stage and reports failures with context

#### [tokenization.hpp](src/tokenization.hpp)

**Responsibility**: Lexical analysis

**Key Classes**:

```cpp
class Tokenizer {
    std::vector<Token> tokenize();            // Main entry point
    std::optional<char> peek();               // Look at next char
    char consume();                           // Get and advance
    bool is_alpha(char c) const;              // Check if letter
    bool is_digit(char c) const;              // Check if digit
};
```

**Algorithm**: Character-by-character scan with lookahead

**Token Vector**: `std::vector<Token>` where each `Token` contains:
- `TokenType type` - Classification (keyword, operator, etc.)
- `std::optional<std::string> value` - Lexeme (actual text)

#### [parser.hpp](src/parser.hpp)

**Responsibility**: Syntax analysis and AST construction

**Key Classes**:

```cpp
class Parser {
    std::optional<NodeProg> parse_prog();           // Program
    std::optional<NodeStmt*> parse_stmt();          // Statement
    std::optional<NodeExpr*> parse_expr();          // Expression
    std::optional<NodeExpr*> parse_bin_expr(int prec);  // Binary expr
    std::optional<NodeTerm*> parse_term();          // Terminal
};
```

**AST Node Hierarchy**:

```
NodeProg (root)
  └── statements: vector<NodeStmt*>
      └── NodeStmt variants:
          ├── NodeStmtExit { expression: NodeExpr* }
          ├── NodeStmtLet { identifier, expr: NodeExpr* }
          └── NodeStmtIf { condition, body, else_body }

NodeExpr variants:
  ├── NodeTermIntLit { value }
  ├── NodeTermIdent { name }
  └── NodeBinExpr* { lhs: NodeExpr*, rhs: NodeExpr* }
      ├── NodeBinExprAdd
      ├── NodeBinExprSub
      ├── NodeBinExprMul
      └── NodeBinExprDiv
```

#### [generation.hpp](src/generation.hpp)

**Responsibility**: Code generation (AST → x86_64 assembly)

**Key Classes**:

```cpp
class Generator {
    std::string gen_prog();                    // Generate complete program
    void gen_stmt(const NodeStmt* stmt);       // Generate statement
    std::string gen_expr(const NodeExpr* expr);  // Generate expression
    void gen_term(const NodeTerm* term);       // Generate term
    void push(const std::string& reg);         // Push to stack
    void pop(const std::string& reg);          // Pop from stack
};
```

**State Management**:

```cpp
std::unordered_map<std::string, Variable> m_scopes;  // Variable tracking
size_t m_stack_size;                                   // Stack pointer offset
std::stringstream m_output;                            // Assembly output buffer
```

#### [arena.hpp](src/arena.hpp)

**Responsibility**: Memory management for AST nodes

**Key Classes**:

```cpp
class ArenaAllocator {
    template<typename T, typename... Args>
    T* alloc(Args&&... args);                 // Allocate and construct
};
```

**Purpose**: Efficient bulk allocation of AST nodes

**Advantages**:
- Reduces fragmentation vs. individual `new`
- Automatic cleanup when arena destroyed
- Deterministic memory layout

---

## Detailed Component Reference

### Tokenizer Reference

#### Tokenization Algorithm

```
INPUTS: Source code string
OUTPUTS: Vector of Token objects

1. Initialize index = 0
2. WHILE index < source.length():
    a. Get current character c = source[index]
    
    IF c is whitespace:
        SKIP (increment index)
    
    ELSE IF c is letter:
        COLLECT identifier/keyword until non-letter
        IF keyword (exit, let, if, else):
            CREATE keyword token
        ELSE:
            CREATE identifier token
    
    ELSE IF c is digit:
        COLLECT digits as integer literal
        CREATE int_lit token
    
    ELSE:
        Match single-character tokens:
        - '(' → open_paren
        - ')' → close_paren
        - ';' → semi
        - '=' → eq
        - '+' → plus
        - '-' → minus
        - '*' → mul
        - '/' → div
        - '{' → open_curly
        - '}' → close_curly
        - else → ERROR (unknown character)

3. RETURN tokens vector
```

#### Token Types Explained

| Token Type | Syntax | Example | Value |
|------------|--------|---------|-------|
| `exit` | Keyword | `exit` | None |
| `let` | Keyword | `let` | None |
| `if` | Keyword | `if` | None |
| `else` | Keyword | `else` | None |
| `int_lit` | Integer | `42`, `1000` | String representation |
| `ident` | Identifier | `x`, `myVar` | Variable name |
| `open_paren` | `(` | `(` | None |
| `close_paren` | `)` | `)` | None |
| `open_curly` | `{` | `{` | None |
| `close_curly` | `}` | `}` | None |
| `semi` | `;` | `;` | None |
| `eq` | `=` | `=` | None |
| `plus` | `+` | `+` | None |
| `minus` | `-` | `-` | None |
| `mul` | `*` | `*` | None |
| `div` | `/` | `/` | None |

### Parser Reference

#### Parsing Algorithm (Recursive Descent)

```cpp
// Top-level program parser
parse_prog() {
    statements = vector<NodeStmt*>
    WHILE not at end:
        stmt = parse_stmt()
        IF stmt is valid:
            statements.push_back(stmt)
        ELSE:
            RETURN error
    RETURN NodeProg{statements}
}

// Statement parser
parse_stmt() {
    PEEK at next token
    IF token is "exit":
        CONSUME "exit"
        EXPECT "("
        expr = parse_expr()
        EXPECT ")"
        OPTIONAL ";"
        RETURN NodeStmtExit{expr}
    
    ELSE IF token is "let":
        CONSUME "let"
        ident = CONSUME identifier
        EXPECT "="
        expr = parse_expr()
        OPTIONAL ";"
        RETURN NodeStmtLet{ident, expr}
    
    ELSE IF token is "if":
        CONSUME "if"
        EXPECT "("
        condition = parse_expr()
        EXPECT ")"
        EXPECT "{"
        body = parse_prog()  // Block of statements
        EXPECT "}"
        
        IF PEEK is "else":
            CONSUME "else"
            EXPECT "{"
            else_body = parse_prog()
            EXPECT "}"
        
        RETURN NodeStmtIf{condition, body, else_body}
    
    ELSE:
        RETURN error
}

// Expression parser (handles precedence)
parse_expr() {
    RETURN parse_bin_expr(0)
}

// Binary expression with precedence climbing
parse_bin_expr(min_prec) {
    lhs = parse_term()
    
    WHILE PEEK is binary operator AND operator.precedence >= min_prec:
        op = CONSUME operator
        op_prec = operator.precedence
        rhs = parse_bin_expr(op_prec + 1)
        
        CREATE appropriate BinExpr node based on operator
        lhs = new BinExprNode{lhs, rhs}
    
    RETURN lhs
}

// Terminal parser (base case)
parse_term() {
    PEEK at next token
    IF token is integer:
        value = CONSUME int_lit
        RETURN NodeTermIntLit{value}
    
    ELSE IF token is identifier:
        name = CONSUME ident
        RETURN NodeTermIdent{name}
    
    ELSE:
        RETURN error
}
```

#### Precedence Climbing Algorithm

Handles operator precedence without ambiguity:

```
Expression: 2 + 3 * 4 - 1

Step 1: parse_bin_expr(0)
  lhs = 2 (parse_term)
  op = '+' (prec 0)
  rhs = parse_bin_expr(1)  ← Enter with min_prec=1
    
    Step 2: parse_bin_expr(1)
      lhs = 3 (parse_term)
      op = '*' (prec 1)  ← prec >= min_prec, continue
      rhs = parse_bin_expr(2)
        
        Step 3: parse_bin_expr(2)
          lhs = 4 (parse_term)
          op = '-' (prec 0)  ← prec < min_prec, stop
          RETURN 4
      
      lhs = (3 * 4)
      op = '-' (prec 0)  ← prec < min_prec, stop
      RETURN (3 * 4)
  
  lhs = (2 + (3 * 4))
  op = '-' (prec 0)  ← prec >= min_prec, continue
  rhs = parse_bin_expr(1)
    
    Step 4: parse_bin_expr(1)
      lhs = 1 (parse_term)
      No more operators
      RETURN 1
  
  lhs = ((2 + (3 * 4)) - 1)

Result: ((2 + (3 * 4)) - 1) ✓ Correct precedence
```

### Generator Reference

#### Code Generation Algorithm

```cpp
gen_prog() {
    FOR EACH statement IN program:
        gen_stmt(statement)
    
    APPEND syscall sequence:
        mov rax, 60         ; exit syscall number
        syscall
    
    RETURN assembled_asm_string
}

gen_stmt(stmt) {
    IF stmt is exit:
        gen_expr(stmt.expression)
        // RAX now contains exit code
        // Pop stack if needed
    
    ELSE IF stmt is let:
        gen_expr(stmt.expression)
        // RAX contains expression result
        push(rax)
        // Variable now at [rsp]
        record variable location
    
    ELSE IF stmt is if:
        gen_expr(stmt.condition)
        // RAX contains condition value
        CREATE unique labels: then_label, else_label, end_label
        
        cmp rax, 0
        je else_label       ; Jump if false (zero)
        
        gen_stmt(stmt.body)
        jmp end_label
        
        else_label:
        IF stmt.else_body exists:
            gen_stmt(stmt.else_body)
        
        end_label:
}

gen_expr(expr) {
    IF expr is int literal:
        mov rax, value
        push rax
    
    ELSE IF expr is identifier:
        LOOKUP variable in scope
        mov rax, [rsp + offset]
        push rax
    
    ELSE IF expr is binary operation:
        rhs = gen_expr(expr.rhs)        ; Evaluate RHS first
        pop rbx                          ; RHS in RBX
        lhs = gen_expr(expr.lhs)        ; Evaluate LHS
        pop rax                          ; LHS in RAX
        
        PERFORM operation (add/sub/mul/div)
        push rax                         ; Result back on stack
}
```

#### Stack Frame Layout

```
[Higher Addresses]
┌──────────────────────┐
│   Variable N         │  [rsp + offset_n]
├──────────────────────┤
│   ...                │
├──────────────────────┤
│   Variable 2         │  [rsp + offset_2]
├──────────────────────┤
│   Variable 1         │  [rsp + offset_1]
├──────────────────────┤
│   Temporary/Return   │  [rsp + 0]  ← Stack top
└──────────────────────┘
[Lower Addresses]

Example: let x = 10; let y = 20;
After "let x = 10":
  [rsp] = 10  (x stored here)
  
After "let y = 20":
  [rsp + 0] = 20    (y stored here)
  [rsp + 8] = 10    (x stored here, shifted up)
```

#### Register Usage in Code Generation

| Register | Usage | Preserved | Notes |
|----------|-------|-----------|-------|
| RAX | Accumulator (temp values) | No | Used for all expressions |
| RBX | Temporary storage | No | Used in binary operations |
| RSP | Stack pointer | Yes | Managed by CPU |
| RDI | Exit code parameter | No | Set before syscall |

---

## Advanced Topics

### Memory Management

#### Arena Allocation

The `ArenaAllocator` class provides efficient bulk allocation for AST nodes:

```cpp
class ArenaAllocator {
    std::vector<std::byte> m_buffer;
    size_t m_offset = 0;
    
public:
    template<typename T, typename... Args>
    T* alloc(Args&&... args) {
        // Allocate space for T
        void* ptr = m_buffer.data() + m_offset;
        m_offset += sizeof(T);
        
        // Construct T in-place
        return new (ptr) T(std::forward<Args>(args)...);
    }
};
```

**Benefits**:
- Avoids fragmentation from individual allocations
- Single deallocation when arena destroyed
- Cache-friendly memory layout
- Deterministic cleanup

#### Variable Scope Management

The Generator maintains a stack of scopes:

```cpp
std::vector<std::unordered_map<std::string, Variable>> m_scopes;

struct Variable {
    size_t stack_loc;  // Position on stack from top
};
```

**Scope Operations**:

```cpp
push_scope() {
    m_scopes.push_back({});
}

declare_variable(name, location) {
    m_scopes.back()[name] = {location};
}

lookup_variable(name) {
    // Search from innermost to outermost scope
    FOR i FROM m_scopes.size()-1 DOWN TO 0:
        IF name IN m_scopes[i]:
            RETURN m_scopes[i][name]
    RETURN NOT_FOUND
}

pop_scope() {
    m_scopes.pop_back();
}
```

### Type System (and Limitations)

**Current Type System**:
- **Single Type**: All values are 64-bit signed integers
- **No Type Checking**: Compiler doesn't validate type compatibility
- **Implicit Conversions**: None needed (only one type exists)

**Future Extensions** (Design Hints):

```cpp
enum class Type {
    Int64,
    Int32,
    Int16,
    Pointer,
    Function,
    Struct
};

// Type-aware expression node
struct TypedExpr {
    NodeExpr* expr;
    Type type;
};
```

### x86_64 Architecture Specifics

#### Calling Convention (System V AMD64 ABI)

The Hydrogen compiler uses the System V AMD64 ABI for Linux:

| Register | Usage | Caller-Save | Purpose |
|----------|-------|-------------|---------|
| RAX | Return value | Yes | Function return |
| RDI | First argument | Yes | Exit code for syscall |
| RSI | Second argument | Yes | - |
| RDX | Third argument | Yes | - |
| RCX | Fourth argument | Yes | - |
| R8, R9 | Fifth, sixth | Yes | - |
| RBX, RBP | Local storage | No | Preserved |
| RSP | Stack pointer | - | Automatic |

#### Syscall Interface

The `exit` system call:

```nasm
mov rax, 60     ; syscall number for exit_group
mov rdi, code   ; exit code in RDI
syscall         ; Invoke kernel
```

#### ELF64 Binary Format

Generated binaries follow ELF64 (64-bit Executable and Linkable Format):

```
ELF Header
├── Magic: 0x7F 'E' 'L' 'F'
├── Class: ELFCLASS64
├── Data: ELFDATA2LSB (little-endian)
├── Version: EV_CURRENT
└── Type: ET_EXEC (executable)

Program Headers
├── .text section (code)
└── ... (other sections)

Section Headers
├── .text (executable code)
├── .shstrtab (section name table)
└── ... (other metadata)
```

### Performance Considerations

#### Optimization Opportunities

1. **Constant Folding**: Evaluate constant expressions at compile time
   ```cpp
   // Current: 5 + 3 generates: mov 5, mov 3, add
   // Optimized: Exit code directly to 8
   ```

2. **Dead Code Elimination**: Remove unreachable statements
   ```cpp
   if (0) { ... }  // Can be eliminated
   ```

3. **Register Allocation**: Minimize stack operations
   ```cpp
   // Current: All values via stack
   // Optimized: Keep hot values in registers
   ```

4. **Jump Optimization**: Combine sequential jumps
   ```nasm
   jmp label1
   label1: jmp label2   ; Can remove label1
   ```

### Semantic Analysis (Type & Error Checking)

#### Current Validation

The compiler performs minimal validation:
- Token syntax verification
- Grammar conformance
- Symbol declaration checking (for identifiers)

#### Future Validation

```cpp
class SemanticAnalyzer {
    void analyze(const NodeProg* prog) {
        // Type checking
        // Variable scope validation
        // Dead code detection
        // Division by zero detection (static analysis)
        // Stack overflow detection
    }
};
```

### Extending the Language

#### Adding New Operators

To add a new operator (e.g., modulo `%`):

1. **Tokenizer**: Add token type
   ```cpp
   enum class TokenType { ..., mod };
   ```

2. **Parser**: Add to expression parser
   ```cpp
   struct NodeBinExprMod {
       NodeExpr *lhs, *rhs;
   };
   ```

3. **Generator**: Add code generation
   ```cpp
   void gen_mod() {
       // mov rax, [rsp + 8]    ; dividend
       // mov rbx, [rsp]        ; divisor
       // xor rdx, rdx          ; clear rdx
       // div rbx               ; rax=quotient, rdx=remainder
       // mov rax, rdx          ; move remainder to rax
   }
   ```

#### Adding New Statement Types

Example: `for` loop

1. **Tokenizer + Parser**: Add `for` keyword and grammar
2. **AST**: Create `NodeStmtFor` node
3. **Generator**: Emit loop labels and conditional jumps

```cpp
struct NodeStmtFor {
    std::string var;        // Loop variable
    NodeExpr* start;        // Initial value
    NodeExpr* end;          // Final value
    std::vector<NodeStmt*> body;
};
```

---

## Examples

### Example 1: Simple Exit with Constant

```hydrogen
exit(42);
```

**Generated Assembly**:
```nasm
section .text
    global _start

_start:
    mov rax, 42
    push rax
    mov rax, [rsp]
    pop rcx
    mov rdi, rax
    mov rax, 60
    syscall
```

**Execution**:
```bash
$ ./build/hydro example1.hy
$ ./out
$ echo $?
42
```

### Example 2: Variable Assignment and Arithmetic

```hydrogen
let x = 100;
let y = 25;
exit(x - y);
```

**Generated Assembly**:
```nasm
section .text
    global _start

_start:
    mov rax, 100
    push rax                ; x at [rsp+0]
    
    mov rax, 25
    push rax                ; y at [rsp+0], x at [rsp+8]
    
    mov rax, [rsp + 8]      ; Load x
    push rax
    mov rax, [rsp + 8]      ; Load y
    mov rax, [rsp]
    pop rbx
    pop rax
    sub rax, rbx            ; x - y
    push rax
    
    mov rax, [rsp]
    mov rdi, rax
    mov rax, 60
    syscall
```

**Execution**:
```bash
$ ./build/hydro example2.hy
$ ./out
$ echo $?
75
```

### Example 3: Operator Precedence

```hydrogen
let result = 2 + 3 * 4 - 1;
exit(result);
```

**AST**:
```
NodeBinExprSub
├── lhs: NodeBinExprAdd
│   ├── lhs: NodeTermIntLit{2}
│   └── rhs: NodeBinExprMul
│       ├── lhs: NodeTermIntLit{3}
│       └── rhs: NodeTermIntLit{4}
└── rhs: NodeTermIntLit{1}
```

**Expected Result**: (2 + (3 * 4)) - 1 = (2 + 12) - 1 = 13

```bash
$ ./build/hydro example3.hy
$ ./out
$ echo $?
13
```

### Example 4: Conditional Branching (if/else)

```hydrogen
let age = 18;
if (age) {
    exit(1);
} else {
    exit(0);
}
```

**Generated Assembly (Simplified)**:
```nasm
mov rax, 18
push rax                ; age on stack
mov rax, [rsp]          ; Load condition
pop rcx
cmp rax, 0
je else_begin
; then block
mov rax, 1
jmp if_end
else_begin:
; else block
mov rax, 0
if_end:
mov rdi, rax
mov rax, 60
syscall
```

**Execution**:
```bash
$ ./build/hydro example4.hy
$ ./out
$ echo $?
1
```

---

## Troubleshooting

### Compilation Issues

#### Problem: "CMake not found"

**Symptoms**:
```
bash: cmake: command not found
```

**Solution**:
```bash
# Ubuntu/Debian
sudo apt-get install cmake

# macOS
brew install cmake

# Fedora/RHEL
sudo dnf install cmake
```

#### Problem: "C++20 compiler not found"

**Symptoms**:
```
error: -std=c++20 is not supported by this compiler
```

**Solution**:
```bash
# Update GCC
sudo apt-get install g++-11

# Or install Clang
sudo apt-get install clang-14

# Verify version
g++ --version
clang++ --version
```

#### Problem: "NASM not found"

**Symptoms**:
```
error: nasm: command not found
```

**Solution**:
```bash
# Ubuntu/Debian
sudo apt-get install nasm

# macOS
brew install nasm

# Build from source
wget https://www.nasm.us/nasm-2.x.tar.gz
tar -xzf nasm-2.x.tar.gz
cd nasm-*
./configure
make
sudo make install
```

### Runtime Issues

#### Problem: "Failed to open file"

**Symptoms**:
```
Failed to open file: myprogram.hy
```

**Causes & Solutions**:

| Cause | Solution |
|-------|----------|
| File doesn't exist | Verify filename: `ls -la myprogram.hy` |
| Wrong path | Use absolute path: `./build/hydro /full/path/myprogram.hy` |
| Permission denied | Grant read permission: `chmod 644 myprogram.hy` |

#### Problem: "Invalid program"

**Symptoms**:
```
Invalid program
```

**Causes & Solutions**:

1. **Syntax error in source**:
   ```hydrogen
   // ❌ Missing semicolon
   let x = 42
   exit(x)
   
   // ✓ Correct
   let x = 42;
   exit(x);
   ```

2. **Invalid expression**:
   ```hydrogen
   // ❌ Empty expression
   exit();
   
   // ✓ Correct
   exit(0);
   ```

3. **Unmatched braces**:
   ```hydrogen
   // ❌ Missing closing brace
   if (x) {
       exit(1)
   
   // ✓ Correct
   if (x) {
       exit(1);
   }
   ```

#### Problem: "Undeclared identifier"

**Symptoms**:
```
Undeclared identifier: variable_name
```

**Cause**: Using a variable that was never declared with `let`

**Solution**:
```hydrogen
// ❌ Wrong
exit(x);

// ✓ Correct
let x = 42;
exit(x);
```

#### Problem: "nasm failed"

**Symptoms**:
```
nasm failed
```

**Causes**:

1. **Invalid assembly generated** (compiler bug)
2. **NASM format issue**: Verify with: `cat out.asm`
3. **NASM not installed**: See "NASM not found" above

**Debug**:
```bash
# Manually assemble to see error
nasm -felf64 out.asm -o out.o
```

#### Problem: "ld failed"

**Symptoms**:
```
ld failed
```

**Causes**:

1. **Object file corrupted**: Verify: `file out.o`
2. **Linker not installed**: Install binutils: `apt-get install binutils`
3. **System architecture mismatch**: Ensure 64-bit system: `uname -m`

### Performance Issues

#### Issue: Slow Compilation

**Potential Causes & Solutions**:

| Cause | Debug | Solution |
|-------|-------|----------|
| Large input file | `wc -l input.hy` | Split into smaller files |
| Slow disk I/O | `iostat` | Use SSD or check disk health |
| Insufficient RAM | `free -h` | Close other applications |
| Unoptimized build | `file build/hydro` | Build in Release mode |

**Build in Release mode for speed**:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

#### Issue: Large Output Binary

**Binary Size Optimization**:

```bash
# Current size
ls -lh out

# Strip debug symbols
strip out
ls -lh out

# Alternative: Build with optimization flags
CMAKE_CXX_FLAGS_RELEASE="-O3 -march=native"
```

### Debugging

#### Enable Verbose Output

```bash
# Verbose CMake configuration
cmake -S . -B build --verbose

# Show compiler commands
cmake --build build -- VERBOSE=1

# See all generated assembly
cat out.asm | less
```

#### Manual Assembly Inspection

```bash
# Disassemble the generated binary
objdump -d out

# Check for specific instructions
nm out | grep -E "start|main"

# Hexdump of binary
hexdump -C out | head -20
```

#### AST Debugging (if available)

Modify the Generator to print AST:

```cpp
void debug_print_ast(const NodeProg* prog) {
    for (const auto& stmt : prog->statements) {
        std::cerr << "Statement: " << (int)stmt->var.index() << "\n";
        // Print node details
    }
}
```

---

## Contributing & Extensions

### Future Language Features

Potential extensions to consider:

1. **Function Declarations**
   ```hydrogen
   fn add(a, b) {
       exit(a + b);
   }
   ```

2. **Loops**
   ```hydrogen
   for (let i = 0; i < 10; i = i + 1) {
       exit(i);
   }
   ```

3. **Arrays**
   ```hydrogen
   let arr = [1, 2, 3];
   exit(arr[0]);
   ```

4. **String Literals**
   ```hydrogen
   let msg = "Hello, World!";
   ```

5. **Comments**
   ```hydrogen
   // This is a comment
   let x = 42;  /* block comment */
   ```

### Development Workflow

```bash
# Create feature branch
git checkout -b feature/new-feature

# Make changes
# Test thoroughly
./build/hydro test_cases/*.hy

# Run validation
cmake --build build -- test

# Commit and push
git add .
git commit -m "Add new feature"
git push origin feature/new-feature
```

---

## References & Resources

### Academic Resources

- **Compiler Design**: "Compilers: Principles, Techniques, and Tools" (Dragon Book)
- **x86_64 ISA**: Intel 64 and IA-32 Architectures Software Developer Manuals
- **System V ABI**: AMD64 Application Binary Interface Specification
- **ELF Format**: Tool Interface Standard (TIS) ELF Specification

### External Tools Documentation

| Tool | Resource | Purpose |
|------|----------|---------|
| NASM | [nasm.us](https://www.nasm.us/) | x86/x64 Assembler |
| GNU ld | [GNU Linker Docs](https://sourceware.org/binutils/docs/ld/) | Object file linking |
| CMake | [CMake Documentation](https://cmake.org/documentation/) | Build system |

### Online Compilers & Tools

- **Compiler Explorer**: https://godbolt.org/ (view generated assembly)
- **Online Assembler**: https://www.asm80.com/ (test x86 code)
- **Binary Analysis**: https://www.onlinedisassembler.com/ (reverse engineering)

---

## License & Attribution

This project is provided as an educational resource for learning compiler design and x86_64 assembly programming.

---

## Frequently Asked Questions (FAQ)

### Q: Can Hydrogen compile complex programs?

**A**: Currently, Hydrogen supports a minimal grammar. For complex logic, you can:
- Chain multiple variable declarations with expressions
- Nest binary operations with proper precedence
- Use if/else for conditional logic

Future versions will support functions and loops.

### Q: How do I debug a compilation error?

**A**: 

1. Check the error message carefully for the type (tokenization, parsing, codegen)
2. Inspect `out.asm` to see the generated assembly
3. Manually trace through the parser logic for your input
4. Add debug print statements to the compiler code if needed

### Q: Why does the compiler generate so much assembly code?

**A**: The current code generator is simple and doesn't optimize:
- Every value goes through the stack
- No register allocation optimization
- No dead code elimination

These can be added as performance enhancements.

### Q: Can I use Hydrogen to compile other languages?

**A**: The compiler is specifically designed for the Hydrogen language syntax. To support other languages, you would need to rewrite the tokenizer and parser for that language's grammar.

### Q: What's the maximum program size?

**A**: Theoretically unlimited, limited only by:
- Available system memory (for parsing AST)
- Disk space (for generated assembly/binary)
- Stack size (for deeply nested expressions)

---

## Changelog

### Version 1.0 (Current)

**Features**:
- Lexical analysis (tokenization)
- Syntax analysis (parsing with precedence climbing)
- x86_64 code generation
- Variable declarations with `let`
- Arithmetic expressions (+, -, *, /)
- Exit codes with `exit()`
- Conditional branching (if/else)

**Limitations**:
- No functions or procedures
- No loops
- Single data type (i64)
- No arrays or complex data structures
- No I/O beyond exit codes

**Known Issues**:
- Minimal error recovery (reports first error)
- No warnings for common mistakes
- Limited optimization

---

## Contact & Support

For issues, questions, or contributions:
- **Report Bugs**: Check existing issues, then create detailed report
- **Feature Requests**: Describe limitation and proposed solution
- **Documentation**: Submit corrections or improvements
- **Testing**: Report edge cases found

---

## Code Architecture Diagram

```
┌──────────────────────────────────────────────────────────┐
│                    main.cpp                              │
│  - Argument parsing                                      │
│  - File I/O (read source)                               │
│  - Orchestrate compilation stages                       │
│  - Invoke NASM & ld                                      │
└────────────┬────────────────────────────────────────────┘
             │
             │ Raw source string
             ▼
┌──────────────────────────────────────────────────────────┐
│                  Tokenizer                               │
│  tokenization.hpp                                        │
│  - Character scanning with lookahead                    │
│  - Keyword recognition                                 │
│  - Token stream generation                             │
└────────────┬────────────────────────────────────────────┘
             │
             │ Token vector
             ▼
┌──────────────────────────────────────────────────────────┐
│                    Parser                                │
│  parser.hpp                                              │
│  - Recursive descent parsing                            │
│  - Precedence climbing for operators                    │
│  - AST construction                                      │
│  - Error detection                                       │
└────────────┬────────────────────────────────────────────┘
             │
             │ NodeProg (AST)
             ▼
┌──────────────────────────────────────────────────────────┐
│                   Generator                              │
│  generation.hpp                                          │
│  - AST traversal (visitor pattern)                      │
│  - Code generation (x86_64 mnemonics)                   │
│  - Stack frame management                               │
│  - Scope tracking                                        │
└────────────┬────────────────────────────────────────────┘
             │
             │ x86_64 assembly (NASM syntax)
             ▼
         out.asm
             │
             │ (External: nasm command)
             ▼
         out.o (object file)
             │
             │ (External: ld command)
             ▼
         out (executable binary)
```

---

**Happy Compiling! 🔧**

For detailed examples and the latest information, visit the project repository or consult the component source files directly.