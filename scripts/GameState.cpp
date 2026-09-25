#include "GameState.h"

#include "Engine.h"
#include "System.h"
#include "SceneManager.h"
#include "Sound.h"
#include "SoundComponent.h"
#include "Body2DComponent.h"
#include "PhysicsSystem.h"

#include <algorithm>

using namespace doriax;

int GameState::score = 0;
int GameState::lives = 3;
int GameState::maxLives = 3;
int GameState::coins = 0;
bool GameState::hasKey = false;
bool GameState::paused = false;

std::string GameState::levelName;
std::string GameState::nextScene;
std::string GameState::lastLevelScene;

bool GameState::levelCompleteRequested = false;
bool GameState::gameOverRequested = false;
bool GameState::resumeRequested = false;
bool GameState::respawnRequested = false;
bool GameState::pauseRequested = false;

bool GameState::touchLeft = false;
bool GameState::touchRight = false;
bool GameState::touchDown = false;
bool GameState::touchJump = false;

std::string GameState::loadingTitle;

static int bestScore = -1;  // loaded on first use
float GameState::shakeTime = 0.0f;
float GameState::shakeStrength = 0.0f;

void GameState::newGame(){
    score = 0;
    lives = maxLives;
    coins = 0;
    hasKey = false;
    paused = false;
    levelCompleteRequested = false;
    gameOverRequested = false;
    resumeRequested = false;
    respawnRequested = false;
    pauseRequested = false;
}

void GameState::configureTransitions(){
    Engine::setAsyncLoading(true);
    SceneManager::setLoadingScene("Loading Scene");
    // lets the menu click finish before the old scene goes
    SceneManager::setLoadingDelay(0.4f);
}

void GameState::loadScene(const std::string& sceneName){
    if (SceneManager::isLoading()) return;

    loadingTitle = (sceneName.rfind("Level ", 0) == 0) ? sceneName : "";
    SceneManager::loadScene(sceneName);
}

void GameState::shake(float seconds, float strength){
    shakeTime = std::max(shakeTime, seconds);
    shakeStrength = std::max(shakeStrength, strength);
}

int GameState::getBestScore(){
    if (bestScore < 0) {
        bestScore = System::instance().getIntegerForKey("bestScore", 0);
    }
    return bestScore;
}

void GameState::recordScore(){
    if (score <= getBestScore()) return;
    bestScore = score;
    System::instance().setIntegerForKey("bestScore", bestScore);
}

void GameState::beginLevel(const std::string& sceneName, const std::string& displayName, const std::string& next){
    shakeTime = 0.0f;
    shakeStrength = 0.0f;
    lastLevelScene = sceneName;
    levelName = displayName;
    nextScene = next;
    hasKey = false;
    paused = false;
    levelCompleteRequested = false;
    gameOverRequested = false;
    resumeRequested = false;
    respawnRequested = false;
    pauseRequested = false;
}

void playSound(Scene* scene, const std::string& name){
    if (!scene) return;
    Entity entity = scene->findEntity(name);
    if (entity == NULL_ENTITY || !scene->findComponent<SoundComponent>(entity)) return;
    Sound sound(scene, entity);
    sound.stop();
    sound.play();
}

bool ensureBody2D(Scene* scene, Entity entity){
    if (!scene || entity == NULL_ENTITY || !scene->isEntityCreated(entity)) return false;
    Body2DComponent* body = scene->findComponent<Body2DComponent>(entity);
    if (!body) return false;
    if (!b2Body_IsValid(body->body)){
        scene->getSystem<PhysicsSystem>()->loadBody2D(entity);
    }
    return b2Body_IsValid(body->body);
}
