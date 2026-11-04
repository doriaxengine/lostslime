#include "MenuController.h"

#include "GameState.h"
#include "MenuButton.h"

#include "Engine.h"
#include "Input.h"
#include "util/FunctionSubscribe.h"

using namespace doriax;

MenuController::MenuController(Scene* scene, Entity entity): ScriptBase(scene, entity) {
    REGISTER_ENGINE_EVENT(onUpdate);
    REGISTER_ENGINE_EVENT(onKeyDown);
    REGISTER_ENGINE_EVENT(onGamepadButtonDown);
}

MenuController::~MenuController() {
    UNREGISTER_ENGINE_EVENT(onUpdate);
    UNREGISTER_ENGINE_EVENT(onKeyDown);
    UNREGISTER_ENGINE_EVENT(onGamepadButtonDown);
}

void MenuController::onUpdate() {
    // overlay scenes stay alive while hidden, so they arm again when shown
    if (!Engine::isSceneRunning(scene)) {
        armed = false;
        armTimer = 0.0f;
        return;
    }

    // ignore the key press that brought the player here
    if (!armed) {
        armTimer += Engine::getDeltatime();
        if (armTimer > 0.3f) {
            armed = true;
            focusPrimaryButton();
        }
    }

    if (scoreText) {
        scoreText->setText(scorePrefix + std::to_string(GameState::score) + "   Best: " + std::to_string(GameState::getBestScore()));
    }
}

// keys and gamepads start on the button of the primary action
void MenuController::focusPrimaryButton() {
    auto scripts = scene->getComponentArray<ScriptComponent>();
    for (size_t i = 0; i < scripts->size(); i++) {
        for (auto& entry : scripts->getComponentFromIndex(i).scripts) {
            if (entry.type == ScriptType::LUA || entry.className != "MenuButton" || !entry.instance) continue;

            MenuButton* button = static_cast<MenuButton*>(entry.instance);
            if (button->action == primaryAction) {
                button->setFocus();
                return;
            }
        }
    }
}

void MenuController::onKeyDown(int key, bool repeat, int mods) {
    if (repeat || !armed) return;
    if ((key == D_KEY_ENTER || key == D_KEY_SPACE) && !primaryAction.empty()) {
        MenuButton::runAction(scene, primaryAction, primaryScene);
    } else if (key == D_KEY_ESCAPE && !escapeAction.empty()) {
        MenuButton::runAction(scene, escapeAction, escapeScene);
    }
}

void MenuController::onGamepadButtonDown(int id, int button) {
    if (!armed) return;
    if ((button == D_GAMEPAD_BUTTON_A || button == D_GAMEPAD_BUTTON_START) && !primaryAction.empty()) {
        MenuButton::runAction(scene, primaryAction, primaryScene);
    } else if (button == D_GAMEPAD_BUTTON_B && !escapeAction.empty()) {
        MenuButton::runAction(scene, escapeAction, escapeScene);
    }
}
