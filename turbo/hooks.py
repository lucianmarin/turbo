from functools import wraps
from typing import Callable, Any

def before(hook: Callable):
    """
    Decorator to execute a hook before the handler.
    The hook signature should be: hook(req, resp, params)
    """
    def decorator(func: Callable):
        @wraps(func)
        def wrapper(req, resp, **params):
            # Execute hook
            hook(req, resp, params)
            # Execute actual handler
            return func(req, resp, **params)
        return wrapper
    return decorator

def after(hook: Callable):
    """
    Decorator to execute a hook after the handler.
    The hook signature should be: hook(req, resp, params)
    """
    def decorator(func: Callable):
        @wraps(func)
        def wrapper(req, resp, **params):
            # Execute actual handler
            result = func(req, resp, **params)
            # Execute hook
            hook(req, resp, params)
            return result
        return wrapper
    return decorator
