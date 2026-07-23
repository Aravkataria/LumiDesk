from flask import Flask

app = Flask(__name__)

@app.route("/")
def home():
    return "ESP32 Connected!"

@app.route("/spotify")
def spotify():
    return """
{
"title":"Blinding Lights",
"artist":"The Weeknd",
"progress":90,
"duration":200,
"playing":true
}
"""

app.run(host="0.0.0.0", port=5000)