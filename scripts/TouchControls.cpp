#include "TouchControls.h"

#include "GameState.h"

#include "Input.h"
#include "System.h"
#include "SceneManager.h"
#include "util/FunctionSubscribe.h"

using namespace doriax;

// follows the last input used, starting from what the device is
static int usingTouch = -1;

TouchControls::TouchControls(Scene* scene, Entity entity): ScriptBase(scene, entity) {
    REGISTER_ENGINE_EVENT(onUpdate);
    REGISTER_ENGINE_EVENT(onKeyDown);
    REGISTER_ENGINE_EVENT(onGamepadButtonDown);
}

TouchControls::~TouchControls() {
    UNREGISTER_ENGINE_EVENT(onUpdate);
    UNREGISTER_ENGINE_EVENT(onKeyDown);
    UNREGISTER_ENGINE_EVENT(onGamepadButtonDown);
}

bool TouchControls::isTouched(Image* button) const {
    Vector3 pos = button->getWorldPosition();
    Vector3 scale = button->getWorldScale();
    float right = pos.x + button->getWidth() * scale.x;
    float bottom = pos.y + button->getHeight() * scale.y;

    for (const Touch& touch : Input::getTouches()) {
        if (touch.position.x >= pos.x - touchMargin && touch.position.x <= right + touchMargin &&
            touch.position.y >= pos.y - touchMargin && touch.position.y <= bottom + touchMargin) {
            return true;
        }
    }
    return false;
}

bool TouchControls::updateButton(Image* button) {
    if (!button || !shown) return false;
    bool touched = isTouched(button);
    button->setAlpha(touched ? 0.9f : 0.5f);
    return touched;
}

void TouchControls::setShown(bool value) {
    shown = value;
    for (Image* button : {left, right, down, jump, pause}) {
        if (button) button->setVisible(value);
    }
}

void TouchControls::onUpdate() {
    if (usingTouch < 0) {
        usingTouch = System::instance().isTouchDevice() ? 1 : 0;
    }
    if (Input::numTouches() > 0) {
        usingTouch = 1;
    }

    bool active = usingTouch && !GameState::paused && !SceneManager::isLoading();
    if (active != shown) {
        setShown(active);
    }

    GameState::touchLeft = updateButton(left);
    GameState::touchRight = updateButton(right);
    GameState::touchDown = updateButton(down);
    GameState::touchJump = updateButton(jump);

    bool pauseDown = updateButton(pause);
    if (pauseDown && !pauseTouched) {
        GameState::pauseRequested = true;
    }
    pauseTouched = pauseDown;
}

// a keyboard or gamepad takes over from the screen
void TouchControls::onKeyDown(int key, bool repeat, int mods) {
    usingTouch = 0;
}

void TouchControls::onGamepadButtonDown(int id, int button) {
    usingTouch = 0;
}
