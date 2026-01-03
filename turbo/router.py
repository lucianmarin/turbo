import re
from typing import Callable, Tuple, Dict, Any, Optional

class Router:
    def __init__(self):
        self.routes = []

    def add_route(self, path: str, method: str, handler: Callable):
        # Convert path like /users/{user_id} to regex
        # This is a simplified implementation. 
        # A real radix tree would be more performant as per specs, 
        # but for Phase 1 skeleton, regex is faster to implement.
        
        # Escape path but allow {param}
        regex_path = re.sub(r"{([^}]+)}", r"(?P<\1>[^/]+)", path)
        regex_path = f"^{regex_path}$"
        
        self.routes.append({
            "regex": re.compile(regex_path),
            "method": method,
            "handler": handler
        })

    def resolve(self, path: str, method: str) -> Tuple[Optional[Callable], Dict[str, Any]]:
        for route in self.routes:
            if route["method"] == method:
                match = route["regex"].match(path)
                if match:
                    return route["handler"], match.groupdict()
        return None, {}
