# Test try/except/else/finally

class MyError:
    def __init__(self, msg):
        self.msg = msg

class OtherError:
    def __init__(self, msg):
        self.msg = msg

def test1_basic():
    print("Test 1: basic try/except")
    x = 0
    try:
        raise "error"
    except:
        x = 1
    print("x =", x)

def test2_no_exception():
    print("Test 2: no exception in try")
    x = 0
    try:
        x = 1
    except:
        x = 2
    print("x =", x)

def test3_else():
    print("Test 3: else block (no exception)")
    x = 0
    try:
        x = 1
    except:
        x = 2
    else:
        x = 3
    print("x =", x)

def test4_finally_no_exc():
    print("Test 4: finally (no exception)")
    x = 0
    try:
        x = 1
    finally:
        x = 2
    print("x =", x)

def test5_finally_with_exc():
    print("Test 5: finally after exception")
    x = 0
    try:
        raise "err"
    except:
        x = 1
    finally:
        x = 2
    print("x =", x)

def test6_class_exception():
    print("Test 6: class-based exception")
    try:
        raise MyError("bad things")
    except:
        print("caught MyError")

def test7_except_matching():
    print("Test 7: except type matching")
    try:
        raise OtherError("other")
    except MyError:
        print("wrong - should not match")
    except OtherError:
        print("matched OtherError correctly")

def test8_except_as():
    print("Test 8: except as var")
    try:
        raise MyError("hello from error")
    except MyError as e:
        print("e.msg =", e.msg)

print("--- Starting tests ---")
test1_basic()
test2_no_exception()
test3_else()
test4_finally_no_exc()
test5_finally_with_exc()
test6_class_exception()
test7_except_matching()
test8_except_as()
print("--- All tests done ---")
