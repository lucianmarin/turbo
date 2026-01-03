from typing import Any, Dict, Optional
import json

class Request:
    def __init__(self, environ: Dict[str, Any]):
        self.environ = environ
        self.path_params = {}
        self._body = None

    @property
    def method(self) -> str:
        return self.environ.get("REQUEST_METHOD", "GET")

    @property
    def path(self) -> str:
        return self.environ.get("PATH_INFO", "/")

    def body(self) -> bytes:
        if self._body is None:
            try:
                content_length = int(self.environ.get("CONTENT_LENGTH", 0))
            except (ValueError, TypeError):
                content_length = 0
            
            if content_length > 0:
                self._body = self.environ["wsgi.input"].read(content_length)
            else:
                self._body = b""
        return self._body

    def json(self) -> Any:
        body = self.body()
        if not body:
            return {}
        return json.loads(body)