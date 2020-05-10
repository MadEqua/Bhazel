#pragma once

#include <fstream>

#include "Core/Singleton.h"


#ifdef BZ_PROFILER

namespace BZ {

    class Profiler {
        BZ_GENERATE_SINGLETON(Profiler)

    public:
        void beginSession(const char* name, const char* filePath = "BhazelProfiler.json");
        void endSession();

        void writeProfile(const char* name, uint32 threadId, uint64 startMicroSecs, uint64 endMicroSecs);

    private:
        struct Session {
            const char* name;
        };

        Session currentSession;
        std::ofstream outputStream;
        uint32 profileCount;

        void writeHeader();
        void writeFooter();
    };


    /*-------------------------------------------------------------------------------------------*/
    class ProfilerTimer {
    public:
        ProfilerTimer(const char* name);
        ~ProfilerTimer();

    private:
        const char* name;
        std::chrono::time_point<std::chrono::high_resolution_clock> start;
    };
}

    #define BZ_PROFILE_BEGIN_SESSION(name, filePath) BZ::Profiler::get().beginSession(name, filePath);
    #define BZ_PROFILE_END_SESSION() BZ::Profiler::get().endSession();

    #define BZ_PROFILE_FUNCTION() BZ::ProfilerTimer timer(__FUNCSIG__);
    #define BZ_PROFILE_SCOPE(name) BZ::ProfilerTimer timer##__LINE__(name);
#else
    #define BZ_PROFILE_BEGIN_SESSION(name, filePath)
    #define BZ_PROFILE_END_SESSION()
    
    #define BZ_PROFILE_FUNCTION()
    #define BZ_PROFILE_SCOPE(name)
#endif