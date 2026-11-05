# Perf 4: OOP Class Hierarchy, Multiple Inheritance, ABCs, and Dataclasses Benchmark
# Exercises: Abstract Base Classes (ABC, @abstractmethod), diamond multiple inheritance
# with cooperative super() MRO dispatch, @dataclass with type annotations, defaults,
# inheritance and equality (__eq__), @property, @staticmethod, @classmethod,
# operator overloading (__add__), and polymorphic isinstance() dispatch.

from abc import ABC, abstractmethod
from dataclasses import dataclass

# 1. Dataclasses with inheritance, default values, and __eq__
@dataclass
class Vec2D:
    x: int = 0
    y: int = 0

    def norm_sq(self):
        return self.x * self.x + self.y * self.y

    def __add__(self, other):
        return Vec2D(self.x + other.x, self.y + other.y)

@dataclass
class Vec3D(Vec2D):
    z: int = 0

    def norm_sq(self):
        return self.x * self.x + self.y * self.y + self.z * self.z

# 2. Abstract Base Class & Diamond Multiple Inheritance with cooperative super()
class Component(ABC):
    def __init__(self, base_val):
        self.base_val = base_val

    @abstractmethod
    def compute(self, factor):
        return self.base_val * factor

    @property
    def tag_weight(self):
        return self.base_val + 7

class ScaleMixin(Component):
    def compute(self, factor):
        prev = super().compute(factor)
        return (prev * 3 + 11) % 1000003

    def bonus(self):
        return self.base_val * 2

class OffsetMixin(Component):
    def compute(self, factor):
        prev = super().compute(factor)
        return (prev + 97) % 1000003

    def penalty(self):
        return self.base_val // 2

# Diamond inheritance: SmartNode -> ScaleMixin -> OffsetMixin -> Component
class SmartNode(ScaleMixin, OffsetMixin):
    node_Registry_count = 0

    def __init__(self, base_val, extra):
        super().__init__(base_val)
        self.extra = extra

    def compute(self, factor):
        res = super().compute(factor)
        return (res + self.extra + self.bonus() - self.penalty()) % 1000003

    @staticmethod
    def combine(a, b):
        return (a * 17 + b * 31) % 1000003

    @classmethod
    def bump_registry(cls):
        cls.node_Registry_count = cls.node_Registry_count + 1
        return cls.node_Registry_count

class LeafNode(OffsetMixin):
    def __init__(self, base_val):
        super().__init__(base_val)

    def compute(self, factor):
        return (super().compute(factor) + self.tag_weight) % 1000003

def main():
    checksum = 0

    # Verify ABC enforcement raises when instantiating abstract Component directly
    abc_blocked = 0
    try:
        bad = Component(10)
    except:
        abc_blocked = 1
    checksum = checksum + abc_blocked

    r = 0
    while r < 40:
        # Build polymorphic list of diamond-inherited and single-inherited instances
        nodes = []
        i = 0
        while i < 500:
            if i % 3 == 0:
                nodes.append(SmartNode(i + r, (i * 7) % 50))
                SmartNode.bump_registry()
            else:
                nodes.append(LeafNode(i + r))
            i = i + 1

        # Polymorphic dispatch, MRO super() chain, properties, and isinstance checks
        round_acc = 0
        idx = 0
        while idx < len(nodes):
            n_obj = nodes[idx]
            val = n_obj.compute((idx % 13) + 1)
            if isinstance(n_obj, ScaleMixin):
                val = SmartNode.combine(val, n_obj.tag_weight)
            elif isinstance(n_obj, Component):
                val = val + n_obj.tag_weight
            round_acc = (round_acc + val) % 1000000007
            idx = idx + 1

        # Dataclass instantiation, inheritance, operator overloading, and __eq__
        v_acc = Vec2D(0, 0)
        eq_hits = 0
        k = 0
        while k < 500:
            v1 = Vec2D(k, k * 2)
            v2 = Vec2D(r, k)
            v_acc = v_acc + v1 + v2
            v3 = Vec3D(k, k * 2, r)
            v4 = Vec3D(k, k * 2, r)
            if v3 == v4:
                eq_hits = eq_hits + v3.norm_sq()
            k = k + 1

        checksum = (checksum + round_acc + v_acc.norm_sq() + eq_hits + SmartNode.node_Registry_count) % 1000000007
        r = r + 1

    print(checksum)

main()
