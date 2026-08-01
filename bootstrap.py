from __future__ import annotations
 
import logging
import socket
import sys
from pathlib import Path
 
# When frozen by PyInstaller, the backend source (app.py etc.) is
# bundled as data files alongside the executable rather than on
# sys.path automatically — this resolves both the dev (running from
# source) and frozen cases the same way.
if getattr(sys, "frozen", False):
    PROJECT_ROOT = Path(sys._MEIPASS)  # type: ignore[attr-defined]
else:
    # bootstrap.py lives in the SAME folder as app.py, not a nested
    # desktop/ subfolder — so the project root is just this folder.
    PROJECT_ROOT = Path(__file__).resolve().parent
 
sys.path.insert(0, str(Path(__file__).resolve().parent))
 
 
def _run_as_backend_child(host: str, port: int) -> None:
    """Entered only when this exe was re-launched with --serve-backend
    (the frozen-exe case — see backend_manager.py for why). Imports the
    existing, untouched app.py and runs uvicorn against it in-process.
    No tray/tkinter/pystray imports happen on this path.
    """
    os_chdir_target = str(PROJECT_ROOT)
    import os
    os.chdir(os_chdir_target)
    sys.path.insert(0, os_chdir_target)
 
    import uvicorn
    import app as backend_app  # existing app.py, unmodified
 
    uvicorn.run(backend_app.app, host=host, port=port, log_level="info")
 
 
if len(sys.argv) >= 4 and sys.argv[1] == "--serve-backend":
    _run_as_backend_child(sys.argv[2], int(sys.argv[3]))
    sys.exit(0)
 
 
from config import ConfigManager
from logging_setup import setup_logging, log_file_path
from backend_manager import BackendManager
from ip_watcher import IPWatcher
from tray import TrayApp
 
SINGLE_INSTANCE_PORT = 47411  # arbitrary, just needs to be unlikely to collide
 
 
def _acquire_single_instance_lock() -> socket.socket | None:
    """Bind a local TCP port as a crude mutex so double-launching
    LumiDesk.exe (e.g. from a startup shortcut + a manual double-click)
    doesn't spin up two backends fighting over the same port.
    """
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        s.bind(("127.0.0.1", SINGLE_INSTANCE_PORT))
        s.listen(1)
        return s
    except OSError:
        return None
 
 
def main() -> None:
    lock = _acquire_single_instance_lock()
    if lock is None:
        # Another instance is already running — just exit quietly.
        return
 
    config = ConfigManager()
    logger = setup_logging(config.config_dir, config.get("logging.level", "INFO"))
    logger.info("LumiDesk starting. project_root=%s", PROJECT_ROOT)
 
    changed = config.write_backend_env(PROJECT_ROOT)
    if changed:
        logger.info("Backend .env updated from config.json.")
 
    backend = BackendManager(
        project_root=PROJECT_ROOT,
        host=config.get("backend.host", "0.0.0.0"),
        port=config.get("backend.port", 8000),
        log_dir=config.config_dir / "logs",
        api_key=config.get("security.api_key"),
    )
    backend.start()
 
    if not backend.wait_until_healthy(timeout=15):
        logger.error(
            "Backend did not report healthy within 15s. It may still be "
            "starting, or check %s for the underlying error.",
            log_file_path(config.config_dir),
        )
        # Keep going rather than exit — the watchdog in BackendManager
        # will keep retrying, and the tray still gives the user a way
        # to see status and open logs.
 
    def on_ip_change(new_ip: str) -> None:
        logger.warning(
            "Local IP changed to %s. The ESP's SERVER_URL was set at "
            "flash time, so it will need a matching DHCP reservation "
            "or a re-flash to keep reaching this PC.",
            new_ip,
        )
 
    ip_watcher = IPWatcher(on_change=on_ip_change)
    ip_watcher.start()
 
    def on_quit() -> None:
        logger.info("Shutting down.")
        ip_watcher.stop()
        backend.stop()
        lock.close()
 
    tray = TrayApp(
        config=config,
        backend=backend,
        ip_watcher=ip_watcher,
        log_path=log_file_path(config.config_dir),
        project_root=PROJECT_ROOT,
        on_quit=on_quit,
    )
    tray.run()  # blocks until Quit is chosen
 
 
if __name__ == "__main__":
    main()