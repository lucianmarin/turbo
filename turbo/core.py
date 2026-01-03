from typing import Dict, Any, Callable, List
import os
from jinja2 import Environment, FileSystemLoader, select_autoescape
from .router import Router
from .request import Request
from .response import Response

class Turbo:
    def __init__(self, template_dir: str = "templates"):
        self.router = Router()
        self.startup_handlers = []
        self.shutdown_handlers = []
        
        # Initialize Jinja2
        if os.path.exists(template_dir):
            self.jinja_env = Environment(
                loader=FileSystemLoader(template_dir),
                autoescape=select_autoescape(['html', 'xml'])
            )
        else:
            self.jinja_env = None

    def on_startup(self, handler: Callable):
        self.startup_handlers.append(handler)
        return handler

    def on_shutdown(self, handler: Callable):
        self.shutdown_handlers.append(handler)
        return handler
    
    def render(self, template_name: str, context: Dict[str, Any] = None) -> Response:
        if self.jinja_env is None:
            raise RuntimeError("Template directory not found. Please create a 'templates' folder.")
        
        if context is None:
            context = {}
            
        template = self.jinja_env.get_template(template_name)
        content = template.render(**context)
        return Response(content, media_type="text/html")

    # WSGI Entry Point
    def __call__(self, environ: Dict[str, Any], start_response: Callable):
        request = Request(environ)
        response = Response()

        handler, params = self.router.resolve(request.path, request.method)

        if handler:
            request.path_params = params
            try:
                # Call handler with new signature: (req, resp, **params)
                # Decorators (@before, @after) will intercept this call.
                result = handler(request, response, **params)
                
                # Handler can return a Response, a dict/list (JSON), or a string (Text).
                # Or it might modify 'response' object directly and return None.
                
                if result is not None:
                    if isinstance(result, Response):
                        response = result
                    elif isinstance(result, (dict, list)):
                        response.json(result)
                    else:
                        response.text(str(result))
                    
            except Exception as e:
                import traceback
                traceback.print_exc() # Print to stderr for server logs
                response.status_code = 500
                response.text(f"Internal Server Error: {str(e)}")
        else:
            response.status_code = 404
            response.text("Not Found")

        return response(start_response)

    def get(self, path: str):
        def wrapper(handler: Callable):
            self.router.add_route(path, "GET", handler)
            return handler
        return wrapper

    def post(self, path: str):
        def wrapper(handler: Callable):
            self.router.add_route(path, "POST", handler)
            return handler
        return wrapper