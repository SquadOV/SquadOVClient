#include "shared/errors/error.h"
#include <codecvt>

#ifndef UNICODE
#define UNICODE
#endif

namespace shared::errors {

#ifdef _WIN32
std::string getWin32ErrorAsString() {
    const auto err = GetLastError();
    if (err == ERROR_SUCCESS) {
        return "";
    }

    LPWSTR pBuffer = NULL;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&pBuffer, 
        0,
        nullptr
    );

    if (!pBuffer) {
        LOG_WARNING("FAILED TO GET WIN32 ERROR STRING: " << err << std::endl);
        return "";
    }

    std::wstring errStr(pBuffer);
    LocalFree(pBuffer);
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(errStr);
}
#endif

}