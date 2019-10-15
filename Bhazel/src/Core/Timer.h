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
}