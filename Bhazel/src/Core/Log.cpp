#include "bzpch.h"

#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>


namespace BZ {

Log::Log() {
    spdlog::set_pattern("%^[%T] %n: %v%$");

    coreLogger = spdlog::stdout_color_mt("BHAZEL");
    coreLogger->set_level(spdlog::level::trace);

    clientLogger = spdlog::stdout_color_mt("APP");
    clientLogger->set_level(spdlog::level::trace);

    coreLogger->info("Initialized Logger.");
}

Log::~Log() {
    coreLogger->info("Shutting down Logger.");
}
}