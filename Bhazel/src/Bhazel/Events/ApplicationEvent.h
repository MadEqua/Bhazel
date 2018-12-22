#pragma once

#include <sstream>

#include "Event.h"

namespace BZ {

    class BZ_API WindowResizeEvent : public Event {
    public:
        WindowResizeEvent(unsigned int width, unsigned int height)
            : width(width), height(height) {}

        inline unsigned int getWidth() const { return width; }
        inline unsigned int getHeight() const { return height; }

        std::string toString() const override {
            std::stringstream ss;
            ss << "WindowResizeEvent: " << width << ", " << height;
            return ss.str();
        }

        EVENT_CLASS_TYPE(WindowResize)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

    private:
        unsigned int width, height;
    };


    class BZ_API WindowCloseEvent : public Event {
    public:
        EVENT_CLASS_TYPE(WindowClose)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class BZ_API AppTickEvent : public Event {
    public:
        EVENT_CLASS_TYPE(AppTick)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };


    class BZ_API AppUpdateEvent : public Event {
    public:
        EVENT_CLASS_TYPE(AppUpdate)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };


    class BZ_API AppRenderEvent : public Event {
    public:
        EVENT_CLASS_TYPE(AppRender)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };
}