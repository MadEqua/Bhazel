#pragma once


namespace BZ {

    class Event;

    class ImGuiRenderer {
    public:
        static void begin();
        static void end();

        static void onEvent(Event &event);

    private:
        friend class Application;

        static void init();
        static void destroy();

        static void initInput();
        static void initGraphics();
    };
}