# Test no wildcard (no matching case)
x = 99
match x:
    case 1:
        print("one")
    case 2:
        print("two")

print("after match")

# Test True/False/None literals
v = True
match v:
    case True:
        print("is true")
    case False:
        print("is false")
    case None:
        print("is none")

# Test string match
s = "hello"
match s:
    case "hi":
        print("hi")
    case "hello":
        print("hello there")
    case _:
        print("unknown")
