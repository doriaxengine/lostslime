#include "Collectible.h"

#include "GameState.h"

#include "Engine.h"
#include "Body2D.h"
#include "util/FunctionSubscribe.h"

#include <cmath>

using namespace doriax;

Collectible::Collectible(Scene* scene, Entity entity): Sprite(scene, entity) {
    REGISTER_ENGINE_EVENT(onUpdate);
}

Collectible::~Collectible() {
    UNREGISTER_ENGINE_EVENT(onUpdate);
}

void Collectible::onUpdate() {
    if (GameState::paused) return;

    if (!started) {
        // the editor reuses the entities when replaying
        started = true;
        collected = false;
        basePosition = getPosition();
        setVisible(true);
        if (ensureBody2D(getScene(), getEntity())) {
            getBody2D().setEnabled(true);
        }
    }

    if (collected) return;

    time += Engine::getDeltatime();
    float offset = std::sin(time * bobSpeed + basePosition.x * 0.01f) * bobHeight;
    setPosition(basePosition.x, basePosition.y + offset, basePosition.z);
}

void Collectible::collect() {
    if (collected) return;
    collected = true;

    if (kind == "key") {
        GameState::hasKey = true;
        playSound(getScene(), "Key Sound");
    } else if (kind == "gem") {
        GameState::score += scoreValue;
        playSound(getScene(), "Gem Sound");
    } else {
        GameState::coins += 1;
        GameState::score += scoreValue;
        playSound(getScene(), "Coin Sound");
    }

    setVisible(false);
    if (ensureBody2D(getScene(), getEntity())) {
        getBody2D().setEnabled(false);
    }
}
