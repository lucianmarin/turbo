name = "Turbo"
version = 42
msg = f"hello {name}, version {version}"
print("fstr1:", msg)

x = 10
y = 20
result = f"{x} + {y} = {x + y}"
print("fstr2:", result)

val = 3.14
print(f"pi is {val}")

escaped = f"{{curly}} and {name}"
print("fstr3:", escaped)

nested = f"items: {[1, 2, 3]}"
print("fstr4:", nested)
