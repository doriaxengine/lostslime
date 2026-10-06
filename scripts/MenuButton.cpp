#include "MenuButton.h"

#include "GameState.h"

#include "System.h"
#include "Log.h"
#include "SceneManager.h"
#include "util/FunctionSubscribe.h"

using namespace doriax;

MenuButton::MenuButton(Scene* scene, Entity entity): Button(scene, entity) {
    REGISTER_BUTTON_EVENT(onPress, onPress);
}

MenuButton::~MenuButton() {
    if (scene && scene->isEntityCreated(entity)) {
        UNREGISTER_BUTTON_EVENT(onPress, onPress);
    }
}

void MenuButton::onPress() {
    runAction(scene, action, targetScene);
}

void MenuButton::runAction(Scene* scene, const std::string& action, const std::string& targetScene) {
    // keys still reach this scene under the loading screen
    if (SceneManager::isLoading()) return;

    playSound(scene, "Select Sound");

    if (action == "play") {
        GameState::newGame();
        GameState::loadScene(targetScene);
    } else if (action == "retry") {
        GameState::lives = GameState::maxLives;
        GameState::hasKey = false;
        std::string level = GameState::lastLevelScene.empty() ? targetScene : GameState::lastLevelScene;
        GameState::loadScene(level);
    } else if (action == "menu") {
        GameState::paused = false;
        GameState::loadScene(targetScene);
    } else if (action == "resume") {
        GameState::resumeRequested = true;
    } else if (action == "quit") {
        System::instance().quit();
    } else {
        Log::warn("MenuButton: unknown action '%s'", action.c_str());
    }
}
