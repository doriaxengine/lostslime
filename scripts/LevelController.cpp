#include "LevelController.h"

#include "GameState.h"
#include "PlayerController.h"

#include "Engine.h"
#include "Input.h"
#include "Log.h"
#include "Camera.h"
#include "SceneManager.h"
#include "PhysicsSystem.h"
#include "ActionSystem.h"
#include "AudioSystem.h"
#include "Sound.h"
#include "util/FunctionSubscribe.h"

#include <algorithm>
#include <cmath>

using namespace doriax;

LevelController::LevelController(Scene* scene, Entity entity): ScriptBase(scene, entity) {
    GameState::configureTransitions();

    REGISTER_ENGINE_EVENT(onUpdate);
    REGISTER_ENGINE_EVENT(onKeyDown);
    REGISTER_ENGINE_EVENT(onGamepadButtonDown);
}

LevelController::~LevelController() {
    UNREGISTER_ENGINE_EVENT(onUpdate);
    UNREGISTER_ENGINE_EVENT(onKeyDown);
    UNREGISTER_ENGINE_EVENT(onGamepadButtonDown);
}

void LevelController::start() {
    started = true;
    GameState::beginLevel(sceneName, levelTitle, nextScene);

    Entity playerRoot = scene->findEntity("Player");
    if (playerRoot != NULL_ENTITY) {
        heroEntity = scene->findEntity("Hero", playerRoot);
    }
    backgroundEntity = scene->findEntity("Background");

    PlayerController* hero = findScript<PlayerController>(scene, heroEntity, "PlayerController");
    if (!hero) {
        Log::error("LevelController: no Player bundle with a Hero sprite in the scene");
        return;
    }

    Vector3 spawn = spawnPoint ? spawnPoint->getWorldPosition() : hero->getWorldPosition();
    hero->respawn(spawn);

    setPaused(false);
    updateCamera(0.0f, true);

    Entity music = scene->findEntity("Music");
    if (music != NULL_ENTITY) {
        musicVolume = Sound(scene, music).getVolume();
        playSound(scene, "Music");
    }
}

// until the loading screen covers the level
void LevelController::fadeOutMusic(float seconds) {
    musicFade = seconds + SceneManager::getLoadingDelay();
    musicFadeTime = musicFade;
}

void LevelController::setPaused(bool paused) {
    GameState::paused = paused;
    scene->getSystem<PhysicsSystem>()->setPaused(paused);
    scene->getSystem<ActionSystem>()->setPaused(paused);
    scene->getSystem<AudioSystem>()->setPaused(paused);

    if (paused) {
        SceneManager::addChildScene("Pause Scene");
    } else {
        SceneManager::removeChildScene("Pause Scene");
    }
}

void LevelController::onKeyDown(int key, bool repeat, int mods) {
    if (repeat || !started || transitionTimer >= 0.0f) return;
    if (key == D_KEY_ESCAPE || key == D_KEY_P) {
        setPaused(!GameState::paused);
    }
}

void LevelController::onGamepadButtonDown(int id, int button) {
    if (!started || transitionTimer >= 0.0f) return;
    if (button == D_GAMEPAD_BUTTON_START) {
        setPaused(!GameState::paused);
    }
}

void LevelController::updateCamera(float dt, bool snap) {
    if (heroEntity == NULL_ENTITY || scene->getCamera() == NULL_ENTITY) return;

    Vector3 heroPos = Object(scene, heroEntity).getWorldPosition();
    float viewWidth = (float)Engine::getCanvasWidth();
    float viewHeight = (float)Engine::getCanvasHeight();

    PlayerController* hero = findScript<PlayerController>(scene, heroEntity, "PlayerController");
    float lookTarget = (hero && !hero->isFacingRight()) ? -lookAheadDistance : lookAheadDistance;
    lookAhead = snap ? lookTarget : lookAhead + (lookTarget - lookAhead) * std::min(1.0f, 2.5f * dt);

    Vector3 target(heroPos.x + lookAhead - viewWidth * 0.5f, heroPos.y + 80.0f - viewHeight * 0.5f, 0.0f);
    target.x = std::clamp(target.x, 0.0f, std::max(0.0f, levelWidth - viewWidth));
    target.y = std::clamp(target.y, 0.0f, std::max(0.0f, levelHeight - viewHeight));

    if (snap || cameraSmoothing <= 0.0f) {
        cameraPosition = target;
    } else {
        float t = std::min(1.0f, cameraSmoothing * dt);
        cameraPosition = cameraPosition + (target - cameraPosition) * t;
    }

    Vector3 shake;
    if (GameState::shakeTime > 0.0f) {
        GameState::shakeTime = std::max(0.0f, GameState::shakeTime - dt);
        shakePhase += dt;
        float amount = GameState::shakeStrength * std::min(1.0f, GameState::shakeTime / 0.25f);
        shake = Vector3(std::sin(shakePhase * 53.0f), std::cos(shakePhase * 41.0f), 0.0f) * amount;
        if (GameState::shakeTime <= 0.0f) GameState::shakeStrength = 0.0f;
    }

    Camera camera(scene, scene->getCamera());
    camera.setPosition(cameraPosition.x + shake.x, cameraPosition.y + shake.y, 1.0f);
    camera.setTarget(cameraPosition.x + shake.x, cameraPosition.y + shake.y, 0.0f);

    if (backgroundEntity != NULL_ENTITY) {
        Object background(scene, backgroundEntity);
        Vector3 bgPos = background.getPosition();
        background.setPosition(cameraPosition.x * (1.0f - backgroundParallax), cameraPosition.y * (1.0f - backgroundParallax), bgPos.z);
    }
}

void LevelController::onUpdate() {
    if (!started) {
        // wait for the loading screen to go
        if (SceneManager::isLoading()) return;
        start();
    }

    float dt = Engine::getDeltatime();

    if (musicFade > 0.0f) {
        musicFade = std::max(0.0f, musicFade - dt);
        Entity music = scene->findEntity("Music");
        if (music != NULL_ENTITY) {
            Sound(scene, music).setVolume(musicVolume * musicFade / musicFadeTime);
        }
    }

    if (GameState::resumeRequested) {
        GameState::resumeRequested = false;
        if (GameState::paused) setPaused(false);
    }

    if (GameState::respawnRequested) {
        GameState::respawnRequested = false;
        PlayerController* hero = findScript<PlayerController>(scene, heroEntity, "PlayerController");
        if (hero) {
            hero->respawn(spawnPoint ? spawnPoint->getWorldPosition() : Vector3(160.0f, 320.0f, 0.0f));
            updateCamera(0.0f, true);
        }
    }

    if (transitionTimer >= 0.0f) {
        transitionTimer -= dt;
        if (transitionTimer < 0.0f) {
            GameState::loadScene(transitionScene);
            transitionTimer = -2.0f; // never fire twice
        }
        return;
    }

    if (GameState::levelCompleteRequested) {
        GameState::levelCompleteRequested = false;
        GameState::score += 100;
        transitionScene = nextScene.empty() ? "Win Scene" : nextScene;
        transitionTimer = 0.6f; // plus the loading fade in
        if (nextScene.empty()) GameState::recordScore();
        fadeOutMusic(transitionTimer);
    } else if (GameState::gameOverRequested) {
        GameState::gameOverRequested = false;
        transitionScene = "Game Over Scene";
        transitionTimer = 0.3f;
        GameState::recordScore();
        fadeOutMusic(transitionTimer);
    }

    if (GameState::paused) return;

    updateCamera(dt, false);
}
