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

- [ ] `try` / `except` / `else` / `finally` — parsed but exception handling not implemented; handlers/else/finally blocks are silently dropped in codegen
- [ ] `raise` — parsed, codegen emits `fprintf(stderr, "Error\n"); exit(1)` — hard abort, not a catchable exception
- [ ] `assert` — parsed, codegen checks truthiness and calls `exit(1)` on failure; assertion message is silently dropped
- [ ] `with` — parsed, codegen assigns context manager to variable and wraps body in bare block; no `__enter__` / `__exit__` calls
- [ ] `async` / `await` — parsed but codegen produces normal sync def/for/with
- [ ] `match` / `case` — parsed as stub, pattern matching not implemented
- [ ] `for` / `else`, `while` / `else` — else clauses on loops
- [ ] `del` — parsed, codegen sets variable to `turbo_none` or subscript to None; does not actually delete
- [ ] `global` / `nonlocal` — parsed, codegen emits C comments only; no effect on scoping
- [ ] `yield` / `yield from` — parsed but codegen is a stub (returns `turbo_none`)
- [ ] `lambda` — parsed but codegen is a stub (returns `turbo_none`)
- [ ] `@decorator` — parsed, decorator AST attached to def/class, but never applied in codegen
- [x] Ternary conditional: `x if cond else y`
- [x] Walrus operator: `:=`

## Operators

- [x] `**` — exponentiation
- [x] `//` — floor division
- [x] Bitwise: `&`, `|`, `^`, `~`, `<<`, `>>`
- [ ] `@` — matrix multiplication (runtime stub: prints NotImplementedError and exits)
- [x] Augmented assignments: `*=`, `/=`, `%=`, `//=`, `**=`, `&=`, `|=`, `^=`, `>>=`, `<<=`

## Comprehensions

- [ ] List comprehensions: `[x for x in y]`
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
