def deco1(func):
    print("deco1")
    return func

def deco2(func):
    print("deco2")
    return func

@deco1
@deco2
def hello():
    print("hello world")

hello()

def class_deco(cls):
    print("class_deco called with:", cls)
    return cls

@class_deco
class MyClass:
    def method(self):
        print("method called")

obj = MyClass()
obj.method()
