#pragma once

#include <Windows.h>

namespace shared::system::win32 {

void elevateProcessPriority(DWORD priority);

}