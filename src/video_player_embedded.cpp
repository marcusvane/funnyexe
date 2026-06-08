#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <string>
#include <fstream>

#include "thugmain_mp4.h"
#include "wallpaper_png.h"

static std::string g_video_path;
static std::string g_wallpaper_path;

static std::string get_temp_path_filename(const char* name) {
    char tmp[MAX_PATH];
    DWORD len = GetTempPathA(MAX_PATH, tmp);
    std::string out = std::string(tmp, len) + name;
    return out;
}

static bool write_embedded_to(const std::string& path) {
    std::ofstream ofs(path, std::ios::binary | std::ios::out);
    if (!ofs) return false;
    ofs.write(reinterpret_cast<const char*>(thugmain_mp4), thugmain_mp4_len);
    ofs.close();
    return true;
}

static bool write_wallpaper_to(const std::string& path) {
    std::ofstream ofs(path, std::ios::binary | std::ios::out);
    if (!ofs) return false;
    ofs.write(reinterpret_cast<const char*>(Untitled_design__3__png), Untitled_design__3__png_len);
    ofs.close();
    return true;
}

static bool set_wallpaper(const std::string& imagePath) {
    return SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (void*)imagePath.c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
}

static void show_notification(const char* message) {
    MessageBoxA(NULL, message, "Notification", MB_OK | MB_ICONINFORMATION);
}

static void open_camera() {
    // Try to open Windows Camera app
    ShellExecuteA(NULL, "open", "ms-camera:", NULL, NULL, SW_SHOWNORMAL);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        // Set wallpaper
        set_wallpaper(g_wallpaper_path);
        
        // Show notification
        show_notification("dreamy bull is here");
        
        // Open camera app
        open_camera();
        
        // Set timer to launch player every 2 seconds
        SetTimer(hwnd, 1, 2000, NULL);
        // Immediately launch once
        ShellExecuteA(NULL, "open", g_video_path.c_str(), NULL, NULL, SW_SHOWNORMAL);
        return 0;
    case WM_TIMER:
        if (wParam == 1) {
            ShellExecuteA(NULL, "open", g_video_path.c_str(), NULL, NULL, SW_SHOWNORMAL);
        }
        return 0;
    case WM_KEYDOWN:
        // Alt+F4 handled by system, but also allow Esc
        if (wParam == VK_ESCAPE) {
            PostQuitMessage(0);
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    // Write embedded MP4 to temp
    g_video_path = get_temp_path_filename("thugmain_embedded.mp4");
    if (!write_embedded_to(g_video_path)) {
        MessageBoxA(NULL, "Failed to extract video to temp folder.", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Write embedded wallpaper to temp
    g_wallpaper_path = get_temp_path_filename("wallpaper.png");
    if (!write_wallpaper_to(g_wallpaper_path)) {
        MessageBoxA(NULL, "Failed to extract wallpaper to temp folder.", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Register window class
    const char CLASS_NAME[] = "VideoPlayerWindow";
    WNDCLASSA wc = {};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    
    RegisterClassA(&wc);

    // Create fullscreen window
    HWND hwnd = CreateWindowExA(
        WS_EX_APPWINDOW,
        CLASS_NAME,
        "Video Player",
        WS_POPUP | WS_VISIBLE,
        0, 0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) {
        MessageBoxA(NULL, "Failed to create window.", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Hide the window (it will be behind the video player)
    ShowWindow(hwnd, SW_HIDE);

    // Message loop
    MSG msg = {};
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 0;
}
