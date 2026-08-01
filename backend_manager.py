from __future__ import annotations
 
import logging
import subprocess
import sys
import threading
import time
from pathlib import Path
 
import requests
 
logger = logging.getLogger("lumidesk.backend")
 
CREATE_NO_WINDOW = 0x08000000  # Windows-only flag; ignored elsewhere
 
 
class BackendManager:
    def __init__(self, project_root: Path, host: str, port: int,
                 log_dir: Path | None = None, python_exe: str | None = None,
                 api_key: str | None = None):
        self.project_root = project_root
        self.host = host
        self.port = port
        self.api_key = api_key
        self.python_exe = python_exe or sys.executable
        # When frozen by PyInstaller, sys.executable is LumiDesk.exe
        # itself, not a real Python interpreter — "LumiDesk.exe -m
        # uvicorn ..." would not work. Instead we re-launch the same
        # exe with a hidden flag that bootstrap.py intercepts before
        # doing anything else, and which runs uvicorn in-process in
        # that child. See bootstrap.py's top-of-file argv check.
        self._frozen = getattr(sys, "frozen", False)
        self._process: subprocess.Popen | None = None
        self._watchdog_thread: threading.Thread | None = None
        self._stopping = threading.Event()
        self._restart_count = 0
        self._max_quick_restarts = 5  # guard against a crash-restart loop
        # Set when the user explicitly clicks "Stop backend" — tells the
        # watchdog "this is intentional, don't bring it back." Cleared by
        # start()/restart(). Without this, there was no way to actually
        # turn the backend off — the watchdog would just relaunch it.
        self._manual_stop = False
        # Backend's own stdout/stderr (its actual tracebacks) go here —
        # previously these were sent to DEVNULL, which is why a crash
        # showed up in lumidesk.log only as "exited unexpectedly", with
        # no way to see *why*.
        self.log_dir = log_dir or (project_root / "logs")
        self.log_dir.mkdir(parents=True, exist_ok=True)
        self.backend_log_path = self.log_dir / "backend.log"
 
    @property
    def base_url(self) -> str:
        host = "127.0.0.1" if self.host == "0.0.0.0" else self.host
        return f"http://{host}:{self.port}"
 
    @property
    def stopped_by_user(self) -> bool:
        return self._manual_stop and not self.is_running()
 
    def is_running(self) -> bool:
        return self._process is not None and self._process.poll() is None
 
    def start(self) -> None:
        """Public entrypoint: called for the initial launch and for a
        user-requested restart. Resets the crash counter, since this is
        a deliberate fresh start, not the watchdog retrying after a crash.
        """
        self._restart_count = 0
        self._manual_stop = False
        self._start_process()
 
        if not self._watchdog_thread or not self._watchdog_thread.is_alive():
            self._watchdog_thread = threading.Thread(
                target=self._watchdog, daemon=True, name="BackendWatchdog"
            )
            self._watchdog_thread.start()
 
    def _start_process(self) -> None:
        """Actually spawns the subprocess. Does NOT touch the crash
        counter — the watchdog calls this directly on a crash-restart so
        the counter it's tracking keeps climbing instead of resetting.
        """
        if self.is_running():
            logger.info("Backend already running, skipping start.")
            return
 
        if self._frozen:
            cmd = [
                self.python_exe, "--serve-backend",
                self.host, str(self.port),
            ]
        else:
            cmd = [
                self.python_exe, "-m", "uvicorn", "app:app",
                "--host", self.host, "--port", str(self.port),
            ]
        logger.info("Starting backend: %s (cwd=%s)", " ".join(cmd), self.project_root)
 
        creationflags = CREATE_NO_WINDOW if sys.platform == "win32" else 0
        # Append mode + reopened each start so this survives across
        # restarts without needing to keep a handle open long-term.
        log_file = open(self.backend_log_path, "a", encoding="utf-8")
        self._process = subprocess.Popen(
            cmd,
            cwd=str(self.project_root),
            stdout=log_file,
            stderr=subprocess.STDOUT,
            creationflags=creationflags,
        )
        self._stopping.clear()
 
    def stop(self, user_initiated: bool = True) -> None:
        if user_initiated:
            self._manual_stop = True
        self._stopping.set()
        if self._process and self._process.poll() is None:
            logger.info("Stopping backend (pid=%s)", self._process.pid)
            self._process.terminate()
            try:
                self._process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                logger.warning("Backend didn't exit cleanly, killing.")
                self._process.kill()
        self._process = None
        self._stopping.clear()  # ready for a future start(), watchdog thread keeps idling
 
    def restart(self) -> None:
        logger.info("Restarting backend.")
        self.stop()
        time.sleep(0.5)
        self.start()
 
    def wait_until_healthy(self, timeout: float = 15.0, interval: float = 0.5) -> bool:
        deadline = time.time() + timeout
        while time.time() < deadline:
            if self.health_check():
                return True
            time.sleep(interval)
        return False
 
    def health_check(self) -> bool:
        headers = {"X-API-Key": self.api_key} if self.api_key else {}
        try:
            resp = requests.get(f"{self.base_url}/health", headers=headers, timeout=2)
            return resp.status_code == 200
        except requests.RequestException:
            return False
 
    def _watchdog(self) -> None:
        """If the backend process dies unexpectedly, restart it a few
        times with a short backoff. If it keeps dying immediately, stop
        trying rather than spin forever — that's a real bug to surface
        in the logs, not paper over with an infinite retry loop.
        """
        while not self._stopping.is_set():
            if self._process is not None:
                exit_code = self._process.poll()
                if exit_code is not None:
                    if self._stopping.is_set():
                        break
                    self._restart_count += 1
                    logger.warning(
                        "Backend exited unexpectedly (code=%s). Restart attempt %d/%d. "
                        "See %s for the backend's own error output.",
                        exit_code, self._restart_count, self._max_quick_restarts,
                        self.backend_log_path,
                    )
                    if self._restart_count > self._max_quick_restarts:
                        logger.error(
                            "Backend crashed %d times in a row — giving up. "
                            "Check %s for the underlying error.",
                            self._restart_count, self.backend_log_path,
                        )
                        break
                    time.sleep(min(2 ** self._restart_count, 30))
                    self._start_process()
            time.sleep(1)