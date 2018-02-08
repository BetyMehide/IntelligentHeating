from flask import Flask, request, jsonify
app = Flask(__name__)

# Tell `app` that if someone asks for `/` (which is the main page)
# then run this function, and send back the return value
@app.route("/")
def hello():
    return "Hello World!"

# Let's accept some data: a json object containing two numbers,
# 'x' and 'y', we'll return the sum of them.
@app.route("/post/add_two_numbers", methods=['POST'])
def add_two_numbers():
    x = request.json['Temperature']
    y = request.json['Humidity']
    z = request.json ['Presesnce']
    with open('log.txt', 'w') as f:
        f.write(x,',',y,',',z,',')
        f.close()
    return jsonify({'abcd':x})

app.run(host='0.0.0.0', port=5000)