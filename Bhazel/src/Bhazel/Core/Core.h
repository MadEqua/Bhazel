#pragma once


#ifdef BZ_PLATFORM_WINDOWS
    #ifdef BZ_DYNAMIC_LINK
        #ifdef BZ_BUILD_DLL
            #define BZ_API __declspec(dllexport)
        #else
            #define BZ_API __declspec(dllimport)
        #endif
    #else
        #define BZ_API
#endif
#else
    #error Windows only for now.
#endif


#ifdef BZ_DIST
    #define BZ_ASSERT(x, ...)
    #define BZ_CORE_ASSERT(x, ...)

    #define BZ_ASSERT_ALWAYS(...)
    #define BZ_CORE_ASSERT_ALWAYS(...)
#else
    #define BZ_ASSERT(x, ...) { if(!(x)) { BZ_LOG_CRITICAL("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
    #define BZ_CORE_ASSERT(x, ...) { if(!(x)) { BZ_LOG_CORE_CRITICAL("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }

    #define BZ_ASSERT_ALWAYS(...) { BZ_LOG_CRITICAL("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); }
    #define BZ_CORE_ASSERT_ALWAYS(...) { BZ_LOG_CORE_CRITICAL("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); }
#endif

#define BIT(x) (1 << x)

#define BZ_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

#include <memory>
namespace BZ {

    template<typename T>
    using Scope = std::unique_ptr<T>;

    template<typename T, class... Args>
    inline Scope<T> MakeScope(Args&&... args) { return std::make_unique<T>(std::forward<Args>(args)...); }

    template<typename T>
    using Ref = std::shared_ptr<T>;

    template<typename T, class... Args>
    inline Ref<T> MakeRef(Args&&... args) { return std::make_shared<T>(std::forward<Args>(args)...); }
}