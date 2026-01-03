from turbo import Turbo, Model, CharField, IntegerField, before
from pydantic import BaseModel

app = Turbo()

# --- Models ---
class User(Model):
    id = IntegerField(primary_key=True)
    name = CharField(max_length=100)
    email = CharField(max_length=100)

# --- Schemas ---
class UserCreate(BaseModel):
    name: str
    email: str

# --- Hooks ---
def validate_user_body(req, resp, params):
    """
    Hook to validate request body against UserCreate schema.
    Injects the validated model into 'params' as 'user_in'.
    """
    try:
        data = req.json()
        params['user_in'] = UserCreate(**data)
    except Exception as e:
        # In a real app, you'd raise a specific error to be caught by the framework
        # and converted to 400 Bad Request.
        # For now, let's just let it bubble up (500) or handle it.
        raise ValueError(f"Invalid body: {e}")

# --- Startup ---
print("Initializing Database...")
User.create_table()

# --- Routes ---
@app.get("/")
def home(req, resp):
    return app.render("index.html", {
        "title": "Turbo Home", 
        "items": ["Fast", "Simple", "Powerful", "Decorators"]
    })

@app.post("/users")
@before(validate_user_body)
def create_user(req, resp, user_in):
    # 'user_in' is injected by the hook
    user = User.objects.create(name=user_in.name, email=user_in.email)
    return {"status": "created", "user": {"name": user.name, "email": user.email}}

@app.get("/users/{user_id}")
def get_user(req, resp, user_id):
    # user_id is passed from the router regex
    try:
        user = User.objects.get(id=int(user_id))
        return {"id": user.id, "name": user.name, "email": user.email}
    except Exception:
        return {"error": "User not found"}

if __name__ == "__main__":
    from wsgiref.simple_server import make_server
    print("Serving on port 8000...")
    with make_server('', 8000, app) as httpd:
        httpd.serve_forever()