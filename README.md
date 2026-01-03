# Turbo Framework

Turbo is a high-performance synchronous Python web framework that merges the minimalist design of Falcon with the developer ergonomics of FastAPI, coupled with a robust blocking Active Record ORM inspired by Django.

## Core Features

- **Sync First:** Built on the robust WSGI standard for standard synchronous execution.
- **Explicit Control:** Request processing pipeline controlled via decorators (`@before`, `@after`) inspired by Falcon hooks.
- **Batteries Included:** Built-in Active Record ORM and Jinja2 template support.
- **Type Safe:** Leverages Pydantic for data validation.

## Installation

Turbo requires Python 3.9+.

```bash
# Clone the repository and install dependencies
pip install -r requirements.txt
```

## Quick Start

Create a `main.py`:

```python
from turbo import Turbo

app = Turbo()

@app.get("/")
def home(req, resp):
    return {"message": "Hello World"}

if __name__ == "__main__":
    from wsgiref.simple_server import make_server
    with make_server('', 8000, app) as httpd:
        print("Serving on port 8000...")
        httpd.serve_forever()
```

Run it:
```bash
python3 main.py
```

## Routing & Handlers

Handlers receive `req` (Request) and `resp` (Response) objects explicitly. Return values can be:
- A `dict` or `list` (serialized to JSON).
- A `Response` object.
- A string (returned as text/plain).

```python
@app.get("/items/{item_id}")
def get_item(req, resp, item_id):
    return {"item_id": item_id}
```

## Templates (Jinja2)

Turbo automatically looks for a `templates` directory.

```python
@app.get("/view")
def view(req, resp):
    return app.render("index.html", {"title": "Hello"})
```

## Turbo ORM

Define models by inheriting from `Model`.

```python
from turbo import Model, CharField, IntegerField

class User(Model):
    id = IntegerField(primary_key=True)
    name = CharField(max_length=100)
    email = CharField(max_length=100)

# Initialize Table
User.create_table()

# Create
user = User.objects.create(name="Alice", age=30)

# Query
user = User.objects.get(id=1)
users = User.objects.filter(name="Alice").all()
```

## Dependency Injection (Hooks)

Use `@before` and `@after` decorators to intercept requests and inject dependencies.

```python
from turbo import before
from pydantic import BaseModel

class ItemModel(BaseModel):
    name: str
    price: float

def validate_body(req, resp, params):
    # Validate and inject into params
    data = req.json()
    params['item'] = ItemModel(**data)

@app.post("/items")
@before(validate_body)
def create_item(req, resp, item):
    return {"name": item.name}
```

## Request & Response Objects

### Request (`req`)
- `req.path`: The request path.
- `req.method`: The HTTP method.
- `req.body()`: Returns request body bytes.
- `req.json()`: Returns parsed JSON body.

### Response (`resp`)
- `resp.status_code`: HTTP status code (default 200).
- `resp.headers`: List of headers.
- `resp.text(content)`: Sets content as text/plain.
- `resp.json(content)`: Sets content as application/json.
