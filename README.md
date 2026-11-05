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
| `fib_iterative` | 10,769.59 | **5.74** | 4.97 | **1,876.4x** (0.87x C) |
| `fib_recursive` | 9,974.44 | **326.98** | 323.33 | **30.5x** (0.99x C) |
| `ackermann` | 2,849.88 | **6.69** | 5.69 | **425.9x** (0.85x C) |
| `pi_wallis` | 510.96 | **5.99** | 5.03 | **85.3x** (0.84x C) |
| `leibniz` | 84,013.21 | **1,136.95** | 1,106.99 | **73.9x** (0.97x C) |
| `matmult` | 10,586.24 | **41.32** | 5.02* / 215.84 | **256.2x** (5.22x C) |
| `sieve` | 270.92 | **14.35** | 16.12 | **18.9x** (1.12x C) |
| `collatz` | 10,065.62 | **204.12** | 203.25 | **49.3x** (1.00x C) |
| `set_ops` | 130.40 | **80.31** | 52.91 | **1.6x** (0.66x C) |
| `dict_manip` | 95.94 | **247.97** | 122.87 | **0.4x** (0.50x C) |
| `tree_structures` | 277.06 | **351.64** | 15.99 | **0.8x** (0.05x C) |
| `class_hierarchy` | 100.43 | **49.38** | 6.09 | **2.0x** (0.12x C) |

*\*Note: `matmult.c` frees `C` without reading it, so `clang -O2` dead-code eliminates the multiplication loop (5.02 ms); with a volatile sink, C 512×512 matmult takes 215.84 ms vs Turbo's 41.32 ms (5.22x faster due to loop-interchanged reduction).*
