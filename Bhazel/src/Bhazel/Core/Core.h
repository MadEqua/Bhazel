#pragma once

#ifdef BZ_DIST
    #define BZ_ASSERT(x, ...)
    #define BZ_ASSERT_CORE(x, ...)

    #define BZ_ASSERT_ALWAYS(...)
    #define BZ_ASSERT_ALWAYS_CORE(...)
#else
    #define BZ_ASSERT(x, ...) if(!(x)) { BZ_LOG_CRITICAL("Assertion Failed! File: {0}. Line: {1}.", __FILE__, __LINE__); BZ_LOG_CRITICAL(__VA_ARGS__); __debugbreak(); }
    #define BZ_ASSERT_CORE(x, ...) if(!(x)) { BZ_LOG_CORE_CRITICAL("Assertion Failed! File: {0}. Line: {1}.", __FILE__, __LINE__); BZ_LOG_CORE_CRITICAL(__VA_ARGS__); __debugbreak(); }

    #define BZ_ASSERT_ALWAYS(...) BZ_ASSERT(0, __VA_ARGS__)
    #define BZ_ASSERT_ALWAYS_CORE(...) BZ_ASSERT_CORE(0, __VA_ARGS__)
#endif

#define BIT(x) (1 << x)

#define BZ_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

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

#define EnumClassFlagOperators(e_) \
    inline e_ operator& (e_ a, e_ b){return static_cast<e_>(static_cast<int>(a)& static_cast<int>(b));} \
    inline e_ operator| (e_ a, e_ b){return static_cast<e_>(static_cast<int>(a)| static_cast<int>(b));} \
    inline e_& operator|= (e_& a, e_ b){a = a | b; return a;}; \
    inline e_& operator&= (e_& a, e_ b) { a = a & b; return a; }; \
    inline e_  operator~ (e_ a) { return static_cast<e_>(~static_cast<int>(a));} \
    inline bool isSet(uint8 mask, e_ flag) { return (mask & static_cast<uint8>(flag));} \
    inline uint8 flagsToMask(e_ e) {return static_cast<uint8>(e);}