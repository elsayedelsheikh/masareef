# Building the Windows version

Run **from Windows PowerShell** (the script downloads its own Qt + MinGW on
first run, ~1 GB into `%LOCALAPPDATA%\masareef-build\qt`):

```powershell
cd \\wsl$\Ubuntu\home\sayed\Projects\masareef\packaging   # or a Windows clone of the repo
.\build-windows.ps1
```

Requirements on the Windows side:

- Python 3 (`winget install Python.Python.3.12` or the Microsoft Store)
- CMake (`winget install Kitware.CMake`)
- Internet access on the first run (Qt download)

The result is `Masareef-<version>-win64.zip` — a portable folder, no
installer. Unzip anywhere and run `Masareef\masareef.exe`. The app stores its
database under `%APPDATA%\Masareef` (automatic startup backups included), and
settings in the registry.

Tip: building directly on a `\\wsl$` path works but is slow; a Windows-side
clone of the repository builds noticeably faster.
