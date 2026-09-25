#include "Enemy.h"

#include "GameState.h"

#include "Engine.h"
#include "Body2D.h"
#include "Action.h"
#include "util/FunctionSubscribe.h"

#include <cmath>

using namespace doriax;

Enemy::Enemy(Scene* scene, Entity entity): Sprite(scene, entity) {
    REGISTER_ENGINE_EVENT(onUpdate);
    REGISTER_ENGINE_EVENT(onPostUpdate);
    REGISTER_ENGINE_EVENT(onFixedUpdate);
}

Enemy::~Enemy() {
    UNREGISTER_ENGINE_EVENT(onUpdate);
    UNREGISTER_ENGINE_EVENT(onPostUpdate);
    UNREGISTER_ENGINE_EVENT(onFixedUpdate);
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

void Enemy::onFixedUpdate() {
    if (GameState::paused || squashed) return;
    if (!ensureBody2D(getScene(), getEntity())) return;

    Body2D body = getBody2D();

    if (!started) {
        started = true;
        // patrol around the bundle root
        Vector3 rootPos = getWorldPosition() - getPosition();
        origin = rootPos;
        body.setPosition(Vector2(origin.x, origin.y));
    }

    float dt = Engine::getUpdateTime();
    time += dt;

    Vector3 pos = getWorldPosition();
    if (movingRight && pos.x > origin.x + patrolDistance) movingRight = false;
    if (!movingRight && pos.x < origin.x - patrolDistance) movingRight = true;

    Vector2 velocity(movingRight ? speed : -speed, 0.0f);
    if (flying) {
        // kinematic body, so hover through the velocity
        float targetY = origin.y + std::sin(time * hoverSpeed) * hoverHeight;
        velocity.y = (targetY - pos.y) * 8.0f;
    }
    body.setLinearVelocity(velocity);
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

// after the animation has switched frames
void Enemy::onPostUpdate() {
    if (GameState::paused || squashed) return;
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

    // flat frame, when the sheet has one
    Entity anim = getScene()->findEntity("Walk Animation", getEntity());
    if (anim != NULL_ENTITY) {
        Action(getScene(), anim).stop();
    }
    SpriteComponent& sprite = getComponent<SpriteComponent>();
    for (unsigned int i = 0; i < sprite.numFramesRect; i++) {
        if (sprite.framesRect[i].name == "flat") {
            setFrame("flat");
            applyFacing();
            break;
        }
    }
}
