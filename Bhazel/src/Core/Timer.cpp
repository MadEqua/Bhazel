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

    uint32 TimeDuration::asMillisecondsUint32() const {
        return static_cast<uint32>(std::chrono::duration_cast<std::chrono::milliseconds>(nanos).count());
    }

    uint64 TimeDuration::asMillisecondsUint64() const {
        return static_cast<uint64>(std::chrono::duration_cast<std::chrono::milliseconds>(nanos).count());
    }

    float TimeDuration::asMicrosecondsFloat() const {
        return std::chrono::duration_cast<std::chrono::duration<float, std::micro>>(nanos).count();
    }

    uint64 TimeDuration::asMicroseconds() const {
        return static_cast<uint64>(std::chrono::duration_cast<std::chrono::microseconds>(nanos).count());
    }

    uint64 TimeDuration::asNanoseconds() const {
        return static_cast<uint64>(nanos.count());
    }


    TimeDuration operator+(const TimeDuration &lhs, const TimeDuration &rhs) {
        return TimeDuration(lhs.nanos + rhs.nanos);
    }

    TimeDuration operator-(const TimeDuration &lhs, const TimeDuration &rhs) {
        return TimeDuration(lhs.nanos - rhs.nanos);
    }

    bool operator<(const TimeDuration &lhs, const TimeDuration &rhs) {
        return lhs.nanos < rhs.nanos;
    }

    bool operator<=(const TimeDuration &lhs, const TimeDuration &rhs) {
        return lhs.nanos <= rhs.nanos;
    }

    bool operator>(const TimeDuration &lhs, const TimeDuration &rhs) {
        return lhs.nanos > rhs.nanos;
    }

    bool operator>=(const TimeDuration &lhs, const TimeDuration &rhs) {
        return lhs.nanos >= rhs.nanos;
    }


    /*-------------------------------------------------------------------------------------------*/
    Timer::Timer() {
        reset();
    }

    void Timer::start() {
        if(state == State::Started) return;

        lastStartPoint = std::chrono::high_resolution_clock::now();
        state = State::Started;
    }

    void Timer::pause() {
        if(state == State::Paused) return;

        state = State::Paused;
        countedTime += TimeDuration(std::chrono::high_resolution_clock::now() - lastStartPoint);
    }

    void Timer::reset() {
        state = State::Paused;
        countedTime = TimeDuration();
    }

    void Timer::restart() {
        reset();
        start();
    }

    TimeDuration Timer::getCountedTime() const {
        if(state == State::Started)
            return countedTime + TimeDuration(std::chrono::high_resolution_clock::now() - lastStartPoint);
        else
            return countedTime;
    }


    /*-------------------------------------------------------------------------------------------*/
    ScopedTimer::ScopedTimer(Fn&& callback) :
        callback(std::move(callback)) {
        timer.start();
    }

    ScopedTimer::~ScopedTimer() {
        callback(timer.getCountedTime());
    }
}