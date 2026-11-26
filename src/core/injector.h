#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>

struct ProcessInfo {
    DWORD       pid;
    std::wstring name;
    std::string  nameUtf8;  // for ImGui (needs UTF-8)
};

struct InjectionResult {
    bool        success;
    std::string message;
};

namespace Injector {
    std::vector<ProcessInfo> GetProcessList();
    InjectionResult          Inject(DWORD pid, const std::wstring& dllPath);
}
