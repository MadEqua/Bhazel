#pragma once

#include "Event.h"

namespace BZ {

    class BZ_API MouseMovedEvent : public Event {
    public:
        MouseMovedEvent(int x, int y)
            : mouseX(x), mouseY(y) {}

        inline int getX() const { return mouseX; }
        inline int getY() const { return mouseY; }

        std::string toString() const override {
            std::stringstream ss;
            ss << "MouseMovedEvent: " << mouseX << ", " << mouseY;
            return ss.str();
        }

        EVENT_CLASS_TYPE(MouseMoved)
        EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

    private:
        int mouseX, mouseY;
    };


    class BZ_API MouseScrolledEvent : public Event {
    public:
        MouseScrolledEvent(int xOffset, int yOffset)
            : xOffset(xOffset), yOffset(yOffset) {}

        inline int getXOffset() const { return xOffset; }
        inline int getYOffset() const { return yOffset; }

        std::string toString() const override {
            std::stringstream ss;
            ss << "MouseScrolledEvent: " << xOffset << ", " << yOffset;
            return ss.str();
        }

        EVENT_CLASS_TYPE(MouseScrolled)
        EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

    private:
        int xOffset, yOffset;
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