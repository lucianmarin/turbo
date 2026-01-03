from typing import List, Type, TypeVar
from .db import db

T = TypeVar("T", bound="Model")

class QuerySet:
    def __init__(self, model_cls: Type[T]):
        self.model_cls = model_cls
        self.filters = {}

    def filter(self, **kwargs) -> 'QuerySet':
        self.filters.update(kwargs)
        return self

    def all(self) -> List[T]:
        sql = f"SELECT * FROM {self.model_cls.table_name}"
        params = []
        if self.filters:
            conditions = []
            for k, v in self.filters.items():
                conditions.append(f"{k} = ?")
                params.append(v)
            sql += " WHERE " + " AND ".join(conditions)
        
        rows = db.fetch_all(sql, tuple(params))
        return [self.model_cls(**row) for row in rows]

    def get(self, **kwargs) -> T:
        self.filter(**kwargs)
        sql = f"SELECT * FROM {self.model_cls.table_name}"
        params = []
        conditions = []
        for k, v in self.filters.items():
            conditions.append(f"{k} = ?")
            params.append(v)
        sql += " WHERE " + " AND ".join(conditions)

        row = db.fetch_one(sql, tuple(params))
        if row:
            return self.model_cls(**row)
        raise Exception(f"{self.model_cls.__name__} matching query does not exist.")

    def create(self, **kwargs) -> T:
        keys = list(kwargs.keys())
        values = list(kwargs.values())
        placeholders = ["?"] * len(keys)
        
        sql = f"INSERT INTO {self.model_cls.table_name} ({', '.join(keys)}) VALUES ({', '.join(placeholders)})"
        
        db.execute(sql, tuple(values))
        return self.model_cls(**kwargs)

class Manager:
    def __init__(self, model_cls):
        self.model_cls = model_cls

    def filter(self, **kwargs) -> QuerySet:
        return QuerySet(self.model_cls).filter(**kwargs)

    def all(self) -> List[T]:
        return QuerySet(self.model_cls).all()

    def get(self, **kwargs) -> T:
        return QuerySet(self.model_cls).get(**kwargs)

    def create(self, **kwargs) -> T:
        return QuerySet(self.model_cls).create(**kwargs)