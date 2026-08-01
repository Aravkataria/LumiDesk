# PyInstaller spec for LumiDesk.exe
#
# Build from inside the project folder (the same folder as bootstrap.py,
# app.py, config.py, etc.) with:
#   pyinstaller lumidesk.spec
#
# Honest caveat: winsdk/pycaw/comtypes lean on Windows COM/WinRT, which
# PyInstaller's static analysis doesn't always fully resolve on its own.
# Treat the hiddenimports list below as a starting point — expect to run
# the built exe, see an ImportError in the log
# (%APPDATA%/LumiDesk/logs/lumidesk.log), add one more hidden import,
# and rebuild, a few times before it's clean.

from pathlib import Path
from PyInstaller.utils.hooks import collect_all

block_cipher = None
project_root = Path(SPECPATH)  # everything lives in this one folder

backend_datas = [
    (str(project_root / "app.py"), "."),
    (str(project_root / "media_service.py"), "."),
    (str(project_root / "lyrics_service.py"), "."),
    (str(project_root / "models.py"), "."),
    (str(project_root / "cache.py"), "."),
    (str(project_root / "providers.py"), "."),
]

routes_dir = project_root / "routes"
if routes_dir.is_dir():
    backend_datas.append((str(routes_dir), "routes"))

hidden_imports = [
    # uvicorn's protocol/loop selection is dynamic and commonly needs
    # to be spelled out explicitly for a frozen build.
    "uvicorn.loops.auto",
    "uvicorn.protocols.http.auto",
    "uvicorn.protocols.websockets.auto",
    "uvicorn.lifespan.on",
    # winsdk / pycaw / comtypes: COM-heavy, may need more entries here
    # discovered from ImportErrors in the built exe's log.
    "winsdk.windows.media.control",
    "pycaw.pycaw",
    "comtypes.stream",
]

binaries = []

# These four ship non-Python data files (dictionaries, lookup tables)
# that PyInstaller's default analysis won't find on its own — this is
# exactly what caused the "kanwadict4.db not found" crash. collect_all
# pulls in their code, data, AND any compiled binaries together.
for pkg in ("pykakasi", "pypinyin", "korean_romanizer", "unidecode"):
    pkg_datas, pkg_binaries, pkg_hidden = collect_all(pkg)
    backend_datas.extend(pkg_datas)
    binaries.extend(pkg_binaries)
    hidden_imports.extend(pkg_hidden)

a = Analysis(
    [str(project_root / "bootstrap.py")],
    pathex=[str(project_root)],
    binaries=binaries,
    datas=backend_datas,
    hiddenimports=hidden_imports,
    hookspath=[],
    runtime_hooks=[],
    excludes=[],
    cipher=block_cipher,
    noarchive=False,
)

pyz = PYZ(a.pure, a.zipped_data, cipher=block_cipher)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.zipfiles,
    a.datas,
    [],
    name="LumiDesk",
    console=False,  # windowed — no terminal flashes on launch
    onefile=True,
    icon=None,      # drop an .ico path here if/when you make one
)
