import json
import time
import webbrowser
from pathlib import Path
from urllib.parse import urlencode

import requests

from config import Config


class AuthManager:
    AUTH_URL = "https://accounts.spotify.com/authorize"
    TOKEN_URL = "https://accounts.spotify.com/api/token"

    def __init__(self):
        self.token_file = Path(Config.TOKEN_FILE)
        self.token = self._load_token()

    # ==========================================================
    # Token Storage
    # ==========================================================

    def _load_token(self):
        if self.token_file.exists():
            try:
                with open(self.token_file, "r", encoding="utf-8") as f:
                    return json.load(f)
            except Exception:
                return None
        return None

    def _save_token(self, token):
        self.token_file.parent.mkdir(parents=True, exist_ok=True)

        with open(self.token_file, "w", encoding="utf-8") as f:
            json.dump(token, f, indent=4)

        self.token = token

    # ==========================================================
    # State
    # ==========================================================

    def has_token(self):
        return self.token is not None

    def token_expired(self):
        if not self.has_token():
            return True

        return time.time() >= self.token["expires_at"] - 60

    def authenticated(self):
        if not self.has_token():
            return False

        if self.token_expired():
            return self.refresh_token()

        return True

    # ==========================================================
    # Login URL
    # ==========================================================

    def get_login_url(self):
        params = {
            "client_id": Config.CLIENT_ID,
            "response_type": "code",
            "redirect_uri": Config.REDIRECT_URI,
            "scope": " ".join(Config.SCOPES),
            "show_dialog": "false"
        }

        return self.AUTH_URL + "?" + urlencode(params)

    def open_browser(self):
        url = self.get_login_url()

        print("\nOpening Spotify Login...\n")
        print(url)

        webbrowser.open(url, new=1)

    # ==========================================================
    # OAuth
    # ==========================================================

    def exchange_code(self, code):

        response = requests.post(
            self.TOKEN_URL,
            data={
                "grant_type": "authorization_code",
                "code": code,
                "redirect_uri": Config.REDIRECT_URI,
                "client_id": Config.CLIENT_ID,
                "client_secret": Config.CLIENT_SECRET,
            },
        )

        if response.status_code != 200:
            raise Exception(response.text)

        token = response.json()

        token["expires_at"] = (
            int(time.time()) + token["expires_in"]
        )

        self._save_token(token)

        return token

    def refresh_token(self):

        if not self.has_token():
            return False

        response = requests.post(
            self.TOKEN_URL,
            data={
                "grant_type": "refresh_token",
                "refresh_token": self.token["refresh_token"],
                "client_id": Config.CLIENT_ID,
                "client_secret": Config.CLIENT_SECRET,
            },
        )

        if response.status_code != 200:
            return False

        new_token = response.json()

        if "refresh_token" not in new_token:
            new_token["refresh_token"] = self.token["refresh_token"]

        new_token["expires_at"] = (
            int(time.time()) + new_token["expires_in"]
        )

        self._save_token(new_token)

        return True

    # ==========================================================
    # Access Token
    # ==========================================================

    def get_access_token(self):

        if not self.authenticated():
            return None

        return self.token["access_token"]

    # ==========================================================
    # Logout
    # ==========================================================

    def logout(self):

        if self.token_file.exists():
            self.token_file.unlink()

        self.token = None


auth = AuthManager()