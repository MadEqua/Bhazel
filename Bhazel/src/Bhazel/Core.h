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

#ifdef BZ_ENABLE_ASSERTS
    #define BZ_ASSERT(x, ...) { if(!(x)) { HZ_ERROR("Assertion  Failed: {0}", __VA_ARGS__); __debugbreak(); } }
    #define BZ_CORE_ASSERT(x, ...) { if(!(x)) { HZ_CORE_ERROR("Assertion  Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
    #define BZ_ASSERT(x, ...)
    #define BZ_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define BZ_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)
