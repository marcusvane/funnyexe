// video_player.cpp
// Downloads an MP4 (if URL provided) and opens it with the default system video player in a loop.

#include <windows.h>
#include <shellapi.h>
#include <urlmon.h>
#include <shlwapi.h>

#include <string>
#include <iostream>

#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")

static bool is_url(const std::string& s) {
    return (s.rfind("http://", 0) == 0) || (s.rfind("https://", 0) == 0);
}

static std::string get_exe_dir() {
    char path[MAX_PATH];
    if (GetModuleFileNameA(NULL, path, MAX_PATH) == 0) return std::string();
    PathRemoveFileSpecA(path);
    return std::string(path);
}

static std::string get_temp_file(const char* name) {
    char tmp[MAX_PATH];
    DWORD len = GetTempPathA(MAX_PATH, tmp);
    std::string out = std::string(tmp, len) + name;
    return out;
}

static bool download_to(const std::string& url, const std::string& dest) {
    std::wcout << L"Downloading: " << std::wstring(url.begin(), url.end()) << L" -> " << std::wstring(dest.begin(), dest.end()) << std::endl;
    HRESULT hr = URLDownloadToFileA(NULL, url.c_str(), dest.c_str(), 0, NULL);
    return SUCCEEDED(hr);
}

static bool open_and_wait(const std::string& path) {
    SHELLEXECUTEINFOA sh = {0};
    sh.cbSize = sizeof(sh);
    sh.fMask = SEE_MASK_NOCLOSEPROCESS;
    sh.hwnd = NULL;
    sh.lpVerb = "open";
    sh.lpFile = path.c_str();
    sh.nShow = SW_SHOWNORMAL;

    if (!ShellExecuteExA(&sh)) {
        DWORD err = GetLastError();
        std::cerr << "ShellExecuteEx failed: " << err << std::endl;
        return false;
    }

    if (sh.hProcess) {
        WaitForSingleObject(sh.hProcess, INFINITE);
        CloseHandle(sh.hProcess);
    } else {
        // If we didn't get a process handle, sleep briefly to avoid tight-looping
        Sleep(2000);
    }

    return true;
}

int main(int argc, char** argv) {
    std::string requested;
    if (argc > 1) requested = argv[1];

    std::string video_path;

    if (requested.empty()) {
        // default to thugmain.mp4 in the exe dir if present
        std::string exe_dir = get_exe_dir();
        std::string local = exe_dir + "\\thugmain.mp4";
        if (PathFileExistsA(local.c_str())) {
            video_path = local;
        } else {
            std::cerr << "No video provided and thugmain.mp4 not found in exe directory." << std::endl;
            return 1;
        }
    } else if (is_url(requested)) {
        std::string tmp = get_temp_file("funnyexe_video.mp4");
        if (!download_to(requested, tmp)) {
            std::cerr << "Download failed." << std::endl;
            return 1;
        }
        video_path = tmp;
    } else {
        video_path = requested;
        if (!PathFileExistsA(video_path.c_str())) {
            std::cerr << "File not found: " << video_path << std::endl;
            return 1;
        }
    }

    std::cout << "Playing: " << video_path << std::endl;

    // Loop: open default player and wait until it exits, then reopen
    while (true) {
        if (!open_and_wait(video_path)) {
            std::cerr << "Failed to open player. Exiting." << std::endl;
            return 1;
        }
        // small pause before restarting
        Sleep(500);
    }

    return 0;
}
