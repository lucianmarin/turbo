from .fields import Field
from .query import Manager
from .db import db

class ModelMeta(type):
    def __new__(cls, name, bases, attrs):
        if name == "Model":
            return super().__new__(cls, name, bases, attrs)
        
        fields = {}
        for key, value in attrs.items():
            if isinstance(value, Field):
                value.name = key
                fields[key] = value
        
        attrs["_fields"] = fields
        attrs["table_name"] = name.lower() + "s"
        
        new_class = super().__new__(cls, name, bases, attrs)
        new_class.objects = Manager(new_class)
        return new_class

class Model(metaclass=ModelMeta):
    def __init__(self, **kwargs):
        for key, value in kwargs.items():
            setattr(self, key, value)
    
    @classmethod
    def create_table(cls):
        fields_sql = []
        for name, field in cls._fields.items():
            definition = f"{name} {field.sql_type()}"
            if field.primary_key:
                definition += " PRIMARY KEY AUTOINCREMENT" if field.sql_type() == "INTEGER" else " PRIMARY KEY"
            if not field.nullable and not field.primary_key:
                 definition += " NOT NULL"
            fields_sql.append(definition)
        
        sql = f"CREATE TABLE IF NOT EXISTS {cls.table_name} ({', '.join(fields_sql)})"
        db.execute(sql)