
#include "pch.h"


#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>
#pragma comment(lib, "dbghelp.lib")

extern "C" __declspec(dllexport) BOOL DumpMemory(const wchar_t* folder)
{
    // Process name = exe file name without path or extension
    wchar_t exe[MAX_PATH];
    GetModuleFileNameW(nullptr, exe, MAX_PATH);
    wchar_t* name = wcsrchr(exe, L'\\');
    name = name ? name + 1 : exe;
    wchar_t* dot = wcsrchr(name, L'.');
    if (dot) *dot = L'\0';

    // Build the folder path: folder + process name
    wchar_t dir[MAX_PATH];
    swprintf_s(dir, L"%s\\%s", folder, name);
    CreateDirectoryW(folder, nullptr);
    CreateDirectoryW(dir, nullptr);

    SYSTEMTIME st;
    GetLocalTime(&st);

    wchar_t path[MAX_PATH];
    swprintf_s(path, L"%s\\%s_%04d%02d%02d_%02d%02d%02d.dmp", dir, name,
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

    HANDLE file = CreateFileW(path, GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return FALSE;

    MINIDUMP_TYPE type = (MINIDUMP_TYPE)(MiniDumpWithFullMemory |
        MiniDumpWithHandleData |
        MiniDumpWithThreadInfo |
        MiniDumpWithFullMemoryInfo);

    BOOL ok = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
        file, type, nullptr, nullptr, nullptr);
    CloseHandle(file);
    return ok;
}

// Optional: dump automatically on an unhandled crash.
static LONG WINAPI CrashHandler(EXCEPTION_POINTERS*)
{
    DumpMemory(L"C:\\Windows\\Temp");
    return EXCEPTION_CONTINUE_SEARCH;
}

static DWORD WINAPI DumpThread(LPVOID)
{
    DumpMemory(L"C:\\Windows\\Temp");
    return 0;
}

BOOL APIENTRY DllMain(HMODULE, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        SetUnhandledExceptionFilter(CrashHandler);
        CreateThread(nullptr, 0, DumpThread, nullptr, 0, nullptr);
    }
    return TRUE;
}

