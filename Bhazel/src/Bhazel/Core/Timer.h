#pragma once

#include <chrono>


namespace BZ {

    class TimeDuration {
    public:
        TimeDuration();
        TimeDuration(uint64 nanos);
        TimeDuration(std::chrono::nanoseconds nanos);

        TimeDuration operator+(const TimeDuration &rhs);
        TimeDuration operator-(const TimeDuration &rhs);
        void operator+=(const TimeDuration &rhs);
        void operator-=(const TimeDuration &rhs);

        float asSeconds() const;
        float asMillisecondsFloat() const;
        uint32 asMillisecondsUint32() const;
        uint64 asMillisecondsUint64() const;
        uint64 asNanoseconds() const;

    private:
        std::chrono::nanoseconds nanos;
    };


    class Timer {
    public:
        Timer();

        void start();
        TimeDuration getElapsedTime() const;

    private:
        bool started;
        std::chrono::time_point<std::chrono::high_resolution_clock> startPoint;
    };
}