class Base:
    def greet(self):
        return "Base"

class Child(Base):
    def greet(self):
        sup = super(Child, self)
        return sup.greet()

c = Child()
print("super method call:", c.greet())
