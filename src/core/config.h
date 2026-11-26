#pragma once
#include <string>
#include <vector>

struct Config {
    std::vector<std::wstring> dlls;
    bool stealthMode        = false;
    bool closeAfterInject   = false;
    bool autoInject         = false;
};

namespace ConfigIO {
    void Save(const Config& cfg);
    Config Load();
    // Returns directory where the exe lives (config.ini placed there)
    std::wstring GetExeDir();
}
