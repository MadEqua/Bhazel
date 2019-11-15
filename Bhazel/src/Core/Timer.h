#pragma once

#include <chrono>


namespace BZ {

    class TimeDuration {
    public:
        TimeDuration();
        TimeDuration(uint64 nanos);
        TimeDuration(std::chrono::nanoseconds nanos);

        void operator+=(const TimeDuration &rhs);
        void operator-=(const TimeDuration &rhs);

        float asSeconds() const;
        float asMillisecondsFloat() const;
        uint32 asMillisecondsUint32() const;
        uint64 asMillisecondsUint64() const;
        uint64 asNanoseconds() const;

    private:
        std::chrono::nanoseconds nanos;

        friend TimeDuration operator+(const TimeDuration &lhs, const TimeDuration &rhs);
        friend TimeDuration operator-(const TimeDuration &lhs, const TimeDuration &rhs);
        friend bool operator<(const TimeDuration &lhs, const TimeDuration &rhs);
        friend bool operator<=(const TimeDuration &lhs, const TimeDuration &rhs);
        friend bool operator>(const TimeDuration &lhs, const TimeDuration &rhs);
        friend bool operator>=(const TimeDuration &lhs, const TimeDuration &rhs);
    };

    TimeDuration operator+(const TimeDuration &lhs, const TimeDuration &rhs);
    TimeDuration operator-(const TimeDuration &lhs, const TimeDuration &rhs);
    bool operator<(const TimeDuration &lhs, const TimeDuration &rhs);
    bool operator<=(const TimeDuration &lhs, const TimeDuration &rhs);
    bool operator>(const TimeDuration &lhs, const TimeDuration &rhs);
    bool operator>=(const TimeDuration &lhs, const TimeDuration &rhs);


    class Timer {
    public:
        Timer();

        void start();
        void pause();
        void reset();

        TimeDuration getCountedTime() const;

    private:
        enum class State {
            Started, Paused
        };
        State state;
        TimeDuration countedTime;

        std::chrono::time_point<std::chrono::high_resolution_clock> lastStartPoint;
    };


    class ScopedTimer {
    public:
        struct Result {
            const char* name;
            TimeDuration timeDuration;
        };
        using Fn = std::function<void(const Result&)>;

        ScopedTimer(const char *name, Fn &&func);
        ~ScopedTimer();

        const char* getName() const { return name; }

    private:
        const char *name;
        Timer timer;
        Fn func;
    };

#define PROFILE_SCOPE(name, fn) ScopedTimer timer##__LINE__(name, fn)
}