import requests

# Set up our two numbers
data = {
    'Temperature': 1,
    'Humidity': 2,
    'Presence': 3,
}

# POST it to the server (note that we could also use "localhost" here)
r = requests.post("http://localhost:5000/post/add_two_numbers", json=data)
print(r)

# Complain if we don't get it right
if r.status_code != 200:
    raise ValueError("ERROR: Didn't receive the result properly!")

result = r.json()

# Print out our addition:
print(result)