def describe(val):
    match val:
        case 1:
            print("one")
        case 2:
            print("two")
        case "hello":
            print("greeting")
        case _:
            print("something else")

describe(1)
describe(2)
describe("hello")
describe(42)

# Test capture pattern
x = 99
match x:
    case y:
        print(y)

# Test wildcard only
match "anything":
    case _:
        print("wildcard matched")
