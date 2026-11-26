#include "config.h"
#include <windows.h>
#include <string>

namespace ConfigIO {

std::wstring GetExeDir() {
    wchar_t buf[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, buf, MAX_PATH);
    std::wstring path(buf);
    size_t sl = path.rfind(L'\\');
    return sl != std::wstring::npos ? path.substr(0, sl) : path;
}

static std::wstring IniPath() {
    return GetExeDir() + L"\\config.ini";
}

static std::wstring ReadStr(const wchar_t* section, const wchar_t* key,
                             const wchar_t* def, const wchar_t* file) {
    wchar_t buf[4096] = {};
    GetPrivateProfileStringW(section, key, def, buf, 4096, file);
    return buf;
}

void Save(const Config& cfg) {
    auto path = IniPath();
    auto boolStr = [](bool b) -> const wchar_t* { return b ? L"1" : L"0"; };

    WritePrivateProfileStringW(L"Settings", L"StealthMode",
                               boolStr(cfg.stealthMode), path.c_str());
    WritePrivateProfileStringW(L"Settings", L"CloseAfterInject",
                               boolStr(cfg.closeAfterInject), path.c_str());
    WritePrivateProfileStringW(L"Settings", L"AutoInject",
                               boolStr(cfg.autoInject), path.c_str());

    WritePrivateProfileStringW(L"DLLs", L"Count",
                               std::to_wstring(cfg.dlls.size()).c_str(), path.c_str());
    for (int i = 0; i < (int)cfg.dlls.size(); i++) {
        std::wstring key = L"DLL" + std::to_wstring(i);
        WritePrivateProfileStringW(L"DLLs", key.c_str(), cfg.dlls[i].c_str(), path.c_str());
    }
}

Config Load() {
    Config cfg;
    auto path = IniPath();

    cfg.stealthMode      = ReadStr(L"Settings", L"StealthMode",      L"0", path.c_str()) == L"1";
    cfg.closeAfterInject = ReadStr(L"Settings", L"CloseAfterInject", L"0", path.c_str()) == L"1";
    cfg.autoInject       = ReadStr(L"Settings", L"AutoInject",       L"0", path.c_str()) == L"1";

    int count = _wtoi(ReadStr(L"DLLs", L"Count", L"0", path.c_str()).c_str());
    for (int i = 0; i < count; i++) {
        std::wstring key = L"DLL" + std::to_wstring(i);
        std::wstring val = ReadStr(L"DLLs", key.c_str(), L"", path.c_str());
        if (!val.empty()) cfg.dlls.push_back(val);
    }
    return cfg;
}

} // namespace ConfigIO
