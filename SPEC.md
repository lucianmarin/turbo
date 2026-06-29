# Turbo Language Specification

Turbo is a compiled programming language with a syntax that matches Python 1-to-1. The Turbo compiler compiles Turbo source code into C code, which is then compiled to native binaries using a C compiler (like `gcc` or `clang`).

This document specifies the Turbo language subset, syntax, C runtime representation, compiler design, and self-hosting bootstrap procedure.

---

## 1. Syntax and Language Subset

Turbo supports a subset of Python syntax. Any valid Turbo program is also a valid Python program (meaning it can be executed directly by the standard Python interpreter).

### 1.1 Types
Turbo supports the following built-in types:
- `int`: Signed 64-bit integer (corresponds to C `long long`).
- `str`: Immutable string (corresponds to a heap-allocated, null-terminated `char*` wrapped in a struct with length).
- `bool`: Boolean values (`True` and `False`).
- `NoneType`: The `None` value.
- `list`: Dynamic array of Turbo values (e.g., `[1, 2, 3]`).
- `dict`: Association list/hash table mapping Turbo values to Turbo values (e.g., `{"a": 1, "b": 2}`).

### 1.2 Statements and Control Flow
Turbo supports:
- **Expression Statements**: Function calls, method calls, etc.
- **Assignments**: `variable = expression`, supporting local and global scoping, and indexing/attribute assignment:
  - `x = 10`
  - `lst[0] = 20`
  - `obj.field = 30`
- **Augmented Assignments**: `+=`, `-=` for integers.
- **Conditional Statements**:
  ```python
  if condition:
      statements
  elif condition:
      statements
  else:
      statements
  ```
- **While Loops**:
  ```python
  while condition:
      statements
  ```
- **For Loops**:
  ```python
  for item in iterable:
      statements
  ```
  Note: `iterable` can be a list, string, or a range (e.g., `range(10)`).
- **Function Definitions**:
  ```python
  def func_name(param1, param2):
      statements
      return value
  ```
- **Class Definitions**:
  ```python
  class ClassName:
      def __init__(self, arg1):
          self.arg1 = arg1
      
      def method(self, x):
          return self.arg1 + x
  ```
  Note: Only simple classes with single/no inheritance, constructor `__init__`, and methods.
- **Pass Statement**: `pass`.

### 1.3 Expressions and Operators
- **Literals**: Integers (e.g. `42`), Strings (e.g. `'hello'`, `"world"`), Booleans (`True`, `False`), `None`, Lists (e.g. `[1, 2]`), Dicts (e.g. `{'key': 'val'}`).
- **Binary Operators**: `+`, `-`, `*`, `/`, `%`
- **Comparison Operators**: `==`, `!=`, `<`, `>`, `<=`, `>=`
- **Logical Operators**: `and`, `or`, `not`
- **Subscripting**: `expr[index]`, `expr[start:end]` (string/list slicing)
- **Attribute Access**: `expr.attribute`
- **Function/Method Calls**: `expr(args...)`
- **Membership**: `item in container`

### 1.4 Comments
Single-line comments starting with `#` are ignored by the lexer.

---

## 2. Built-in Functions and Methods

To support writing a self-hosting compiler, Turbo provides a core set of built-ins:
- `print(*args)`: Prints values to standard output.
- `len(x)`: Returns the length of a string, list, or dict.
- `str(x)`: Converts a value to its string representation.
- `int(x)`: Converts a string or float/bool to an integer.
- `ord(char)`: Returns the integer ASCII value of a character.
- `chr(code)`: Returns a string of one character with the given ASCII code.
- `range(stop)` / `range(start, stop)`: Returns an iterable range of integers.
- `open(filepath, mode)`: Opens a file and returns a file object. Supported modes: `'r'`, `'w'`.
- **File Object Methods**:
  - `f.read()`: Reads the entire file content into a string.
  - `f.write(text)`: Writes string to the file.
  - `f.close()`: Closes the file.
- **List Methods**:
  - `list.append(x)`: Appends an element to the list.
- **String Methods**:
  - `str.split(sep)`: Splits string by separator into a list of strings. (If `sep` is omitted or `None`, splits by whitespace).

---

## 3. C Runtime Architecture

Turbo values are represented dynamically in C as pointers to `TurboObject` structs.

### 3.1 Type Definitions
```c
typedef enum {
    TYPE_NONE,
    TYPE_INT,
    TYPE_BOOL,
    TYPE_STR,
    TYPE_LIST,
    TYPE_DICT,
    TYPE_FUNC,
    TYPE_CLASS,
    TYPE_INSTANCE,
    TYPE_FILE
} TurboType;

typedef struct TurboObject TurboObject;

typedef TurboObject* (*TurboCFunction)(int argc, TurboObject** args);

struct TurboObject {
    TurboType type;
    union {
        long long int_val;
        bool bool_val;
        struct {
            char* chars;
            int length;
        } str_val;
        struct {
            TurboObject** items;
            int length;
            int capacity;
        } list_val;
        struct {
            // Simple association list or hash map
            TurboObject** keys;
            TurboObject** values;
            int length;
            int capacity;
        } dict_val;
        struct {
            TurboCFunction func_ptr;
            char* name;
        } func_val;
        struct {
            char* name;
            // Methods and default attributes
        } class_val;
        struct {
            TurboObject* class_obj;
            // Instance variables stored in association list
            char** keys;
            TurboObject** values;
            int length;
            int capacity;
        } inst_val;
        struct {
            FILE* handle;
        } file_val;
    };
};
```

### 3.2 Memory Management
To keep the bootstrap compiler simple, the initial version will use standard dynamic allocation (`malloc`). Since compiler runs are short-lived, memory is reclaimed by the operating system on exit. This avoids the overhead and complexity of writing a garbage collector in the compiler's target code generator.

---

## 4. Compiler Architecture and Self-Hosting

The compiler is designed to be self-hosting. The compiler codebase (`turbo.py`) is written in the Turbo subset of Python.

### 4.1 Phases of compilation in `turbo.py`:
1. **Lexical Analysis**: Reads the input Turbo source file, manages the indentation stack, and generates a list of tokens.
2. **Parsing**: A recursive descent parser that reads tokens and builds an AST.
3. **Semantic Analysis (Optional/Simple)**: Tracks local vs global variable declarations.
4. **Code Generation**: Traverses the AST and prints corresponding C code.

### 4.2 Bootstrap Flow
1. **Stage 0 (Python)**: We run the `turbo.py` script using the standard Python 3 interpreter to compile `turbo.py` itself:
   ```bash
   python3 turbo.py turbo.py -o stage1_turbo.c
   ```
2. **Stage 1 (C Compilation)**: We compile `stage1_turbo.c` and the C runtime using a standard C compiler (e.g. `clang` or `gcc`):
   ```bash
   clang -O2 stage1_turbo.c runtime.c -o stage1_turbo
   ```
3. **Stage 2 (Self-Compilation)**: We use the compiled `stage1_turbo` binary to compile `turbo.py` again:
   ```bash
   ./stage1_turbo turbo.py -o stage2_turbo.c
   ```
4. **Stage 2 Verification**: We compile `stage2_turbo.c` to a binary and verify that its output is identical to `stage1_turbo` (or that it successfully compiles other programs).
   ```bash
   clang -O2 stage2_turbo.c runtime.c -o stage2_turbo
   diff stage1_turbo.c stage2_turbo.c
   ```
   If the generated C files are identical, the compiler is fully bootstrapped and self-hosting!
