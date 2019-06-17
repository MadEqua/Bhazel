#pragma once

#include "Event.h"

namespace BZ {

    class BZ_API WindowResizeEvent : public Event {
    public:
        WindowResizeEvent(uint32 width, uint32 height)
            : width(width), height(height) {}

        inline uint32 getWidth() const { return width; }
        inline uint32 getHeight() const { return height; }

        std::string toString() const override {
            std::stringstream ss;
            ss << "WindowResizeEvent: " << width << ", " << height;
            return ss.str();
        }

        EVENT_CLASS_TYPE(WindowResize)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

    private:
        uint32 width, height;
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