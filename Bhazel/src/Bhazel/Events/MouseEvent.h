#pragma once

#include "Event.h"

namespace BZ {

    class BZ_API MouseMovedEvent : public Event {
    public:
        MouseMovedEvent(float x, float y)
            : mouseX(x), mouseY(y) {}

        inline float getX() const { return mouseX; }
        inline float getY() const { return mouseY; }

        std::string toString() const override {
            std::stringstream ss;
            ss << "MouseMovedEvent: " << mouseX << ", " << mouseY;
            return ss.str();
        }

        EVENT_CLASS_TYPE(MouseMoved)
        EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

    private:
        float mouseX, mouseY;
    };


    class BZ_API MouseScrolledEvent : public Event {
    public:
        MouseScrolledEvent(float xOffset, float yOffset)
            : xOffset(xOffset), yOffset(yOffset) {}

        inline float getXOffset() const { return xOffset; }
        inline float getYOffset() const { return yOffset; }

        std::string toString() const override {
            std::stringstream ss;
            ss << "MouseScrolledEvent: " << xOffset << ", " << yOffset;
            return ss.str();
        }

        EVENT_CLASS_TYPE(MouseScrolled)
        EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

    private:
        float xOffset, yOffset;
    };


    class BZ_API MouseButtonEvent : public Event {
    public:
        inline int getMouseButton() const { return button; }

        EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

    protected:
        MouseButtonEvent(int button)
            : button(button) {}

        int button;
    };


    class BZ_API MouseButtonPressedEvent : public MouseButtonEvent {
    public:
        MouseButtonPressedEvent(int button)
            : MouseButtonEvent(button) {}

        std::string toString() const override {
            std::stringstream ss;
            ss << "MouseButtonPressedEvent: " << button;
            return ss.str();
        }

        EVENT_CLASS_TYPE(MouseButtonPressed)
    };


    class BZ_API MouseButtonReleasedEvent : public MouseButtonEvent {
    public:
        MouseButtonReleasedEvent(int button)
            : MouseButtonEvent(button) {}

        std::string toString() const override {
            std::stringstream ss;
            ss << "MouseButtonReleasedEvent: " << button;
            return ss.str();
        }

        EVENT_CLASS_TYPE(MouseButtonReleased)
    };
}