from __future__ import annotations
 
import tkinter as tk
from tkinter import messagebox
from typing import Callable
 
 
def open_settings_window(config, on_saved: Callable[[], None]) -> None:
    root = tk.Tk()
    root.title("LumiDesk Settings")
    root.resizable(False, False)
 
    pad = {"padx": 10, "pady": 6}
 
    tk.Label(root, text="Weather latitude").grid(row=0, column=0, sticky="w", **pad)
    lat_var = tk.StringVar(value=str(config.get("weather.lat") or ""))
    tk.Entry(root, textvariable=lat_var, width=20).grid(row=0, column=1, **pad)
 
    tk.Label(root, text="Weather longitude").grid(row=1, column=0, sticky="w", **pad)
    lon_var = tk.StringVar(value=str(config.get("weather.lon") or ""))
    tk.Entry(root, textvariable=lon_var, width=20).grid(row=1, column=1, **pad)
 
    tk.Label(root, text="Backend port").grid(row=2, column=0, sticky="w", **pad)
    port_var = tk.StringVar(value=str(config.get("backend.port")))
    tk.Entry(root, textvariable=port_var, width=20).grid(row=2, column=1, **pad)
 
    def save():
        try:
            lat = float(lat_var.get()) if lat_var.get().strip() else None
            lon = float(lon_var.get()) if lon_var.get().strip() else None
            port = int(port_var.get())
        except ValueError:
            messagebox.showerror("LumiDesk", "Latitude/longitude must be numbers, port must be an integer.")
            return
 
        if not (0 < port < 65536):
            messagebox.showerror("LumiDesk", "Port must be between 1 and 65535.")
            return
 
        config.set("weather.lat", lat, save=False)
        config.set("weather.lon", lon, save=False)
        config.set("backend.port", port)  # last call in the batch triggers the save
 
        root.destroy()
        on_saved()
 
    tk.Button(root, text="Save", command=save).grid(row=3, column=0, columnspan=2, pady=10)
    root.mainloop()
 