#include "bzpch.h"

#include "SceneManager.h"

#include "Scene/Scene.h"


namespace BZ {

SceneManager::~SceneManager() {
    for (Scene *scene : scenes) {
        delete scene;
    }
}

void SceneManager::setCurrentScene(uint32 index) {
    BZ_ASSERT_CORE(index < scenes.size(), "Invalid index!");
    currentSceneIdx = index;
}

Scene &SceneManager::getCurrentScene() {
    BZ_ASSERT_CORE(currentSceneIdx < scenes.size(), "currentSceneIdx is invalid!");
    return *scenes[currentSceneIdx];
}

void SceneManager::onAttachToEngine() {
    scenes[currentSceneIdx]->onAttachToEngine();
}

void SceneManager::onUpdate(const FrameTiming &frameTiming) {
    scenes[currentSceneIdx]->onUpdate(frameTiming);
}

void SceneManager::onImGuiRender(const FrameTiming &frameTiming) {
    scenes[currentSceneIdx]->onImGuiRender(frameTiming);
}

void SceneManager::onEvent(Event &e) {
    scenes[currentSceneIdx]->onEvent(e);
}
}