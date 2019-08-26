#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace BZ {

    class Log
    {
        BZ_GENERATE_SINGLETON(Log)
    
    public:
        std::shared_ptr<spdlog::logger>& getCoreLogger() { return coreLogger; }
        std::shared_ptr<spdlog::logger>& getClientLogger() { return clientLogger; }

    private:
        std::shared_ptr<spdlog::logger> coreLogger;
        std::shared_ptr<spdlog::logger> clientLogger;
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
    #define BZ_LOG_CORE_CRITICAL(...) BZ::Log::getInstance().getCoreLogger()->critical(__VA_ARGS__)
    #define BZ_LOG_CORE_ERROR(...) BZ::Log::getInstance().getCoreLogger()->error(__VA_ARGS__)
    #define BZ_LOG_CORE_WARN(...)  BZ::Log::getInstance().getCoreLogger()->warn(__VA_ARGS__)
    #define BZ_LOG_CORE_INFO(...)  BZ::Log::getInstance().getCoreLogger()->info(__VA_ARGS__)
    #define BZ_LOG_CORE_DEBUG(...) BZ::Log::getInstance().getCoreLogger()->debug(__VA_ARGS__)
    #define BZ_LOG_CORE_TRACE(...) BZ::Log::getInstance().getCoreLogger()->trace(__VA_ARGS__)

    #define BZ_LOG_CRITICAL(...) BZ::Log::getInstance().getClientLogger()->critical(__VA_ARGS__)
    #define BZ_LOG_ERROR(...) BZ::Log::getInstance().getClientLogger()->error(__VA_ARGS__)
    #define BZ_LOG_WARN(...)  BZ::Log::getInstance().getClientLogger()->warn(__VA_ARGS__)
    #define BZ_LOG_INFO(...)  BZ::Log::getInstance().getClientLogger()->info(__VA_ARGS__)
    #define BZ_LOG_DEBUG(...) BZ::Log::getInstance().getClientLogger()->debug(__VA_ARGS__)
    #define BZ_LOG_TRACE(...) BZ::Log::getInstance().getClientLogger()->trace(__VA_ARGS__)
#endif