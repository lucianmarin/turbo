# Turbo TODO

Features from Python not yet implemented in Turbo.

## Types

- [x] `float` — float literal parsing (`3.14`, `2.5e2`)
- [x] `complex` — imaginary literal parsing (`4j`, `3+4j`)
- [x] `bytes` / `bytearray` — `b"..."` literal parsing
- [x] `tuple` — tuple literal parsing (`(1,)`, `(1, 2)`, `()`)
- [x] `set` / `frozenset` — set literal parsing (`{1, 2}`)
- [x] Arbitrary-precision integers (currently `long long` only, 64-bit signed)

## Control Flow

- [x] `try` / `except` / `else` / `finally` — implemented with setjmp/longjmp; supports bare `except:`, typed `except ExcType:`, `except ExcType as e:`, `else`, and `finally`
- [x] `raise` — raises a catchable exception via `turbo_raise()`; prints "Unhandled exception" and exits if no matching handler
- [x] `assert` — parsed, codegen checks truthiness and calls `exit(1)` on failure; assertion message is evaluated and printed when present
- [x] `with` — parsed, codegen assigns context manager to variable and wraps body in bare block; no `__enter__` / `__exit__` calls
  - [x] calls `__enter__()` and binds result to `as` variable
  - [x] wraps body in try/finally via setjmp/longjmp
  - [x] calls `__exit__(exc_type, exc_val, None)` with proper args
  - [x] re-raises exception if `__exit__` returns false
- [x] `async` / `await` — parsed and codegen produces sync C code; `async def/for/with` are handled; `await` evaluates the expression synchronously
- [x] `match` / `case` — pattern matching implemented via if/elif chain; supports literal, capture, and wildcard (`_`) patterns
- [x] `for` / `else`, `while` / `else` — else clauses on loops
- [x] `del` — removes list elements and dict keys via `turbo_delitem`; variables set to `turbo_none`
- [x] `global` / `nonlocal` — names excluded from local variable declarations; `global` resolves to module-level globals; `nonlocal` prevents local shadowing (full closure support not yet implemented)
- [x] `yield` / `yield from` — implemented via eager list collection: generator functions collect yielded values into a list and return it at the end; `yield from` iterates the sub-iterator and appends each value
- [x] `lambda` — codegen generates a proper C function and returns `make_func(...)`
- [x] `@decorator` — parsed and applied in codegen via `turbo_call`; supports stacking and class decorators
- [x] Ternary conditional: `x if cond else y`
- [x] Walrus operator: `:=`

## Operators

- [x] `**` — exponentiation
- [x] `//` — floor division
- [x] Bitwise: `&`, `|`, `^`, `~`, `<<`, `>>`
- [x] `@` — matrix multiplication on lists of lists with dimension validation
- [x] Augmented assignments: `*=`, `/=`, `%=`, `//=`, `**=`, `&=`, `|=`, `^=`, `>>=`, `<<=`, `@=`

## Comprehensions

- [x] List comprehensions: `[x for x in y]`
- [ ] Dict comprehensions: `{k: v for k, v in x}`
- [ ] Set comprehensions: `{x for x in y}`
- [ ] Generator expressions: `(x for x in y)`

## String Methods

- [x] `.join(iterable)` — join strings with separator
- [x] `.upper()` / `.lower()` — case conversion
- [x] `.replace(old, new)` — substring replacement
- [x] `.find(sub)` / `.index(sub)` — substring search
- [x] `.strip()` / `.lstrip()` / `.rstrip()` — trim whitespace
- [x] `.startswith(prefix)` / `.endswith(suffix)` — prefix/suffix check
- [x] `.count(sub)` — count occurrences
- [ ] `.format(*args)` — string formatting
- [ ] f-strings: `f"hello {name}"` — lexer tokenizes as plain string, no expression interpolation
- [x] Triple-quoted strings: `"""..."""` / `'''...'''`

## List Methods

- [x] `.extend(iterable)` — extend list with elements
- [x] `.insert(index, item)` — insert at position
- [x] `.remove(item)` — remove first occurrence
- [x] `.index(item)` — find first index
- [x] `.count(item)` — count occurrences
- [x] `.sort()` / `.reverse()` — in-place sorting / reversal
- [x] `.copy()` / `.clear()` — shallow copy / clear

## Dict Methods

- [x] `.keys()` — view of keys
- [x] `.values()` — view of values
- [x] `.items()` — view of key-value pairs
- [x] `.get(key, default)` — safe lookup with fallback
- [x] `.pop(key)` — remove and return
- [ ] `.popitem()` — remove and return arbitrary pair
- [x] `.update(other)` — merge another dict
- [x] `.clear()` / `.copy()` — clear / shallow copy
- [x] `.setdefault(key, default)` — insert if missing

## Built-in Functions

- [x] `input(prompt)` — read from stdin
- [x] `type(obj)` — get type name
- [x] `isinstance(obj, type)` — type check
- [x] `hasattr` / `getattr` / `setattr` — attribute introspection
- [x] `repr(obj)` — unambiguous string representation
- [x] `abs(x)` — absolute value
- [x] `round(x)` — round to nearest integer
- [x] `pow(x, y)` — exponentiation
- [x] `hex(x)` / `bin(x)` / `oct(x)` — base conversion
- [x] `float(x)` — convert to float
- [x] `bool(x)` — convert to bool
- [x] `list(x)` / `dict(x)` — type constructors
- [ ] `tuple(x)` / `set(x)` — type constructors (runtime exists, not wired as builtins)
- [ ] `super()` — access parent class (stub, returns None)
- [ ] `eval(expr)` / `exec(code)` — dynamic evaluation
- [ ] `iter(obj)` / `next(obj)` — iterator protocol (`iter` returns list slice copy; `next` returns None stub)
- [x] `all(iterable)` / `any(iterable)` — predicate checks
- [x] `sum(iterable)` / `min(iterable)` / `max(iterable)` — aggregation
- [x] `sorted(iterable)` — return sorted list
- [x] `reversed(iterable)` — return reversed list copy
- [x] `enumerate(iterable)` — indexed iteration
- [x] `zip(*iterables)` — parallel iteration
- [x] `map(func, iterable)` / `filter(func, iterable)` — functional tools

## Object-Oriented Programming

- [ ] `@staticmethod` — static methods
- [ ] `@classmethod` — class methods
- [ ] `@property` — computed attributes
- [ ] `@dataclass` — auto-generated boilerplate
- [ ] `super()` — parent class method resolution (runtime stub only)
- [ ] `__str__` / `__repr__` — string representation protocol
- [ ] Operator overloading: `__add__`, `__eq__`, `__lt__`, etc.
- [ ] `__slots__` — memory-efficient attribute declaration
- [ ] `__new__` / metaclasses — custom instance creation
- [ ] Abstract base classes

## Module System

- [ ] Real `import` — actual module loading (currently a stub)
- [ ] `__init__.py` — package initialization
- [ ] Standard library: `sys`, `os`, `math`, `json`, `re`, `time`, etc.

## Other

- [ ] Type hints: `def foo(x: int) -> str:` — parsed and silently skipped (no effect)
- [ ] `__name__ == "__main__"` idiom
- [ ] Docstrings (triple-quoted expression statements) — lexer parses, not extracted or used
- [ ] Step parameter in slicing: `seq[start:stop:step]`
- [ ] Garbage collector or reference counting (currently malloc-only)
- [ ] Package system / `__init__.py`
