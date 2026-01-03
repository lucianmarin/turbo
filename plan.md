# Development Plan (Sync Refactor)

## Phase 1: WSGI Core
- [x] Refactor `Turbo` class to implement the WSGI callable interface (`environ`, `start_response`).
- [x] Update `Request` to read from `environ['wsgi.input']`.
- [x] Update `Response` to return iterable bytes.
- [x] Remove `async`/`await` from Router and Core.

## Phase 2: Sync Dependency Injection (Deprecated)
- [x] Remove `await` from `resolve_dependencies`.
- [x] Ensure Pydantic models are parsed synchronously from the request body.

## Phase 3: Sync ORM
- [x] Refactor `Database` class to use direct `sqlite3` calls (remove `asyncio.to_thread`).
- [x] Update `QuerySet` and `Manager` methods (`all`, `get`, `create`) to be synchronous.
- [x] Update `Model.create_table` to be synchronous.

## Phase 4: Integration & Testing
- [x] Update `demo_app.py` to remove `async`/`await`.
- [x] Verify functionality using `wsgiref.simple_server` or `gunicorn`.

## Phase 5: Template Support
- [x] Initialize `jinja2.Environment` in `Turbo` class.
- [x] Add `render` method to `Turbo` class for HTML responses.
- [x] Create a `templates` directory and a sample HTML file.
- [x] Update `demo_app.py` to demonstrate HTML rendering.

## Phase 6: Refactor to Decorator-based DI
- [x] Implement `turbo/hooks.py` with `before` and `after` decorators.
- [x] Update `Turbo.__call__` to invoke handlers with `(req, resp, **params)` signature.
- [x] Remove `turbo/injector.py`.
- [x] Create helper hooks for Pydantic validation.
- [x] Refactor `demo_app.py` to use `@before` hooks instead of type hints.
