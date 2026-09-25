#include "ExitDoor.h"

#include "GameState.h"

#include "Engine.h"
#include "util/FunctionSubscribe.h"

#include <algorithm>

using namespace doriax;

ExitDoor::ExitDoor(Scene* scene, Entity entity): Sprite(scene, entity) {
    REGISTER_ENGINE_EVENT(onUpdate);
}

ExitDoor::~ExitDoor() {
    UNREGISTER_ENGINE_EVENT(onUpdate);
}

void ExitDoor::setOpen(bool value) {
    open = value;
    setFrame(open ? "open" : "closed");

    // upper half
    Entity top = getScene()->findEntity("Door Top", getEntity());
    if (top != NULL_ENTITY) {
        Sprite topSprite(getScene(), top);
        topSprite.setFrame(open ? "open" : "closed");
    }
}

void ExitDoor::onUpdate() {
    if (GameState::paused) return;

    if (!started) {
        started = true;
        entered = false;
        setOpen(!needsKey);
    }

    lockedHintTimer = std::max(0.0f, lockedHintTimer - (float)Engine::getDeltatime());

    if (!open && (!needsKey || GameState::hasKey)) {
        setOpen(true);
        playSound(getScene(), "Door Sound");
    }
}

bool ExitDoor::tryEnter() {
    if (entered) return false;
    if (open) {
        entered = true;
        playSound(getScene(), "Door Sound");
        return true;
    }
    if (lockedHintTimer <= 0.0f) {
        lockedHintTimer = 1.0f;
        playSound(getScene(), "Locked Sound");
    }
    return false;
}
