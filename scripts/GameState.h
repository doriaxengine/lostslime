#pragma once

#include <string>
#include <cstdint>

#include "Scene.h"
#include "Entity.h"
#include "ScriptComponent.h"

// collision categories
namespace Layer {
    constexpr uint16_t TERRAIN  = 0x0001;
    constexpr uint16_t PLAYER   = 0x0002;
    constexpr uint16_t HAZARD   = 0x0004;
    constexpr uint16_t ENEMY    = 0x0008;
    constexpr uint16_t PICKUP   = 0x0010;
    constexpr uint16_t DOOR     = 0x0020;
    constexpr uint16_t PLATFORM = 0x0040;  // one-way
}

struct GameState {
    static int score;
    static int lives;
    static int maxLives;
    static int coins;
    static bool hasKey;
    static bool paused;

    static std::string levelName;
    static std::string nextScene;
    static std::string lastLevelScene; // for retry

    // handled by LevelController
    static bool levelCompleteRequested;
    static bool gameOverRequested;
    static bool resumeRequested;
    static bool respawnRequested;
    static bool pauseRequested;

    // on-screen buttons held, set by TouchControls
    static bool touchLeft;
    static bool touchRight;
    static bool touchDown;
    static bool touchJump;

    static std::string loadingTitle;

    static float shakeTime;
    static float shakeStrength;

    static void newGame();
    static void beginLevel(const std::string& sceneName, const std::string& displayName, const std::string& next);

    static void configureTransitions();
    static void loadScene(const std::string& sceneName);

    static void shake(float seconds, float strength);
    static int getBestScore();
    static void recordScore();
};

// C++ script instance of an entity, or nullptr
template<typename T>
T* findScript(doriax::Scene* scene, doriax::Entity entity, const char* className){
    if (!scene || entity == NULL_ENTITY || !scene->isEntityCreated(entity)) return nullptr;
    doriax::ScriptComponent* scripts = scene->findComponent<doriax::ScriptComponent>(entity);
    if (!scripts) return nullptr;
    for (auto& entry : scripts->scripts){
        if (entry.type != doriax::ScriptType::LUA && entry.className == className && entry.instance){
            return static_cast<T*>(entry.instance);
        }
    }
    return nullptr;
}

// plays one of the Sounds bundle entities
void playSound(doriax::Scene* scene, const std::string& name);

// loads the Box2D body before the first physics step, false if there is none
bool ensureBody2D(doriax::Scene* scene, doriax::Entity entity);
