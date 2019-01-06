#pragma once

#include "Bhazel/Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace BZ {

    class BZ_API Log
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

#define BZ_CORE_FATAL(...) BZ::Log::getCoreLogger()->fatal(__VA_ARGS__)
#define BZ_CORE_ERROR(...) BZ::Log::getCoreLogger()->error(__VA_ARGS__)
#define BZ_CORE_WARN(...)  BZ::Log::getCoreLogger()->warn(__VA_ARGS__)
#define BZ_CORE_INFO(...)  BZ::Log::getCoreLogger()->info(__VA_ARGS__)
#define BZ_CORE_TRACE(...) BZ::Log::getCoreLogger()->trace(__VA_ARGS__)

#define BZ_FATAL(...) BZ::Log::getClientLogger()->fatal(__VA_ARGS__)
#define BZ_ERROR(...) BZ::Log::getClientLogger()->error(__VA_ARGS__)
#define BZ_WARN(...)  BZ::Log::getClientLogger()->warn(__VA_ARGS__)
#define BZ_INFO(...)  BZ::Log::getClientLogger()->info(__VA_ARGS__)
#define BZ_TRACE(...) BZ::Log::getClientLogger()->trace(__VA_ARGS__)