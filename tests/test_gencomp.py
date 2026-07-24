def test_basic():
    r = (x * 2 for x in [1, 2, 3, 4, 5])
    print("basic:", r)
    assert r == [2, 4, 6, 8, 10]

def test_with_if():
    r = (x for x in [1, 2, 3, 4, 5] if x > 2)
    print("with_if:", r)
    assert r == [3, 4, 5]

def test_with_if_else():
    r = (x * 10 if x > 3 else x for x in [1, 2, 3, 4, 5])
    print("with_if_else:", r)
    assert r == [1, 2, 3, 40, 50]

def test_range():
    r = (x ** 2 for x in range(5))
    print("range:", r)
    assert r == [0, 1, 4, 9, 16]

def test_string():
    r = (c for c in "hello")
    print("string:", r)
    assert r == ["h", "e", "l", "l", "o"]

def test_empty():
    r = (x for x in [])
    print("empty:", r)
    assert r == []

def test_tuple_iter():
    r = (x * 2 for x in (10, 20, 30))
    print("tuple_iter:", r)
    assert r == [20, 40, 60]

def test_nested_for():
    r = (x + y for x in [1, 2, 3] for y in [10, 20])
    print("nested_for:", r)
    assert r == [11, 21, 12, 22, 13, 23]

def test_nested_with_if():
    r = (x + y for x in [1, 2, 3] if x > 1 for y in [10, 20] if y < 20)
    print("nested_with_if:", r)
    assert r == [12, 13]

def test_double_filter():
    r = (x for x in [1, 2, 3, 4, 5, 6] if x > 2 if x < 5)
    print("double_filter:", r)
    assert r == [3, 4]

def main():
    test_basic()
    test_with_if()
    test_with_if_else()
    test_range()
    test_string()
    test_empty()
    test_tuple_iter()
    test_nested_for()
    test_nested_with_if()
    test_double_filter()
    print("ALL PASSED")

main()