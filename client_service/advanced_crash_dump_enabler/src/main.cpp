#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    std::vector<std::string> exeNames = {
        "SquadOV.exe",
        "squadov_client_service.exe"
    };

    const std::string dumpFolder = "%APPDATA%\\SquadOV\\WERDumps";
    const DWORD dumpCount = 10;
    const DWORD dumpType = 1;

    std::vector<HKEY> keysToClose;
    for (const auto& exe : exeNames) {
        std::string subKey = "SOFTWARE\\Microsoft\\Windows\\Windows Error Reporting\\LocalDumps\\" + exe;

        HKEY exeKey;
        if (RegCreateKeyExA(
                HKEY_LOCAL_MACHINE,
                subKey.c_str(),
                0,
                NULL,
                0,
                KEY_ALL_ACCESS,
                nullptr,
                &exeKey,
                nullptr
            ) != ERROR_SUCCESS
        ) {
            std::cerr << "Failed to set dump key for " << exe << std::endl;
        } else {
            RegSetValueExA(exeKey, "DumpFolder", 0, REG_EXPAND_SZ, (const BYTE*)dumpFolder.c_str(), dumpFolder.size() + 1);
            RegSetValueExA(exeKey, "DumpCount", 0, REG_DWORD, (const BYTE*)&dumpCount, sizeof(DWORD));
            RegSetValueExA(exeKey, "DumpType", 0, REG_DWORD, (const BYTE*)&dumpType, sizeof(DWORD));
            keysToClose.push_back(exeKey); 
        };
    }

    for (const auto& k : keysToClose) {
        RegCloseKey(k);
    }

    return 0;
}