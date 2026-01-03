import sqlite3
from typing import Optional, List, Dict, Any

class Database:
    def __init__(self, db_name: str = "db.sqlite3"):
        self.db_name = db_name

    def get_connection(self):
        return sqlite3.connect(self.db_name)

    def execute(self, sql: str, params: tuple = ()):
        with self.get_connection() as conn:
            conn.execute(sql, params)
            conn.commit()

    def fetch_one(self, sql: str, params: tuple = ()) -> Optional[Dict[str, Any]]:
        with self.get_connection() as conn:
            conn.row_factory = sqlite3.Row
            cursor = conn.execute(sql, params)
            row = cursor.fetchone()
            return dict(row) if row else None

    def fetch_all(self, sql: str, params: tuple = ()) -> List[Dict[str, Any]]:
        with self.get_connection() as conn:
            conn.row_factory = sqlite3.Row
            cursor = conn.execute(sql, params)
            rows = cursor.fetchall()
            return [dict(row) for row in rows]

# Global DB instance
db = Database()
