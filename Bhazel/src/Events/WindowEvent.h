#pragma once

#include "Event.h"


namespace BZ {

class WindowResizedEvent : public Event {
  public:
    WindowResizedEvent(uint32 width, uint32 height) : width(width), height(height) {}

    inline uint32 getWidth() const { return width; }
    inline uint32 getHeight() const { return height; }

    std::string toString() const override {
        std::stringstream ss;
        ss << "WindowResizedEvent: " << width << ", " << height;
        return ss.str();
    }

    EVENT_CLASS_TYPE(WindowResized)
    EVENT_CLASS_CATEGORY(EventCategoryWindow)

  private:
    uint32 width, height;
};


class WindowClosedEvent : public Event {
  public:
    EVENT_CLASS_TYPE(WindowClosed)
    EVENT_CLASS_CATEGORY(EventCategoryWindow)
};


class WindowIconifiedEvent : public Event {
  public:
    WindowIconifiedEvent(bool iconified) : iconified(iconified) {}

    inline bool isMinimized() const { return iconified; }
    inline bool isRestored() const { return !iconified; }

    std::string toString() const override {
        std::stringstream ss;
        ss << "WindowIconifiedEvent: " << iconified;
        return ss.str();
    }

    EVENT_CLASS_TYPE(WindowIconified)
    EVENT_CLASS_CATEGORY(EventCategoryWindow)

  private:
    bool iconified;
};
}