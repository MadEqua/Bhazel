#pragma once


namespace BZ {

class Scene;
struct FrameTiming;
class Event;

// TODO: give this more flexibility and options
class SceneManager {
  public:
    ~SceneManager();

    void addScene(Scene *scene) { scenes.push_back(scene); }

    void setCurrentScene(uint32 index);
    Scene &getCurrentScene();

    void onAttachToEngine();
    void onUpdate(const FrameTiming &frameTiming);
    void onImGuiRender(const FrameTiming &frameTiming);
    void onEvent(Event &ev);

  private:
    std::vector<Scene *> scenes;
    uint32 currentSceneIdx = 0;
};
}