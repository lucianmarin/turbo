def test_del_list():
    print("del list:")
    lst = [10, 20, 30, 40]
    print("before:", lst)
    del lst[1]
    print("after del lst[1]:", lst)
    print("len after:", len(lst))

def test_del_list_negative():
    print("del list negative:")
    lst = [1, 2, 3]
    del lst[-1]
    print("after del lst[-1]:", lst)

def test_del_dict():
    print("del dict:")
    d = {"a": 1, "b": 2, "c": 3}
    print("before:", d)
    del d["b"]
    print("after del d['b']:", d)

def test_del_var():
    print("del var:")
    x = 42
    del x
    x = 99
    print("x after reassign:", x)

def main():
    test_del_list()
    test_del_list_negative()
    test_del_dict()
    test_del_var()

main()
