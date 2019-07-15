#pragma once

#include <chrono>


namespace BZ {

    struct Timestep {
        Timestep(float timeSeconds = 0.0f) : timeSeconds(timeSeconds) {}

        float getSeconds() const { return timeSeconds; }
        float getMilliseconds() const { return timeSeconds * 1000.0f; }

        operator float() const { return timeSeconds; }

    private:
        float timeSeconds;
    };


    class Timer {
    public:
        Timer();

        void start();

        float getElapsedSeconds() const;
        uint32 getElapsedMilliseconds() const;
        
        Timestep getAsTimestep() const;

    private:
        bool started;
        std::chrono::time_point<std::chrono::high_resolution_clock> startPoint;
    };
}