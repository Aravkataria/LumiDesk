from fastapi import FastAPI, Request
from fastapi.responses import HTMLResponse, RedirectResponse
import uvicorn
from routes.spotify import router as spotify_router
from config import Config
from auth import auth


app = FastAPI(
    title="Spotify OLED Backend",
    version="2.0"
)
app.include_router(spotify_router)

# Prevent opening browser multiple times
login_started = False


@app.on_event("startup")
def startup_event():

    global login_started

    print("\n==============================")
    print(" Spotify OLED Backend Starting ")
    print("==============================\n")


    if not auth.authenticated():

        if not login_started:
            login_started = True
            auth.open_browser()

            print(
                "\nWaiting for Spotify authentication..."
            )

    else:

        print("Spotify already authenticated ")



@app.get("/")
def home():

    if auth.authenticated():

        return {
            "status": "online",
            "spotify": "connected"
        }


    return {
        "status": "online",
        "spotify": "not connected"
    }



@app.get("/login")
def login():

    auth.open_browser()

    return HTMLResponse(
        """
        <html>
        <body>

        <h2>
        Opening Spotify Login...
        </h2>

        <p>
        You can close this window after login.
        </p>

        </body>
        </html>
        """
    )



@app.get("/callback")
def callback(code: str):

    try:

        auth.exchange_code(code)

        return HTMLResponse(
            """
            <html>

            <body>

            <h1>
            Spotify Connected Successfully 🎵
            </h1>

            <p>
            You can close this tab.
            </p>

            </body>

            </html>
            """
        )


    except Exception as e:

        return HTMLResponse(
            f"""
            <h1>
            Authentication Failed
            </h1>

            <p>
            {e}
            </p>
            """
        )



@app.get("/status")
def status():

    return {

        "authenticated":
            auth.authenticated()

    }



if __name__ == "__main__":

    uvicorn.run(

        app,

        host=Config.HOST,

        port=Config.PORT

    )