#include "Collectible.h"

#include "GameState.h"

#include "Body2D.h"
#include "util/FunctionSubscribe.h"

using namespace doriax;

Collectible::Collectible(Scene* scene, Entity entity): Sprite(scene, entity) {
    REGISTER_ENGINE_EVENT(onUpdate);
}

Collectible::~Collectible() {
    UNREGISTER_ENGINE_EVENT(onUpdate);
}

// the bob is the Bob action of the bundle
void Collectible::onUpdate() {
    if (GameState::paused || started) return;

    // the editor reuses the entities when replaying
    started = true;
    collected = false;
    setVisible(true);
    if (ensureBody2D(getScene(), getEntity())) {
        getBody2D().setEnabled(true);
    }
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
