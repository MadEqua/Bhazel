#pragma once

#include <spdlog/spdlog.h>

#include "Core/Singleton.h"


namespace BZ {

class Log {
    BZ_GENERATE_SINGLETON(Log)

  public:
    std::shared_ptr<spdlog::logger> &getCoreLogger() { return coreLogger; }
    std::shared_ptr<spdlog::logger> &getClientLogger() { return clientLogger; }

  private:
    std::shared_ptr<spdlog::logger> coreLogger;
    std::shared_ptr<spdlog::logger> clientLogger;
};
}

#define BZ_LOG_CORE_CRITICAL(...) BZ::Log::get().getCoreLogger()->critical(__VA_ARGS__)
#define BZ_LOG_CORE_ERROR(...) BZ::Log::get().getCoreLogger()->error(__VA_ARGS__)
#define BZ_LOG_CORE_WARN(...) BZ::Log::get().getCoreLogger()->warn(__VA_ARGS__)

#define BZ_LOG_CRITICAL(...) BZ::Log::get().getClientLogger()->critical(__VA_ARGS__)
#define BZ_LOG_ERROR(...) BZ::Log::get().getClientLogger()->error(__VA_ARGS__)
#define BZ_LOG_WARN(...) BZ::Log::get().getClientLogger()->warn(__VA_ARGS__)

#ifdef BZ_FULL_LOGGER
    #define BZ_LOG_CORE_INFO(...) BZ::Log::get().getCoreLogger()->info(__VA_ARGS__)
    #define BZ_LOG_CORE_DEBUG(...) BZ::Log::get().getCoreLogger()->debug(__VA_ARGS__)
    #define BZ_LOG_CORE_TRACE(...) BZ::Log::get().getCoreLogger()->trace(__VA_ARGS__)

    #define BZ_LOG_INFO(...) BZ::Log::get().getClientLogger()->info(__VA_ARGS__)
    #define BZ_LOG_DEBUG(...) BZ::Log::get().getClientLogger()->debug(__VA_ARGS__)
    #define BZ_LOG_TRACE(...) BZ::Log::get().getClientLogger()->trace(__VA_ARGS__)
#else
    #define BZ_LOG_CORE_INFO(...)
    #define BZ_LOG_CORE_DEBUG(...)
    #define BZ_LOG_CORE_TRACE(...)

    #define BZ_LOG_INFO(...)
    #define BZ_LOG_DEBUG(...)
    #define BZ_LOG_TRACE(...)
#endif


///////////////////////////////////////////////////////////////////////
// Custom Printers
///////////////////////////////////////////////////////////////////////
#include <spdlog/fmt/ostr.h>

inline std::ostream &operator<<(std::ostream &os, const glm::vec3 &vec) {
    return os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
}

inline std::ostream &operator<<(std::ostream &os, const glm::vec4 &vec) {
    return os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
}

inline std::ostream &operator<<(std::ostream &os, const glm::mat3 &mat) {
    return os << "|" << mat[0].x << " " << mat[1].x << " " << mat[2].x << "|" << std::endl
              << "|" << mat[0].y << " " << mat[1].y << " " << mat[2].y << "|" << std::endl
              << "|" << mat[0].z << " " << mat[1].z << " " << mat[2].z << "|";
}

inline std::ostream &operator<<(std::ostream &os, const glm::mat4 &mat) {
    return os << "|" << mat[0].x << " " << mat[1].x << " " << mat[2].x << " " << mat[3].x << "|" << std::endl
              << "|" << mat[0].y << " " << mat[1].y << " " << mat[2].y << " " << mat[3].y << "|" << std::endl
              << "|" << mat[0].z << " " << mat[1].z << " " << mat[2].z << " " << mat[3].z << "|" << std::endl
              << "|" << mat[0].w << " " << mat[1].w << " " << mat[2].w << " " << mat[3].w << "|";
}