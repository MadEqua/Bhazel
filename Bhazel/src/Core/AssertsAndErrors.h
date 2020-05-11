#pragma once

#ifdef BZ_ASSERTS
    #define BZ_ASSERT(x, ...) if(!(x)) { BZ_LOG_CRITICAL("Assertion Failed! File: {0}. Line: {1}.", __FILE__, __LINE__); BZ_LOG_CRITICAL(__VA_ARGS__); __debugbreak(); }
    #define BZ_ASSERT_CORE(x, ...) if(!(x)) { BZ_LOG_CORE_CRITICAL("Assertion Failed! File: {0}. Line: {1}.", __FILE__, __LINE__); BZ_LOG_CORE_CRITICAL(__VA_ARGS__); __debugbreak(); }

    #define BZ_ASSERT_ALWAYS(...) BZ_ASSERT(0, __VA_ARGS__)
    #define BZ_ASSERT_ALWAYS_CORE(...) BZ_ASSERT_CORE(0, __VA_ARGS__)
#else
    #define BZ_ASSERT(x, ...)
    #define BZ_ASSERT_CORE(x, ...)

    #define BZ_ASSERT_ALWAYS(...)
    #define BZ_ASSERT_ALWAYS_CORE(...)
#endif

#define BZ_CRITICAL_ERROR(x, ...) if(!(x)) { BZ_LOG_CRITICAL("Critical Error! File: {0}. Line: {1}.", __FILE__, __LINE__); BZ_LOG_CRITICAL(__VA_ARGS__); exit(1); }
#define BZ_CRITICAL_ERROR_CORE(x, ...) if(!(x)) { BZ_LOG_CORE_CRITICAL("Critical Error! File: {0}. Line: {1}.", __FILE__, __LINE__); BZ_LOG_CORE_CRITICAL(__VA_ARGS__); exit(1); }

#define BZ_CRITICAL_ERROR_ALWAYS(...) BZ_CRITICAL_ERROR(0, __VA_ARGS__)
#define BZ_CRITICAL_ERROR_CORE_ALWAYS(...) BZ_CRITICAL_ERROR_CORE(0, __VA_ARGS__)