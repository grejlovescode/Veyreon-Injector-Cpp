#include "ui/common.h"
#include <commdlg.h>
#include <cstring>
#include "core/config.h"

namespace vui {

std::string WideToUtf8(const std::wstring& w) {
    if (w.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string s(n - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &s[0], n, nullptr, nullptr);
    return s;
}

std::string FileName(const std::string& path) {
    size_t sl = path.find_last_of("\\/");
    return sl != std::string::npos ? path.substr(sl + 1) : path;
}

std::string Lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)tolower(c); });
    return s;
}

void SetStatus(const std::string& msg, Tone tone) {
    g_statusText = msg;
    g_statusTone = tone;
    SYSTEMTIME t;
    GetLocalTime(&t);
    char stamp[16];
    snprintf(stamp, sizeof stamp, "%02d:%02d:%02d", t.wHour, t.wMinute, t.wSecond);
    g_log.insert(g_log.begin(), { stamp, msg, tone });
    if (g_log.size() > 200) g_log.pop_back();
}

// ─── Actions ──────────────────────────────────────────────────────────────────
void FilterProcesses() {
    std::string q = Lower(g_searchBuf);
    g_filteredProcs.clear();
    for (auto& p : g_allProcs) {
        if (q.empty() || Lower(p.nameUtf8).find(q) != std::string::npos ||
            std::to_string(p.pid).find(q) != std::string::npos)
            g_filteredProcs.push_back(p);
    }
}

void RefreshProcesses() {
    g_allProcs = Injector::GetProcessList();
    FilterProcesses();
}

void GoTo(int page) {
    if (g_page == page) return;
    g_page = page;
    g_pageT = 0.0f;
    if (page == PageProcesses) {
        g_searchBuf[0] = '\0';
        g_scrollProcs = {};
        RefreshProcesses();
    }
}

void AddDlls() {
    OPENFILENAMEW ofn = {};
    static wchar_t fileBuf[4096];
    fileBuf[0] = L'\0';
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner   = g_hwnd;
    ofn.lpstrFilter = L"DLL Files (*.dll)\0*.dll\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile   = fileBuf;
    ofn.nMaxFile    = 4096;
    ofn.Flags       = OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
    if (!GetOpenFileNameW(&ofn)) { SetForegroundWindow(g_hwnd); return; }
    SetForegroundWindow(g_hwnd);

    auto add = [](const std::wstring& path) {
        if (std::find(g_dlls.begin(), g_dlls.end(), path) == g_dlls.end())
            g_dlls.push_back(path);
    };
    size_t before = g_dlls.size();
    wchar_t* p = fileBuf;
    std::wstring dir = p;
    p += dir.size() + 1;
    if (*p == L'\0') {
        add(dir);                       // single file
    } else {
        while (*p) {                    // multiple: first token is the directory
            add(dir + L'\\' + p);
            p += wcslen(p) + 1;
        }
    }
    if (g_dlls.size() > before)
        SetStatus("Added " + std::to_string(g_dlls.size() - before) + " module(s).", Tone::Info);
}

void RemoveDll(int i) {
    if (i < 0 || i >= (int)g_dlls.size()) return;
    g_dlls.erase(g_dlls.begin() + i);
    g_selDll = std::min(g_selDll, (int)g_dlls.size() - 1);
}

void Execute() {
    if (!g_hasProcess) { SetStatus("Select a target process first.", Tone::Error); return; }
    if (g_dlls.empty()) { SetStatus("Add at least one DLL.", Tone::Error); return; }

    int ok = 0, fail = 0;
    for (auto& dll : g_dlls) {
        auto res = Injector::Inject(g_selProcess.pid, dll);
        if (res.success) ok++;
        else {
            fail++;
            SetStatus(FileName(WideToUtf8(dll)) + ": " + res.message, Tone::Error);
        }
    }
    if (fail == 0)
        SetStatus("Injected " + std::to_string(ok) + " DLL(s) successfully!", Tone::Success);
    else
        SetStatus("Done: " + std::to_string(ok) + " ok, " + std::to_string(fail) + " failed.", Tone::Warn);

    if (g_stealthMode) {
        SetWindowDisplayAffinity(g_hwnd, 0x00000011u);
        ShowWindow(g_hwnd, SW_HIDE);
    }
    if (g_closeAfterInject && fail == 0)
        PostQuitMessage(0);
}

void SaveConfig() {
    Config cfg;
    cfg.dlls             = g_dlls;
    cfg.stealthMode      = g_stealthMode;
    cfg.closeAfterInject = g_closeAfterInject;
    cfg.autoInject       = g_autoInject;
    ConfigIO::Save(cfg);
    SetStatus("Configuration saved.", Tone::Success);
}

void LoadConfig() {
    Config cfg = ConfigIO::Load();
    g_dlls             = cfg.dlls;
    g_selDll           = -1;
    g_stealthMode      = cfg.stealthMode;
    g_closeAfterInject = cfg.closeAfterInject;
    g_autoInject       = cfg.autoInject;
    SetStatus("Configuration loaded.", Tone::Success);
}

} // namespace vui
