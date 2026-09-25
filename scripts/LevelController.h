#pragma once

#include "ScriptBase.h"
#include "ScriptProperty.h"
#include "Object.h"

#include <string>

class PlayerController;

class LevelController : public doriax::ScriptBase {
public:
    DPROPERTY("Scene Name")
    std::string sceneName = "Level One";

    DPROPERTY("Level Title")
    std::string levelTitle = "Green Hills";

    DPROPERTY("Next Scene")
    std::string nextScene = "Level Two";

    DPROPERTY("Level Width")
    float levelWidth = 3840.0f;

    DPROPERTY("Level Height")
    float levelHeight = 896.0f;

    DPROPERTY("Camera Smoothing")
    float cameraSmoothing = 8.0f;

    DPROPERTY("Background Parallax")
    float backgroundParallax = 0.3f;

    DPROPERTY("Look Ahead")
    float lookAheadDistance = 110.0f;

    DPROPERTY("Spawn Point")
    doriax::Object* spawnPoint = nullptr;

    LevelController(doriax::Scene* scene, doriax::Entity entity);
    virtual ~LevelController();

    void onUpdate();
    void onKeyDown(int key, bool repeat, int mods);
    void onGamepadButtonDown(int id, int button);

    void setPaused(bool paused);

private:
    void start();
    void updateCamera(float dt, bool snap);

    void fadeOutMusic(float seconds);

    bool started = false;
    float transitionTimer = -1.0f;
    float lookAhead = 0.0f;
    float shakePhase = 0.0f;
    float musicVolume = 0.0f;
    float musicFade = 0.0f;
    float musicFadeTime = 0.0f;
    std::string transitionScene;
    doriax::Entity heroEntity = NULL_ENTITY;
    doriax::Entity backgroundEntity = NULL_ENTITY;
    doriax::Vector3 cameraPosition;
};
