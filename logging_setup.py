from __future__ import annotations
 
import logging
import logging.handlers
from pathlib import Path
 
 
def setup_logging(config_dir: Path, level: str = "INFO") -> logging.Logger:
    log_dir = config_dir / "logs"
    log_dir.mkdir(parents=True, exist_ok=True)
    log_path = log_dir / "lumidesk.log"
 
    logger = logging.getLogger("lumidesk")
    logger.setLevel(getattr(logging, level.upper(), logging.INFO))
 
    if logger.handlers:
        # setup_logging may be called again after a settings change;
        # don't stack duplicate handlers.
        logger.handlers.clear()
 
    formatter = logging.Formatter(
        "%(asctime)s [%(levelname)s] %(name)s: %(message)s"
    )
 
    file_handler = logging.handlers.RotatingFileHandler(
        log_path, maxBytes=2_000_000, backupCount=3, encoding="utf-8"
    )
    file_handler.setFormatter(formatter)
    logger.addHandler(file_handler)
 
    return logger
 
 
def log_file_path(config_dir: Path) -> Path:
    return config_dir / "logs" / "lumidesk.log"
 