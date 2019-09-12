#include "bzpch.h"
#include "Timer.h"


namespace BZ {

    TimeDuration::TimeDuration() :
        nanos(std::chrono::nanoseconds::zero()) {
    }

    TimeDuration::TimeDuration(uint64 nanos) :
        nanos(nanos) {
    }

    TimeDuration::TimeDuration(std::chrono::nanoseconds nanos) : 
        nanos(nanos) {
    }

    TimeDuration TimeDuration::operator+(const TimeDuration &rhs) {
        nanos += rhs.nanos;
        return *this;
    }

    TimeDuration TimeDuration::operator-(const TimeDuration &rhs) {
        nanos -= rhs.nanos;
        return *this;
    }

    void TimeDuration::operator+=(const TimeDuration &rhs) {
        nanos += rhs.nanos;
    }

    void TimeDuration::operator-=(const TimeDuration &rhs) {
        nanos -= rhs.nanos;
    }

    float TimeDuration::asSeconds() const { 
        return std::chrono::duration_cast<std::chrono::duration<float>>(nanos).count();
    }

    float TimeDuration::asMillisecondsFloat() const {
        return std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(nanos).count();
    }

    uint64 TimeDuration::asMillisecondsUint() const {
        return static_cast<uint64>(std::chrono::duration_cast<std::chrono::milliseconds>(nanos).count());
    }

    uint64 TimeDuration::asNanoseconds() const {
        return static_cast<uint64>(std::chrono::duration_cast<std::chrono::nanoseconds>(nanos).count());
    }


    Timer::Timer() :
        started(false) {
    }

    void Timer::start() {
        startPoint = std::chrono::high_resolution_clock::now();
        started = true;
    }

    TimeDuration Timer::getElapsedTime() const {
        if(started)
            return TimeDuration(std::chrono::high_resolution_clock::now() - startPoint);
        else
            return TimeDuration();
    }
}