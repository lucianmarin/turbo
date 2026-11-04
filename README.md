# 🏎️ Turbo: Python Syntax, Native Performance

Turbo is an experimental compiled programming language that matches standard Python syntax 1-to-1 but compiles down to native machine code via a lightweight C runtime.

---

## ⚡ The Philosophy: Developer Speed Meets Execution Speed

Traditionally, developers are forced to make a trade-off:
* **Write fast code** (using C, C++, Rust, or Go), which incurs high cognitive overhead, verbose syntax, complex type systems, and slow initial prototyping.
* **Write code fast** (using Python, JS, or Ruby), which allows rapid iteration and high productivity, but incurs slow interpreter start-up times, massive memory footprints, and poor performance in CPU-bound tasks.

**Turbo eliminates this compromise.** By pairing Python's highly readable, minimal-boilerplate syntax with a native ahead-of-time compiler, Turbo enables a turbocharged development loop.

### 1. Zero-Cost Ergonomics
Python’s syntax is clean, expressive, and relies on indentation rather than braces, reducing visual clutter. Developers spend less time writing boilerplate declarations and more time implementing logic. In Turbo, you write code that looks exactly like standard Python—making it instantly familiar to millions of developers—but get the compilation benefits of low-level languages.

### 2. High-Performance Execution
Unlike interpreted Python, which runs on a Virtual Machine (CPython) and suffers from interpreter loop overhead:
* **Native Compilation**: Turbo translates your code into optimized C code, which is compiled directly to machine instructions by compilers like `gcc` or `clang`.
* **Zero Dependency Binaries**: The compiled program is a single, self-contained native executable. There is no need to bundle a Python interpreter, a virtual environment, or external runtime libraries.
* **Instant Startup**: The program executes instantly without VM initialization overhead, making it ideal for CLI tools, system utilities, and performance-critical microservices.

### 3. Fully Self-Hosting
To prove its viability, Turbo compiles itself. The compiler (`turbo.py`) is written in standard Python/Turbo syntax and is fully self-hosting: it compiles its own codebase into a native compiler binary which can then be used to compile itself again recursively.

---

## 🚀 Quick Start

### Prerequisites
* A C compiler (e.g., `gcc` or `clang` installed on your system).
* Python 3 (only used for Stage 0 bootstrapping).

### Run the Bootstrap Verification Pipeline
To clean up previous builds, run test scripts, compile the Turbo compiler, bootstrap it, and check for C-output equivalence:

```bash
chmod +x build.sh
./build.sh
```

### Compile Your First Turbo Script
Write a python file, say `hello.py`:
```python
print("Hello from Turbo!")
```

Compile it to C using the compiled Turbo binary:
```bash
./stage2_turbo hello.py -o hello.c
```

Compile the output C file to a native executable with the runtime:
```bash
gcc hello.c runtime.c -o hello
./hello
```

---

## 📊 Performance: Turbo vs Python vs C

Benchmark suite (`perf/`) comparing Python 3, Turbo (LLVM IR), and hand-written C (`clang -O2`).

| Benchmark | Python 3 (ms) | Turbo LLVM IR (ms) | C `clang -O2` (ms) | Turbo vs Python |
|---|---:|---:|---:|---:|
| `fib_iterative` | 10,658.46 | **7.28** | 9.03 | **1,464.7x** |
| `fib_recursive` | 10,188.20 | **323.49** | 327.84 | **31.5x** (1.00x C) |
| `ackermann` | 3,513.94 | **10.38** | 10.39 | **338.7x** (1.00x C) |
| `pi_wallis` | 531.06 | **8.67** | 9.28 | **61.2x** (1.00x C) |
| `leibniz` | 81,281.36 | **1,131.65** | 1,103.76 | **71.8x** (1.02x C) |
| `matmult` | 10,506.18 | **520.35** | 7.67* / 211.21 | **20.2x** |
| `sieve` | 300.48 | **86.47** | 17.61 | **3.5x** |
| `collatz` | 11,316.72 | **3,042.91** | 205.10 | **3.7x** |

*\*Note: `matmult.c` frees `C` without reading it, so `clang -O2` dead-code eliminates the multiplication loop (7.67 ms); with a volatile sink, C 512×512 matmult takes ~211 ms vs Turbo's 520 ms.*
