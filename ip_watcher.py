from __future__ import annotations

import logging
import socket
import threading
import time
from typing import Callable

logger = logging.getLogger("lumidesk.ip")


def get_local_ip() -> str | None:
    """Best-effort local network IP. Opens a UDP socket to a public
    address without sending any data, just to see which local interface
    the OS would route through.
    """
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(("8.8.8.8", 80))
        return s.getsockname()[0]
    except OSError:
        return None
    finally:
        s.close()


class IPWatcher(threading.Thread):
    def __init__(self, on_change: Callable[[str], None], interval: float = 30.0):
        super().__init__(daemon=True, name="IPWatcher")
        self.on_change = on_change
        self.interval = interval
        self._stop_event = threading.Event()
        self._last_ip: str | None = None

    def current_ip(self) -> str | None:
        return self._last_ip

    def run(self) -> None:
        self._last_ip = get_local_ip()
        logger.info("Local IP: %s", self._last_ip)
        while not self._stop_event.is_set():
            self._stop_event.wait(self.interval)
            if self._stop_event.is_set():
                break
            ip = get_local_ip()
            if ip and ip != self._last_ip:
                logger.warning("Local IP changed: %s -> %s", self._last_ip, ip)
                self._last_ip = ip
                self.on_change(ip)

    def stop(self) -> None:
        self._stop_event.set()