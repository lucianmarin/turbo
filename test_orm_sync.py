from turbo import Model, CharField, IntegerField

# Define Model
class User(Model):
    id = IntegerField(primary_key=True)
    name = CharField(max_length=100)
    age = IntegerField()

def main():
    print("Creating table...")
    User.create_table()
    
    print("Creating user...")
    user = User.objects.create(name="Bob", age=40)
    print(f"Created user: {user.name}, {user.age}")
    
    print("Fetching user...")
    fetched_user = User.objects.get(name="Bob")
    print(f"Fetched user: {fetched_user.name}, {fetched_user.age}")
    
    print("Fetching all...")
    all_users = User.objects.all()
    print(f"Count: {len(all_users)}")

if __name__ == "__main__":
    main()
