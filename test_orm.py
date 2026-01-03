import asyncio
from turbo import Turbo, Model, CharField, IntegerField

# Define Model
class User(Model):
    name = CharField(max_length=100)
    age = IntegerField()
    # We need to manually add ID in our MVP ORM or handle it. 
    # My current models.py implementation doesn't auto-add ID unless I fix it.
    # Let's add it explicitly for now to be safe, or check models.py logic.
    id = IntegerField(primary_key=True)

async def main():
    print("Creating table...")
    await User.create_table()
    
    print("Creating user...")
    user = await User.objects.create(name="Alice", age=30)
    print(f"Created user: {user.name}, {user.age}")
    
    print("Fetching user...")
    fetched_user = await User.objects.get(name="Alice")
    print(f"Fetched user: {fetched_user.name}, {fetched_user.age}")
    
    print("Fetching all...")
    all_users = await User.objects.all()
    print(f"Count: {len(all_users)}")

if __name__ == "__main__":
    asyncio.run(main())
