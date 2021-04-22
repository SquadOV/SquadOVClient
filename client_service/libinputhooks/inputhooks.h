#pragma once

#include <Windows.h>

#ifndef DLLEXPORT
#ifdef COMPILING_LIBRARY
#define DLLEXPORT extern "C" __declspec(dllexport)
#else
#define DLLEXPORT
#define DLLEXPORT extern "C" __declspec(dllimport)
#endif
#endif

extern HWND targetWindow;

DLLEXPORT LRESULT __cdecl squadOvKeyboardProc(int code, WPARAM wParam, LPARAM lParam);
DLLEXPORT LRESULT __cdecl squadOvMouseProc(int code, WPARAM wParam, LPARAM lParam);