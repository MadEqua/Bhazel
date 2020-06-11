#pragma once

#define ENABLE_PROFILER 0

#ifndef BZ_DIST
    #define BZ_ASSERTS
    #define BZ_FULL_LOGGER
    #define BZ_HOT_RELOAD_SHADERS
    #define BZ_GRAPHICS_DEBUG

    #if ENABLE_PROFILER
        #define BZ_PROFILER
    #endif
#endif


#define BZ_BIT(x) (1 << x)
#define BZ_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)
#define BZ_FLAG_CHECK(mask, flag) static_cast<bool>(mask & flag)
#define BZ_BIT_MASK(type, oneCount) ((type)(-((oneCount) != 0))) & (((type)-1) >> ((sizeof(type) * 8) - (oneCount)))

#define BZ_NON_COPYABLE(T)              \
    T(const T &) = delete;              \
    void operator=(const T &) = delete; \
    T(const T &&) = delete;             \
    void operator=(const T &&) = delete;

#define BZ_ENGINE BZ::Engine::get()
#define BZ_GRAPHICS_CTX BZ::Engine::get().getGraphicsContext()

namespace BZ {

template <typename T> using Scope = std::unique_ptr<T>;

template <typename T, class... Args> inline Scope<T> MakeScope(Args &&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T> using Ref = std::shared_ptr<T>;

template <typename T, class... Args> inline Ref<T> MakeRef(Args &&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T> inline Ref<T> MakeRef(T *obj) {
    return std::shared_ptr<T>(obj);
}

template <typename T> inline Ref<T> MakeRefNull() {
    return std::shared_ptr<T>();
}
}