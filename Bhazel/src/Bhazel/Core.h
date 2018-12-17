#pragma once

#ifdef BZ_PLATFORM_WINDOWS
    #ifdef BZ_BUILD_DLL
        #define BZ_API __declspec(dllexport)
    #else
        #define BZ_API __declspec(dllimport)
    #endif
#else
    #error Windows only for now.
#endif