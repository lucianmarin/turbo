x = 10

def foo():
    global x
    print(x)
    x = 20

print("before:", x)
foo()
print("after:", x)

y = 100

def bar():
    global y
    y = y + 1

print("y before:", y)
bar()
print("y after:", y)

count = 0
results = []

def increment():
    global count
    count = count + 1
    results.append(count)

increment()
increment()
increment()
print("count:", count)
print("results:", results)
