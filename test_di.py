from turbo import Turbo, before
from pydantic import BaseModel

app = Turbo()

class Item(BaseModel):
    name: str
    price: float

def parse_item(req, resp, params):
    data = req.json()
    params['item'] = Item(**data)

@app.post("/items")
@before(parse_item)
def create_item(req, resp, item):
    return {"name": item.name, "price": item.price, "tax": item.price * 0.1}

@app.get("/users/{user_id}")
def get_user(req, resp, user_id):
    return {"user_id": user_id, "type": str(type(user_id)), "req_path": req.path}

if __name__ == "__main__":
    from wsgiref.simple_server import make_server
    print("Serving on port 8000...")
    with make_server('', 8000, app) as httpd:
        httpd.serve_forever()
