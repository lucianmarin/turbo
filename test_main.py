from turbo import Turbo

app = Turbo()

@app.get("/")
def home(req, resp):
    return {"message": "Hello World"}

@app.get("/items/{item_id}")
def get_item(req, resp, item_id):
    return {"item_id": item_id}

if __name__ == "__main__":
    from wsgiref.simple_server import make_server
    print("Serving on port 8000...")
    with make_server('', 8000, app) as httpd:
        httpd.serve_forever()
