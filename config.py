from __future__ import annotations

import copy
import json
import os
import secrets
import threading
from pathlib import Path
from typing import Any


APP_NAME = "LumiDesk"

DEFAULTS: dict[str, Any] = {
    "backend": {
        "host": "0.0.0.0",
        "port": 8000,
    },
    "weather": {
        "lat": None,
        "lon": None,
    },
    "logging": {
        "level": "INFO",
    },
    "launch": {
        "start_minimized": True,
        "start_with_windows": False,
    },
    "updates": {
        "auto_check": False,
        "channel": "stable",
    },
    "security": {
        # Generated on first run, then reused — not a placeholder, an
        # actual per-install secret. The ESP needs the same value
        # compiled into its secrets.h to be allowed to talk to this
        # backend; see the desktop README for the firmware side.
        "api_key": None,
    },
}


def _config_dir() -> Path:
    base = os.getenv("APPDATA")
    if base:
        return Path(base) / APP_NAME
    # Non-Windows fallback (dev machines, testing).
    return Path.home() / f".{APP_NAME.lower()}"


def _deep_merge(defaults: dict, override: dict) -> dict:
    """Fill in any keys missing from `override` using `defaults`,
    recursively, without dropping unknown keys the user may have added.
    """
    result = copy.deepcopy(defaults)
    for key, value in override.items():
        if isinstance(value, dict) and isinstance(result.get(key), dict):
            result[key] = _deep_merge(result[key], value)
        else:
            result[key] = value
    return result


class ConfigManager:
    """Loads, merges, and persists config.json. Thread-safe for the
    simple read/write patterns the tray app and settings window use.
    """

    def __init__(self, config_dir: Path | None = None):
        self.config_dir = config_dir or _config_dir()
        self.config_path = self.config_dir / "config.json"
        self._lock = threading.Lock()
        self._data: dict[str, Any] = {}
        self.load()

    def load(self) -> dict[str, Any]:
        with self._lock:
            self.config_dir.mkdir(parents=True, exist_ok=True)
            if self.config_path.exists():
                try:
                    on_disk = json.loads(self.config_path.read_text(encoding="utf-8"))
                except (json.JSONDecodeError, OSError):
                    # Corrupt or unreadable config — don't crash the launcher,
                    # fall back to defaults and let the next save fix the file.
                    on_disk = {}
            else:
                on_disk = {}

            merged = _deep_merge(DEFAULTS, on_disk)
            if not merged["security"]["api_key"]:
                merged["security"]["api_key"] = secrets.token_hex(32)
            self._data = merged
            self._write_locked()
            return copy.deepcopy(self._data)

    def _write_locked(self) -> None:
        tmp_path = self.config_path.with_suffix(".tmp")
        tmp_path.write_text(json.dumps(self._data, indent=2), encoding="utf-8")
        tmp_path.replace(self.config_path)  # atomic on Windows/NTFS

    def save(self) -> None:
        with self._lock:
            self._write_locked()

    def get(self, dotted_key: str, default: Any = None) -> Any:
        node: Any = self._data
        for part in dotted_key.split("."):
            if not isinstance(node, dict) or part not in node:
                return default
            node = node[part]
        return node

    def set(self, dotted_key: str, value: Any, save: bool = True) -> None:
        parts = dotted_key.split(".")
        with self._lock:
            node = self._data
            for part in parts[:-1]:
                node = node.setdefault(part, {})
            node[parts[-1]] = value
            if save:
                self._write_locked()

    def as_dict(self) -> dict[str, Any]:
        with self._lock:
            return copy.deepcopy(self._data)

    def write_backend_env(self, project_root: Path) -> bool:
        """Write a .env file the existing backend already reads via
        load_dotenv(). Returns True if the file changed on disk so the
        caller can decide whether a backend restart is needed.

        This deliberately mirrors ONLY the keys app.py already consumes
        (LAT, LON). It does not invent new backend config surface.
        """
        lat = self.get("weather.lat")
        lon = self.get("weather.lon")
        api_key = self.get("security.api_key")
        lines = []
        if lat is not None:
            lines.append(f"LAT={lat}")
        if lon is not None:
            lines.append(f"LON={lon}")
        if api_key:
            lines.append(f"API_KEY={api_key}")
        new_content = "\n".join(lines) + ("\n" if lines else "")

        env_path = project_root / ".env"
        old_content = env_path.read_text(encoding="utf-8") if env_path.exists() else None
        if old_content == new_content:
            return False

        env_path.write_text(new_content, encoding="utf-8")
        return True