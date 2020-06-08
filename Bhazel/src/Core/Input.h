#pragma once


namespace BZ {

class Window;

class Input {
  public:
    static bool isKeyPressed(int keycode);
    static bool isMouseButtonPressed(int button);
    static glm::ivec2 getMousePosition();
    static int getMouseX();
    static int getMouseY();

  private:
    static void init();

    friend class Application;
};
}
