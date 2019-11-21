#include "bzpch.h"

#include "Profiler.h"
#include <chrono>


#ifdef BZ_PROFILER

namespace BZ {

    Profiler::Profiler() {
        BZ_LOG_INFO("Initialized Profiler.");
    }

    Profiler::~Profiler() {
        BZ_LOG_INFO("Shutting down Profiler.");
    }

    void Profiler::beginSession(const char *name, const char *filePath) {
        outputStream.open(filePath);
        writeHeader();
        currentSession = Session{name};
        profileCount = 0;
    }

    void Profiler::endSession() {
        writeFooter();
        outputStream.close();
        profileCount = 0;
    }

    void Profiler::writeProfile(const char* name, uint32 threadId, uint64 startMicroSecs, uint64 endMicroSecs) {
        if (profileCount++ > 0)
            outputStream << ",";

        std::string nameStr(name);
        std::replace(nameStr.begin(), nameStr.end(), '"', '\'');

        outputStream << "{\"cat\":\"function\",\"dur\":" << (endMicroSecs - startMicroSecs) << 
            ",\"name\":\"" << name <<
            "\",\"ph\":\"X\",\"pid\":0,\"tid\":" << threadId << ",\"ts\":" << startMicroSecs <<
            "}";

        //outputStream.flush();
    }


    void Profiler::writeHeader() {
        outputStream << "{\"otherData\": {},\"traceEvents\":[";
        //outputStream.flush();
    }

    void Profiler::writeFooter() {
        outputStream << "]}";
        //outputStream.flush();
    }


    ProfilerTimer::ProfilerTimer(const char *name) :
        name(name), 
        start(std::chrono::high_resolution_clock::now()) {
    }

    ProfilerTimer::~ProfilerTimer() {
        uint64 endMicroSecs = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
        uint64 startMicroSecs = std::chrono::time_point_cast<std::chrono::microseconds>(start).time_since_epoch().count();

        uint32 threadId = std::hash<std::thread::id>{}(std::this_thread::get_id());
        Profiler::get().writeProfile(name, threadId, startMicroSecs, endMicroSecs);
    }
}
#endif