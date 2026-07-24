s1 = "hello {}, it's {}!".format("world", 42)
print("format1:", s1)

s2 = "{1} + {0} = {2}".format(10, 20, 30)
print("format2:", s2)

s3 = "value: {}".format(3.14)
print("format3:", s3)

s4 = "{{braces}}".format()
print("format4:", s4)
