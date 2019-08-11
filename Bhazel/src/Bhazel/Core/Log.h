#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace BZ {

    class Log
    {
    public:
        static void init();

        inline static std::shared_ptr<spdlog::logger>& getCoreLogger() { return coreLogger; }
        inline static std::shared_ptr<spdlog::logger>& getClientLogger() { return clientLogger; }

    private:
        static std::shared_ptr<spdlog::logger> coreLogger;
        static std::shared_ptr<spdlog::logger> clientLogger;
    };
}

#ifdef BZ_DIST
    #define BZ_LOG_CORE_CRITICAL(...)
    #define BZ_LOG_CORE_ERROR(...)
    #define BZ_LOG_CORE_WARN(...)
    #define BZ_LOG_CORE_INFO(...)
    #define BZ_LOG_CORE_DEBUG(...)
    #define BZ_LOG_CORE_TRACE(...)

    #define BZ_LOG_CRITICAL(...)
    #define BZ_LOG_ERROR(...)
    #define BZ_LOG_WARN(...)
    #define BZ_LOG_INFO(...)
    #define BZ_LOG_DEBUG(...)
    #define BZ_LOG_TRACE(...)
#else
    #define BZ_LOG_CORE_CRITICAL(...) BZ::Log::getCoreLogger()->critical(__VA_ARGS__)
    #define BZ_LOG_CORE_ERROR(...) BZ::Log::getCoreLogger()->error(__VA_ARGS__)
    #define BZ_LOG_CORE_WARN(...)  BZ::Log::getCoreLogger()->warn(__VA_ARGS__)
    #define BZ_LOG_CORE_INFO(...)  BZ::Log::getCoreLogger()->info(__VA_ARGS__)
    #define BZ_LOG_CORE_DEBUG(...) BZ::Log::getCoreLogger()->debug(__VA_ARGS__)
    #define BZ_LOG_CORE_TRACE(...) BZ::Log::getCoreLogger()->trace(__VA_ARGS__)

    #define BZ_LOG_CRITICAL(...) BZ::Log::getClientLogger()->critical(__VA_ARGS__)
    #define BZ_LOG_ERROR(...) BZ::Log::getClientLogger()->error(__VA_ARGS__)
    #define BZ_LOG_WARN(...)  BZ::Log::getClientLogger()->warn(__VA_ARGS__)
    #define BZ_LOG_INFO(...)  BZ::Log::getClientLogger()->info(__VA_ARGS__)
    #define BZ_LOG_DEBUG(...) BZ::Log::getClientLogger()->debug(__VA_ARGS__)
    #define BZ_LOG_TRACE(...) BZ::Log::getClientLogger()->trace(__VA_ARGS__)
#endif