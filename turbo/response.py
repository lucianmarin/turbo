from typing import Any, List, Tuple, Union
import json

class Response:
    def __init__(self, content: Any = None, status_code: int = 200, media_type: str = "text/plain"):
        self.content = content
        self.status_code = status_code
        self.media_type = media_type
        self.headers = []

    def json(self, content: Any):
        self.content = json.dumps(content).encode("utf-8")
        self.media_type = "application/json"
        return self

    def text(self, content: str):
        self.content = content.encode("utf-8")
        self.media_type = "text/plain"
        return self

    @property
    def status_string(self) -> str:
        # Simple mapping, can be expanded
        reasons = {
            200: "OK",
            404: "Not Found",
            500: "Internal Server Error"
        }
        return f"{self.status_code} {reasons.get(self.status_code, 'Unknown')}"

    def __call__(self, start_response) -> List[bytes]:
        body = self.content
        if body is None:
            body = b""
        elif isinstance(body, str):
            body = body.encode("utf-8")
        
        headers = [
            ("Content-Type", self.media_type),
            ("Content-Length", str(len(body)))
        ]
        start_response(self.status_string, headers)
        return [body]
