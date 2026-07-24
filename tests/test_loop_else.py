# Test for/else and while/else

def test_while_else_no_break():
    print("while/else without break:")
    i = 0
    while i < 3:
        print("  i =", i)
        i = i + 1
    else:
        print("  else: loop completed")

def test_while_else_with_break():
    print("while/else with break:")
    i = 0
    while i < 5:
        print("  i =", i)
        if i == 2:
            print("  breaking")
            break
        i = i + 1
    else:
        print("  else should NOT print")

def test_for_else_no_break():
    print("for/else without break:")
    for x in [1, 2, 3]:
        print("  x =", x)
    else:
        print("  else: loop completed")

def test_for_else_with_break():
    print("for/else with break:")
    for x in [1, 2, 3, 4, 5]:
        print("  x =", x)
        if x == 3:
            print("  breaking")
            break
    else:
        print("  else should NOT print")

def test_while_else_never_runs():
    print("while/else with false condition:")
    while False:
        print("  body should NOT print")
    else:
        print("  else: loop never ran but else still executes")

def test_for_else_empty():
    print("for/else empty list:")
    for x in []:
        print("  body should NOT print")
    else:
        print("  else: empty iterable")

def main():
    test_while_else_no_break()
    test_while_else_with_break()
    test_for_else_no_break()
    test_for_else_with_break()
    test_while_else_never_runs()
    test_for_else_empty()

main()
