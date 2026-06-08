# funnyexe

This repository includes a fullscreen video player for `thugmain.mp4`.

## Usage

1. Install dependencies:

```bash
pip install -r requirements.txt
```

2. Run the player:

```bash
python video_player.py
```

3. Or specify a different video file:

```bash
python video_player.py path/to/video.mp4
```

You can also pass a remote URL (for example a Discord CDN link). The player will download it to the system temp folder and play it in a loop:

```bash
python video_player.py "https://cdn.discordapp.com/.../thugmain.mp4"
```

## Build a Windows EXE with PyInstaller

This repository is currently in a Linux environment, so a native Windows `.exe` cannot be built here. To create a Windows executable, run the following on a Windows machine or Windows build environment.

Install PyInstaller:

```bat
pip install pyinstaller
```

Create a single-file executable and bundle the video:

```bat
pyinstaller --onefile --noconsole --add-data "thugmain.mp4;." video_player.py
```

Alternatively, use the prepared spec file:

```bat
pyinstaller video_player.spec
```

Then the generated EXE will be in `dist\video_player.exe`.

> On Windows, pressing `Alt+F4` closes the fullscreen window through the OS window manager. The player also supports `Esc` or `q` to exit.

## CI Windows EXE build

This repo includes a GitHub Actions workflow that builds a standalone Windows `.exe` using PyInstaller and uploads it as an artifact. Trigger the workflow from the Actions tab or push to `main`.

Workflow: `.github/workflows/build-windows.yml`

After the workflow completes, download the artifact named `video_player-windows-exe` from the run and run `video_player.exe` on Windows.

### Native C++ build (no Python, no console)

I added a native C++ program that embeds `thugmain.mp4` directly into the executable and plays it repeatedly every 2 seconds with the system default video player. The EXE runs as a GUI application (no console window) and appears in Task Manager as a normal process.

- Source: `src/video_player_embedded.cpp`
- Embedded header: `src/thugmain_mp4.h` (generated from `thugmain.mp4`)
- Compiled: `dist/video_player.exe` (cross-compiled with MinGW on Linux)

**Features:**
- No console window
- Embedded MP4 (no external files needed)
- Launches default video player every 2 seconds
- Exit with Alt+F4

To build locally on Windows with the Visual Studio Developer Command Prompt:

```bat
cl /EHsc /O2 /Fevideos_player.exe src\video_player_embedded.cpp shell32.lib shlwapi.lib
```

To cross-compile on Linux with MinGW:

```bash
x86_64-w64-mingw32-g++ -static -O2 -mwindows -o video_player.exe src/video_player_embedded.cpp -lshell32 -lshlwapi
```
