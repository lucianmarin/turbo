# Turbo Test Program

def test_arithmetic():
    x = 10
    y = 3
    print("Arithmetic:")
    print("x + y =", x + y)
    print("x - y =", x - y)
    print("x * y =", x * y)
    print("x / y =", x / y)
    print("x % y =", x % y)

def test_comparisons():
    print("Comparisons:")
    x = 5
    y = 10
    print("x < y:", x < y)
    print("x > y:", x > y)
    print("x == y:", x == y)
    print("x != y:", x != y)

def test_lists():
    print("Lists:")
    lst = [1, 2]
    lst.append(3)
    lst.append(4)
    print("lst:", lst)
    print("len(lst):", len(lst))
    print("lst[0]:", lst[0])
    print("lst[-1]:", lst[-1])
    
    # Slice test
    print("lst[1:3]:", lst[1:3])

def test_strings():
    print("Strings:")
    s = "hello" + " world"
    print("s:", s)
    print("len(s):", len(s))
    print("s[0]:", s[0])
    print("s[6:11]:", s[6:11])
    
    # Split test
    parts = s.split()
    print("split parts:", parts)

def test_loops():
    print("Loops:")
    i = 0
    while i < 3:
        print("while loop count:", i)
        i = i + 1

    for j in range(3):
        print("for j in range(3):", j)

    for item in ["apple", "banana"]:
        print("for item in list:", item)

class Greeter:
    def __init__(self, prefix):
        self.prefix = prefix
    
    def greet(self, target):
        print(self.prefix + ", " + target + "!")

def test_classes():
    print("Classes:")
    g = Greeter("Hello")
    g.greet("Turbo")
    print("g.prefix attribute access:", g.prefix)

def test_bigint():
    print("BigInts:")
    a = 123456789012345678901234567890
    b = 987654321098765432109876543210
    print("a + b =", a + b)
    print("b - a =", b - a)
    print("a * b =", a * b)
    c = 2 ** 100
    print("2 ** 100 =", c)
    print("c - 1 =", c - 1)
    print("-c =", -c)
    d = 1000000000000000000000000000000
    print("d // 3 =", d // 3)
    print("d % 3 =", d % 3)
    e = -123456789012345678901234567890
    print("e =", e)
    print("abs(e) =", abs(e))
    print("e + 10 =", e + 10)
    print("a > b:", a > b)
    print("a < b:", a < b)
    print("a == a:", a == a)
    print("a != b:", a != b)
    print("e < 0:", e < 0)
    print("e > 0:", e > 0)

def main():
    test_arithmetic()
    test_comparisons()
    test_lists()
    test_strings()
    test_loops()
    test_classes()
    test_bigint()

main()
