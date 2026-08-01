from __future__ import annotations
 
import logging
import os
import subprocess
import sys
import threading
import tkinter as tk
 
import pystray
from PIL import Image, ImageDraw
 
logger = logging.getLogger("lumidesk.tray")
 
 
def _build_icon_image() -> Image.Image:
    """Small generated glyph so the app doesn't need a bundled .ico —
    one less binary asset to keep in sync. A rounded square with an "L".
    """
    size = 64
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.rounded_rectangle([2, 2, size - 2, size - 2], radius=14, fill=(30, 32, 38, 255))
    draw.rectangle([20, 16, 26, 44], fill=(0, 220, 170, 255))
    draw.rectangle([20, 38, 46, 44], fill=(0, 220, 170, 255))
    return img
 
 
class TrayApp:
    def __init__(self, config, backend, ip_watcher, log_path, project_root, on_quit):
        self.config = config
        self.backend = backend
        self.ip_watcher = ip_watcher
        self.log_path = log_path
        self.project_root = project_root
        self.on_quit = on_quit
        self.icon: pystray.Icon | None = None
 
    def _status_text(self, item) -> str:
        if self.backend.is_running():
            ip = self.ip_watcher.current_ip() or "unknown"
            port = self.config.get("backend.port")
            return f"Backend: running | {ip}:{port}"
        if self.backend.stopped_by_user:
            return "Backend: stopped (you turned it off)"
        return "Backend: stopped (crashed — see logs)"
 
    def _open_logs(self, icon, item) -> None:
        try:
            if sys.platform == "win32":
                os.startfile(self.log_path)  # noqa: S606
            else:
                subprocess.Popen(["xdg-open", str(self.log_path)])
        except OSError:
            logger.exception("Couldn't open log file.")
 
    def _start_backend(self, icon, item) -> None:
        threading.Thread(target=self.backend.start, daemon=True).start()
 
    def _stop_backend(self, icon, item) -> None:
        threading.Thread(target=self.backend.stop, daemon=True).start()
 
    def _copy_api_key(self, icon, item) -> None:
        key = self.config.get("security.api_key", "")
        # A throwaway hidden Tk root just to reach the OS clipboard —
        # no window is shown, and the key never touches any log file.
        root = tk.Tk()
        root.withdraw()
        root.clipboard_clear()
        root.clipboard_append(key)
        root.update()
        root.after(200, root.destroy)
        root.mainloop()
 
    def _restart_backend(self, icon, item) -> None:
        threading.Thread(target=self.backend.restart, daemon=True).start()
 
    def _backend_is_stopped(self, item) -> bool:
        return not self.backend.is_running()
 
    def _backend_is_running(self, item) -> bool:
        return self.backend.is_running()
 
    def _open_settings(self, icon, item) -> None:
        # Imported lazily so tkinter only spins up when actually needed.
        from settings_window import open_settings_window
        threading.Thread(
            target=open_settings_window,
            args=(self.config, self._on_settings_saved),
            daemon=True,
        ).start()
 
    def _on_settings_saved(self) -> None:
        self.config.write_backend_env(self.project_root)
        logger.info("Settings changed, restarting backend to pick them up.")
        self.backend.restart()
 
    def _quit(self, icon, item) -> None:
        icon.stop()
        self.on_quit()
 
    def run(self) -> None:
        menu = pystray.Menu(
            pystray.MenuItem(self._status_text, None, enabled=False),
            pystray.Menu.SEPARATOR,
            pystray.MenuItem("Start backend", self._start_backend, enabled=self._backend_is_stopped),
            pystray.MenuItem("Stop backend", self._stop_backend, enabled=self._backend_is_running),
            pystray.MenuItem("Restart backend", self._restart_backend),
            pystray.MenuItem("Copy API key (for ESP setup)", self._copy_api_key),
            pystray.MenuItem("Open settings", self._open_settings),
            pystray.MenuItem("Open logs", self._open_logs),
            pystray.Menu.SEPARATOR,
            pystray.MenuItem("Quit LumiDesk", self._quit),
        )
        self.icon = pystray.Icon("LumiDesk", _build_icon_image(), "LumiDesk", menu)
        self.icon.run() 