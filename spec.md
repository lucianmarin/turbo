# Turbo Framework Specification (Sync Edition)

## 1. Overview
**Turbo** is a high-performance synchronous Python web framework. It merges the minimalist design of Falcon with the developer ergonomics of FastAPI, coupled with a robust blocking Active Record ORM inspired by Django. It is built on the **WSGI** standard.

## 2. Core Philosophy
- **Simplicity:** Standard synchronous execution flow (WSGI).
- **Explicit Control:** Request processing pipeline controlled via decorators (Falcon-style).
- **Productivity:** "Batteries-included" synchronous ORM.

## 3. Architecture Components

### 3.1. The App Core (WSGI)
- **WSGI Native:** Compatible with Gunicorn, uWSGI, etc.
- **Routing:** Regex-based routing.
- **Request/Response Objects:**
  - Handlers receive `req` and `resp` objects explicitly.
  - Signature: `def handler(req, resp, **params):`

### 3.2. Dependency Injection & Validation
- **Decorators:** `@before` and `@after` hooks to execute logic around handlers.
- **Validation:** Explicit validation hooks (e.g., `validate_body`) that parse Pydantic models and inject them into `params`.

### 3.3. Turbo ORM (Sync)
- **Pattern:** Active Record.
- **Backend:** Standard `sqlite3`.
- **QuerySet API:** Chainable methods (`filter`, `get`, `all`).

### 3.4. Template Engine
- **Engine:** Jinja2.
- **API:** Helper method `render(template_name, context)` on the App instance.

## 4. API Design Example

```python
from turbo import Turbo, Model, CharField, IntegerField, before
from pydantic import BaseModel

app = Turbo()

# ORM Model
class User(Model):
    name = CharField(max_length=100)
    age = IntegerField()

# Pydantic Schema
class UserCreate(BaseModel):
    name: str
    age: int

# Dependency Injection / Validation Hook
def require_user_body(req, resp, params):
    # Manually parse or use a helper
    data = req.json()
    params['user_in'] = UserCreate(**data)

@app.post("/users")
@before(require_user_body)
def create_user(req, resp, user_in):
    # user_in is injected via the hook into params
    user = User.objects.create(name=user_in.name, age=user_in.age)
    return {"id": user.id, "name": user.name}
```

## 5. Technical Requirements
- **Python:** 3.9+
- **Dependencies:** 
  - `pydantic`
  - `jinja2`
  - WSGI Server