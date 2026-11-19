// Minimal Win32 surface NiflyDLL touches, mapped onto POSIX/dyld.
// Only FindProjectRoot() needs it: it asks which module a symbol lives in, to find skeletons/ next to the library.
#pragma once
#include <dlfcn.h>
#include <cstring>
#include <climits>

#define MAX_PATH PATH_MAX
typedef void* HMODULE;
typedef const char* LPCSTR;
typedef char* LPSTR;
#define GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS 0x4
#define GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT 0x2

// "Handle" is any address inside the module; dladdr resolves it back to the image path.
inline int GetModuleHandleExA(unsigned, LPCSTR addr, HMODULE* hm) { *hm = (HMODULE)addr; return 1; }
inline unsigned GetModuleFileNameA(HMODULE hm, LPSTR buf, unsigned size) {
    Dl_info info;
    if (!dladdr((const void*)hm, &info) || !info.dli_fname) return 0;
    std::strncpy(buf, info.dli_fname, size - 1);
    buf[size - 1] = 0;
    return (unsigned)std::strlen(buf);
}
