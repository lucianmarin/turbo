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

| Benchmark | Python 3 (ms) | Turbo LLVM IR (ms) | C `clang -O2` (ms) | Turbo vs Python & C |
|---|---:|---:|---:|---:|
| `fib_iterative` | 5,565.35 | **3.71** | 11.59 | **1,500.1x** (3.12x C) |
| `fib_recursive` | 5,120.07 | **169.68** | 168.99 | **30.2x** (1.00x C) |
| `ackermann` | 1,471.14 | **4.19** | 13.52 | **351.1x** (3.23x C) |
| `pi_wallis` | 261.66 | **3.02** | 7.35 | **86.6x** (2.43x C) |
| `leibniz` | 41,044.80 | **580.97** | 544.48 | **70.6x** (0.94x C) |
| `matmult` | 5,333.91 | **22.01** | 6.68* / 111.42 | **242.3x** (5.06x C) |
| `sieve` | 136.83 | **8.63** | 13.24 | **15.9x** (1.53x C) |
| `collatz` | 5,106.73 | **103.76** | 102.84 | **49.2x** (0.99x C) |
| `set_ops` | 70.14 | **42.78** | 28.76 | **1.6x** (0.67x C) |
| `dict_manip` | 50.81 | **13.98** | 12.77 | **3.6x** (0.91x C) |
| `tree_structures` | 143.86 | **78.30** | 13.83 | **1.8x** (0.18x C) |
| `class_hierarchy` | 53.27 | **12.04** | 8.51 | **4.4x** (0.71x C) |

*\*Note: `matmult.c` frees `C` without reading it, so `clang -O2` dead-code eliminates the multiplication loop (6.68 ms); with a volatile sink, C 512×512 matmult takes 111.42 ms vs Turbo's 22.01 ms (5.06x faster due to loop-interchanged reduction).*
