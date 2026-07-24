def gen1():
    yield 1
    yield 2

def gen2():
    yield from gen1()
    yield 3

def main():
    result = gen2()
    print("gen2():", result)

main()
