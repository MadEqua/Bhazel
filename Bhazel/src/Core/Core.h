#pragma once

#define ENABLE_PROFILER 0

#ifndef BZ_DIST
    #define BZ_ASSERTS
    #define BZ_FULL_LOGGER
    #define BZ_HOT_RELOAD_SHADERS

    #if ENABLE_PROFILER
        #define BZ_PROFILER
    #endif
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

    template<typename T>
    inline Ref<T> MakeRefNull() { return std::shared_ptr<T>(); }
}

#define EnumClassFlagOperators(e_) \
    inline e_ operator& (e_ a, e_ b){return static_cast<e_>(static_cast<int>(a)& static_cast<int>(b));} \
    inline e_ operator| (e_ a, e_ b){return static_cast<e_>(static_cast<int>(a)| static_cast<int>(b));} \
    inline e_& operator|= (e_& a, e_ b){a = a | b; return a;}; \
    inline e_& operator&= (e_& a, e_ b) { a = a & b; return a; }; \
    inline e_  operator~ (e_ a) { return static_cast<e_>(~static_cast<int>(a));} \
    inline bool isSet(uint8 mask, e_ flag) { return (mask & static_cast<uint8>(flag));} \
    inline uint8 flagsToMask(e_ e) {return static_cast<uint8>(e);}