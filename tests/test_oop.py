# OOP Test Suite

def assert_eq(a, b):
    if a != b:
        print("FAIL: expected")
        print(b)
        print("got")
        print(a)
        assert False

# Test 1: __str__
class WithStr:
    def __init__(self, val):
        self.val = val
    def __str__(self):
        return "WithStr(" + self.val + ")"

def test_str():
    print("Test __str__:")
    obj = WithStr("hello")
    s = str(obj)
    print("  str(obj):", s)
    assert_eq(s, "WithStr(hello)")
    print("  OK")

# Test 2: __repr__
class WithRepr:
    def __init__(self, val):
        self.val = val
    def __repr__(self):
        return "WithRepr(" + self.val + ")"

def test_repr():
    print("Test __repr__:")
    obj = WithRepr("world")
    r = repr(obj)
    print("  repr(obj):", r)
    assert_eq(r, "WithRepr(world)")
    print("  OK")

# Test 3: @staticmethod
class MathUtils:
    @staticmethod
    def add(a, b):
        return a + b
    @staticmethod
    def mul(a, b):
        return a * b

def test_staticmethod():
    print("Test @staticmethod:")
    r1 = MathUtils.add(10, 20)
    print("  MathUtils.add(10, 20):", r1)
    assert_eq(r1, 30)
    r2 = MathUtils.mul(3, 4)
    print("  MathUtils.mul(3, 4):", r2)
    assert_eq(r2, 12)
    inst = MathUtils()
    r3 = inst.add(5, 6)
    print("  inst.add(5, 6):", r3)
    assert_eq(r3, 11)
    print("  OK")

# Test 4: @classmethod
class Counter:
    count = 0
    @classmethod
    def get_count(cls):
        return cls.count
    @classmethod
    def inc(cls):
        cls.count = cls.count + 1

def test_classmethod():
    print("Test @classmethod:")
    c1 = Counter()
    c2 = Counter()
    print("  initial count:", Counter.get_count())
    assert_eq(Counter.get_count(), 0)
    Counter.inc()
    print("  after inc:", Counter.get_count())
    assert_eq(Counter.get_count(), 1)
    c1.inc()
    print("  after c1.inc:", Counter.get_count())
    assert_eq(Counter.get_count(), 2)
    print("  OK")

# Test 5: @property
class Rectangle:
    def __init__(self, w, h):
        self._w = w
        self._h = h
    @property
    def area(self):
        return self._w * self._h
    @property
    def perimeter(self):
        return 2 * (self._w + self._h)

def test_property():
    print("Test @property:")
    r = Rectangle(3, 4)
    a = r.area
    print("  r.area:", a)
    assert_eq(a, 12)
    p = r.perimeter
    print("  r.perimeter:", p)
    assert_eq(p, 14)
    r2 = Rectangle(10, 5)
    print("  r2.area:", r2.area)
    assert_eq(r2.area, 50)
    print("  OK")

# Test 6: @dataclass
@dataclass
class Point:
    x: 0
    y: 0

@dataclass
class Person:
    name: ""
    age: 0

def test_dataclass():
    print("Test @dataclass:")
    p = Point(3, 4)
    print("  p.x:", p.x, "p.y:", p.y)
    assert_eq(p.x, 3)
    assert_eq(p.y, 4)
    p2 = Point()
    print("  p2.x:", p2.x, "p2.y:", p2.y)
    assert_eq(p2.x, 0)
    assert_eq(p2.y, 0)
    person = Person("Alice", 30)
    print("  person.name:", person.name, "person.age:", person.age)
    assert_eq(person.name, "Alice")
    assert_eq(person.age, 30)
    print("  OK")

# Test 7: Operator overloading
class Vector:
    def __init__(self, x, y):
        self.x = x
        self.y = y
    def __add__(self, other):
        return Vector(self.x + other.x, self.y + other.y)
    def __eq__(self, other):
        if self.x == other.x and self.y == other.y:
            return True
        return False

def test_operator_overloading():
    print("Test operator overloading:")
    v1 = Vector(1, 2)
    v2 = Vector(3, 4)
    v3 = v1 + v2
    print("  v1 + v2 = Vector(", v3.x, ",", v3.y, ")")
    assert_eq(v3.x, 4)
    assert_eq(v3.y, 6)
    v4 = Vector(1, 2)
    print("  v1 == v4:", v1 == v4)
    assert_eq(v1 == v4, True)
    print("  v1 == v2:", v1 == v2)
    assert_eq(v1 == v2, False)
    print("  OK")

# Test 8: Combined OOP features
class Temperature:
    def __init__(self, celsius):
        self._c = celsius
    @property
    def fahrenheit(self):
        return self._c * 9 / 5 + 32
    @staticmethod
    def from_f(f):
        return Temperature((f - 32) * 5 / 9)
    def __str__(self):
        return str(self._c) + "C"
    def __repr__(self):
        return "Temperature(" + str(self._c) + ")"

def test_combined():
    print("Test combined OOP:")
    t = Temperature(100)
    print("  t:", t)
    assert_eq(str(t), "100C")
    print("  repr(t):", repr(t))
    assert_eq(repr(t), "Temperature(100)")
    print("  t.fahrenheit:", t.fahrenheit)
    assert_eq(t.fahrenheit, 212.0)
    t2 = Temperature.from_f(32)
    print("  from_f(32):", t2)
    assert_eq(str(t2), "0C")
    print("  OK")

def main():
    test_str()
    test_repr()
    test_staticmethod()
    test_classmethod()
    test_property()
    test_dataclass()
    test_operator_overloading()
    test_combined()
    print("=== ALL OOP TESTS PASSED ===")

main()
