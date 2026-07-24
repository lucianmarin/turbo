print("=== tuple() tests ===")
lst = [1, 2, 3]
t = tuple(lst)
print("tuple from list:", t)
print("len:", len(t))
print("first:", t[0])

t2 = tuple("abc")
print("tuple from str:", t2)

s = {1, 2, 3}
t3 = tuple(s)
print("tuple from set:", t3)

t4 = tuple((4, 5, 6))
print("tuple from tuple:", t4)

print("=== set() tests ===")
s1 = set([1, 2, 3, 2, 1])
print("set from list:", s1)
print("len:", len(s1))

s2 = set("abracadabra")
print("set from str:", s2)

s3 = set((1, 2, 3))
print("set from tuple:", s3)

s4 = set({4, 5, 6})
print("set from set:", s4)

print("=== iter/next tests ===")
it = iter([10, 20, 30])
print("next:", next(it))
print("next:", next(it))
print("next:", next(it))
print("next on empty:", next(it, "default"))

it2 = iter("xyz")
print("next from str iter:", next(it2))
print("next from str iter:", next(it2))
print("next from str iter:", next(it2))
print("next from empty:", next(it2, "done"))

it3 = iter({"a": 1, "b": 2})
print("next from dict:", next(it3))
print("next from dict:", next(it3))
print("next from empty:", next(it3, "none"))
