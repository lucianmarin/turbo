from typing import Any

class Field:
    def __init__(self, primary_key: bool = False, nullable: bool = False, default: Any = None):
        self.primary_key = primary_key
        self.nullable = nullable
        self.default = default
        self.name = None  # Set by metaclass

    def sql_type(self) -> str:
        raise NotImplementedError

class IntegerField(Field):
    def sql_type(self) -> str:
        return "INTEGER"

class CharField(Field):
    def __init__(self, max_length: int = 255, **kwargs):
        super().__init__(**kwargs)
        self.max_length = max_length

    def sql_type(self) -> str:
        return f"VARCHAR({self.max_length})"
