#pragma once

#include "Event.h"

namespace BZ {

    class BZ_API KeyEvent : public Event {
    public:
        inline int getKeyCode() const { return keyCode; }

        EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

    protected:
        KeyEvent(int keyCode) : keyCode(keyCode) {}
        int keyCode;
    };


    class BZ_API KeyPressedEvent : public KeyEvent {
    public:
        KeyPressedEvent(int keyCode, int repeatCount) : 
            KeyEvent(keyCode), repeatCount(repeatCount) {}
        
        inline int getRepeatCount() const { return repeatCount; }

        std::string toString() const override {
            std::stringstream ss;
            ss << "KeyPressedEvent: " << keyCode << " (" << repeatCount << " repeats)";
            return ss.str();
        }

        EVENT_CLASS_TYPE(KeyPressed);

    private:
        int repeatCount;
    };


    class BZ_API KeyReleasedEvent : public KeyEvent {
    public:
        KeyReleasedEvent(int keyCode) : 
            KeyEvent(keyCode) {}

        std::string toString() const override {
            std::stringstream ss;
            ss << "KeyReleasedEvent: " << keyCode;
            return ss.str();
        }

        EVENT_CLASS_TYPE(KeyReleased);
    };
}