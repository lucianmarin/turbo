x = "module"

def outer():
    x = "outer"
    def inner():
        nonlocal x
        x = "inner"
    inner()
    print("outer x:", x)

outer()
print("module x:", x)
