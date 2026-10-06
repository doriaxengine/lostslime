#include "Enemy.h"

#include "GameState.h"

#include "Engine.h"
#include "Body2D.h"
#include "Action.h"
#include "util/FunctionSubscribe.h"

using namespace doriax;

Enemy::Enemy(Scene* scene, Entity entity): Sprite(scene, entity) {
    REGISTER_ENGINE_EVENT(onUpdate);
    REGISTER_ENGINE_EVENT(onPostUpdate);
}

Enemy::~Enemy() {
    UNREGISTER_ENGINE_EVENT(onUpdate);
    UNREGISTER_ENGINE_EVENT(onPostUpdate);
}

void Enemy::applyFacing() {
    bool facingRight = movingRight;
    Rect rect = getTextureRect();
    if (rect.getWidth() == 0.0f) return;

    bool mirrored = rect.getWidth() < 0.0f;
    bool wantMirrored = facingRight != spriteFacesRight;
    if (mirrored != wantMirrored) {
        setTextureRect(Rect(rect.getX() + rect.getWidth(), rect.getY(), -rect.getWidth(), rect.getHeight()));
    }
}

void Enemy::onUpdate() {
    if (GameState::paused) return;

    if (squashed) {
        squashTimer += Engine::getDeltatime();
        if (squashTimer > 0.6f) {
            setVisible(false);
        }
    }
}

// after the actions have moved it and switched frames
void Enemy::onPostUpdate() {
    if (GameState::paused) return;

    if (squashed) {
        // flat frame, when the sheet has one
        SpriteComponent& sprite = getComponent<SpriteComponent>();
        for (unsigned int i = 0; i < sprite.numFramesRect; i++) {
            if (sprite.framesRect[i].name == "flat") {
                setFrame("flat");
                applyFacing();
                break;
            }
        }
        return;
    }

    // faces where the patrol takes it
    float x = getWorldPosition().x;
    if (hasLastX && x != lastX) movingRight = x > lastX;
    hasLastX = true;
    lastX = x;

    applyFacing();
}

void Enemy::squash() {
    if (squashed) return;
    squashed = true;
    squashTimer = 0.0f;

    if (ensureBody2D(getScene(), getEntity())) {
        Body2D body = getBody2D();
        body.setLinearVelocity(Vector2::ZERO);
        body.setEnabled(false);
    }

    // stays where it was squashed: its walk, patrol and hover stop
    Entity parent = getComponent<Transform>().parent;
    auto actions = getScene()->getComponentArray<ActionComponent>();
    for (size_t i = 0; i < actions->size(); i++) {
        Entity target = actions->getComponentFromIndex(i).target;
        if (target == getEntity() || target == parent) {
            Action(getScene(), actions->getEntity(i)).stop();
        }
    }
}
