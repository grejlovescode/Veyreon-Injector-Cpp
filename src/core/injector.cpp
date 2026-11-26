#include "injector.h"
#include <algorithm>

static std::string WideToUtf8(const std::wstring& w) {
    if (w.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string s(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, s.data(), len, nullptr, nullptr);
    return s;
}

namespace Injector {

std::vector<ProcessInfo> GetProcessList() {
    std::vector<ProcessInfo> list;

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return list;

    PROCESSENTRY32W entry = {};
    entry.dwSize = sizeof(entry);

    if (Process32FirstW(snap, &entry)) {
        do {
            ProcessInfo p;
            p.pid     = entry.th32ProcessID;
            p.name    = entry.szExeFile;
            p.nameUtf8 = WideToUtf8(p.name);
            list.push_back(std::move(p));
        } while (Process32NextW(snap, &entry));
    }
    CloseHandle(snap);

    std::sort(list.begin(), list.end(), [](const ProcessInfo& a, const ProcessInfo& b) {
        return a.nameUtf8 < b.nameUtf8;
    });
    return list;
}

InjectionResult Inject(DWORD pid, const std::wstring& dllPath) {
    if (GetFileAttributesW(dllPath.c_str()) == INVALID_FILE_ATTRIBUTES)
        return { false, "DLL file not found." };

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess)
        return { false, "OpenProcess failed. Error: " + std::to_string(GetLastError()) };

    size_t pathSize = (dllPath.size() + 1) * sizeof(wchar_t);
    void*  remote   = VirtualAllocEx(hProcess, nullptr, pathSize,
                                      MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remote) {
        CloseHandle(hProcess);
        return { false, "VirtualAllocEx failed." };
    }

    if (!WriteProcessMemory(hProcess, remote, dllPath.c_str(), pathSize, nullptr)) {
        VirtualFreeEx(hProcess, remote, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return { false, "WriteProcessMemory failed." };
    }

    HMODULE   k32      = GetModuleHandleA("kernel32.dll");
    LPTHREAD_START_ROUTINE loadLibW =
        (LPTHREAD_START_ROUTINE)GetProcAddress(k32, "LoadLibraryW");

    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, loadLibW, remote, 0, nullptr);
    if (!hThread) {
        VirtualFreeEx(hProcess, remote, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return { false, "CreateRemoteThread failed. Error: " + std::to_string(GetLastError()) };
    }

    WaitForSingleObject(hThread, 5000);

    DWORD exitCode = 0;
    GetExitCodeThread(hThread, &exitCode);

    CloseHandle(hThread);
    VirtualFreeEx(hProcess, remote, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    if (exitCode == 0)
        return { false, "LoadLibraryW returned NULL (DLL load failed)." };

    return { true, "Injection successful." };
}

} // namespace Injector
