#include "PlayerController.h"

#include "GameState.h"
#include "Collectible.h"
#include "ExitDoor.h"
#include "Enemy.h"

#include "Engine.h"
#include "Input.h"
#include "Object.h"
#include "Ray.h"
#include "Particles.h"
#include "SceneManager.h"
#include "PhysicsSystem.h"
#include "util/FunctionSubscribe.h"

#include <cmath>
#include <algorithm>

using namespace doriax;

PlayerController::PlayerController(Scene* scene, Entity entity): Sprite(scene, entity) {
    physics = scene->getSystem<PhysicsSystem>().get();

    REGISTER_ENGINE_EVENT(onUpdate);
    REGISTER_ENGINE_EVENT(onFixedUpdate);
    REGISTER_ENGINE_EVENT(onKeyDown);
    REGISTER_ENGINE_EVENT(onKeyUp);
    REGISTER_ENGINE_EVENT(onGamepadButtonDown);
    REGISTER_ENGINE_EVENT(onGamepadButtonUp);
    if (physics) {
        REGISTER_EVENT(physics->beginSensorContact2D, onBeginSensorContact);
        REGISTER_EVENT(physics->preSolve2D, onPreSolve);
    }
}

PlayerController::~PlayerController() {
    UNREGISTER_ENGINE_EVENT(onUpdate);
    UNREGISTER_ENGINE_EVENT(onFixedUpdate);
    UNREGISTER_ENGINE_EVENT(onKeyDown);
    UNREGISTER_ENGINE_EVENT(onKeyUp);
    UNREGISTER_ENGINE_EVENT(onGamepadButtonDown);
    UNREGISTER_ENGINE_EVENT(onGamepadButtonUp);
    if (physics) {
        UNREGISTER_EVENT(physics->beginSensorContact2D, onBeginSensorContact);
        UNREGISTER_EVENT(physics->preSolve2D, onPreSolve);
    }
}

void PlayerController::respawn(Vector3 worldPosition) {
    if (!ensureBody2D(getScene(), getEntity())) return;

    dead = false;
    deathTimer = 0.0f;
    knockbackTimer = 0.0f;
    jumpBufferTimer = 0.0f;
    dropping = false;
    lastVelocityY = 0.0f;
    fallSpeed = 0.0f;
    facingRight = true;
    setVisible(true);
    setAlpha(1.0f);

    Body2D body = getBody2D();
    body.setEnabled(true);
    body.setPosition(Vector2(worldPosition.x, worldPosition.y));
    body.setLinearVelocity(Vector2::ZERO);
}

void PlayerController::requestJump() {
    if (GameState::paused || dead || SceneManager::isLoading()) return;
    jumpBufferTimer = jumpBufferTime;
    jumpHeld = true;
}

void PlayerController::onKeyDown(int key, bool repeat, int mods) {
    if (repeat) return;
    if (key == D_KEY_SPACE || key == D_KEY_UP || key == D_KEY_W) {
        requestJump();
    }
}

void PlayerController::onKeyUp(int key, bool repeat, int mods) {
    if (key == D_KEY_SPACE || key == D_KEY_UP || key == D_KEY_W) {
        jumpHeld = false;
    }
}

void PlayerController::onGamepadButtonDown(int id, int button) {
    if (button == D_GAMEPAD_BUTTON_A || button == D_GAMEPAD_BUTTON_B) {
        requestJump();
    }
}

void PlayerController::onGamepadButtonUp(int id, int button) {
    if (button == D_GAMEPAD_BUTTON_A || button == D_GAMEPAD_BUTTON_B) {
        jumpHeld = false;
    }
}

uint16_t PlayerController::checkGround() {
    Vector3 feet = getWorldPosition();
    float halfWidth = colliderWidth * 0.5f - 4.0f;
    Vector3 down(0.0f, -10.0f, 0.0f);

    // platforms only hold a hero that isn't rising
    uint16_t mask = Layer::TERRAIN;
    if (lastVelocityY < 1.0f && !dropping) mask |= Layer::PLATFORM;

    for (float offset : {-halfWidth, 0.0f, halfWidth}) {
        Ray ray(Vector3(feet.x + offset, feet.y + 4.0f, 0.0f), down);
        RayReturn hit = ray.intersects(getScene(), RayFilter::BODY_2D, false, Layer::PLAYER, mask, getEntity());
        if (hit) {
            groundTopY = hit.point.y;
            return Body2D(getScene(), hit.body).getCategoryBitsFilter(hit.shapeIndex);
        }
    }
    return 0;
}

float PlayerController::platformTop(Body2D& platform, unsigned long shape) {
    Body2DComponent* body = getScene()->findComponent<Body2DComponent>(platform.getEntity());
    if (!body || shape >= body->numShapes) return 0.0f;

    const Shape2D& box = body->shapes[shape];
    float top = std::max(box.pointA.y, box.pointB.y);
    for (size_t i = 0; i < box.numVertices; i++) top = std::max(top, box.vertices[i].y);
    return top + Object(getScene(), platform.getEntity()).getWorldPosition().y;
}

bool PlayerController::onPreSolve(Body2D bodyA, unsigned long shapeA, Body2D bodyB, unsigned long shapeB, Manifold2D) {
    bool heroIsA = bodyA.getEntity() == getEntity();
    if (!heroIsA && bodyB.getEntity() != getEntity()) return true;

    Body2D& platform = heroIsA ? bodyB : bodyA;
    unsigned long shape = heroIsA ? shapeB : shapeA;
    if (!(platform.getCategoryBitsFilter(shape) & Layer::PLATFORM)) return true;

    float top = platformTop(platform, shape);
    if (dropping && std::fabs(top - dropTopY) < 1.0f) return false;

    // solid only if the feet were above it before the step
    return stepFeetY + 8.0f >= top;
}

void PlayerController::emitDust() {
    if (dustEmitter == NULL_ENTITY) {
        dustEmitter = getScene()->findEntity("Dust Emitter", getEntity());
    }
    if (dustEmitter == NULL_ENTITY) return;

    Particles dust(getScene(), dustEmitter);
    dust.reset();
    dust.start();
}

void PlayerController::onFixedUpdate() {
    if (GameState::paused) return;
    if (!ensureBody2D(getScene(), getEntity())) return;

    Body2D body = getBody2D();
    if (!body.isFixedRotation()) {
        body.setFixedRotation(true);
    }

    float dt = Engine::getUpdateTime();
    Vector2 velocity = body.getLinearVelocity();

    if (dead) {
        body.setLinearVelocity(Vector2(0.0f, velocity.y));
        return;
    }

    // head hit something
    if (lastVelocityY > 150.0f && velocity.y <= 20.0f && velocity.y < lastVelocityY - 120.0f) {
        playSound(getScene(), "Bump Sound");
    }

    float direction = 0.0f;
    if (Input::isKeyPressed(D_KEY_LEFT) || Input::isKeyPressed(D_KEY_A)) direction -= 1.0f;
    if (Input::isKeyPressed(D_KEY_RIGHT) || Input::isKeyPressed(D_KEY_D)) direction += 1.0f;
    downHeld = Input::isKeyPressed(D_KEY_DOWN) || Input::isKeyPressed(D_KEY_S);

    if (Input::numGamepads() > 0) {
        int gamepadId = Input::getGamepadId(0);
        if (Input::isGamepadConnected(gamepadId)) {
            float axis = Input::getGamepadAxis(gamepadId, D_GAMEPAD_AXIS_LEFT_X);
            if (std::fabs(axis) > 0.25f) direction = axis;
            if (Input::isGamepadButtonPressed(gamepadId, D_GAMEPAD_BUTTON_DPAD_LEFT)) direction = -1.0f;
            if (Input::isGamepadButtonPressed(gamepadId, D_GAMEPAD_BUTTON_DPAD_RIGHT)) direction = 1.0f;
            if (Input::isGamepadButtonPressed(gamepadId, D_GAMEPAD_BUTTON_DPAD_DOWN)) downHeld = true;
            if (Input::getGamepadAxis(gamepadId, D_GAMEPAD_AXIS_LEFT_Y) > 0.5f) downHeld = true;
        }
    }

    // no steering while switching scenes
    if (SceneManager::isLoading()) direction = 0.0f;

    groundCategory = checkGround();
    grounded = groundCategory != 0;
    if (dropping && (getWorldPosition().y < dropTopY - 70.0f || (grounded && std::fabs(groundTopY - dropTopY) > 1.0f))) {
        dropping = false;
    }
    // landing dust, once the fall has stopped
    fallSpeed = std::min(fallSpeed, velocity.y);
    if (grounded && velocity.y > -50.0f) {
        if (fallSpeed < -300.0f) emitDust();
        fallSpeed = 0.0f;
    }
    if (grounded) {
        coyoteTimer = coyoteTime;
    } else {
        coyoteTimer = std::max(0.0f, coyoteTimer - dt);
    }
    jumpBufferTimer = std::max(0.0f, jumpBufferTimer - dt);
    knockbackTimer = std::max(0.0f, knockbackTimer - dt);

    if (knockbackTimer <= 0.0f) {
        velocity.x = direction * moveSpeed;
        if (direction > 0.05f) facingRight = true;
        if (direction < -0.05f) facingRight = false;
    }

    if (jumpBufferTimer > 0.0f && coyoteTimer > 0.0f) {
        jumpBufferTimer = 0.0f;
        coyoteTimer = 0.0f;
        if (groundCategory == Layer::PLATFORM && downHeld) {
            // drop through; the push also wakes a sleeping body
            dropping = true;
            dropTopY = groundTopY;
            velocity.y = -60.0f;
        } else {
            velocity.y = jumpSpeed;
            jumpCutApplied = false;
            if (grounded) emitDust();
            playSound(getScene(), "Jump Sound");
        }
        grounded = false;
    }

    // releasing the button early shortens the jump
    if (!jumpHeld && !jumpCutApplied && velocity.y > 0.0f) {
        velocity.y *= 0.45f;
        jumpCutApplied = true;
    }
    if (velocity.y <= 0.0f) {
        jumpCutApplied = true;
    }

    if (velocity.y < -maxFallSpeed) velocity.y = -maxFallSpeed;

    body.setLinearVelocity(velocity);
    lastVelocityY = velocity.y;
    stepFeetY = getWorldPosition().y;

    if (getWorldPosition().y < fallDeathY) {
        fallOut();
    }
}

// costs a heart and respawns the hero
void PlayerController::fallOut() {
    if (dead) return;
    GameState::lives = std::max(0, GameState::lives - 1);
    if (GameState::lives <= 0) {
        die();
        return;
    }
    playSound(getScene(), "Hurt Sound");
    invulnerableTimer = hurtInvulnerability;
    GameState::respawnRequested = true;
}

void PlayerController::applyFrame(const std::string& name) {
    setFrame(name);
    Rect rect = getTextureRect();
    if (!facingRight && rect.getWidth() > 0.0f) {
        // mirror by sampling the frame right to left
        setTextureRect(Rect(rect.getX() + rect.getWidth(), rect.getY(), -rect.getWidth(), rect.getHeight()));
    }
    currentFrame = name;
}

void PlayerController::onUpdate() {
    if (GameState::paused) return;

    float dt = Engine::getDeltatime();

    if (dead) {
        deathTimer += dt;
        applyFrame("hit");
        if (deathTimer > 1.2f) {
            GameState::gameOverRequested = true;
        }
        return;
    }

    if (invulnerableTimer > 0.0f) {
        invulnerableTimer = std::max(0.0f, invulnerableTimer - dt);
        bool blinkOn = std::fmod(invulnerableTimer, 0.2f) < 0.1f;
        setAlpha(blinkOn ? 1.0f : 0.35f);
    } else {
        setAlpha(1.0f);
    }

    if (!ensureBody2D(getScene(), getEntity())) return;
    Body2D body = getBody2D();
    Vector2 velocity = body.getLinearVelocity();

    std::string frame = "idle";
    if (knockbackTimer > 0.0f) {
        frame = "hit";
    } else if (!grounded) {
        frame = "jump";
    } else if (downHeld && std::fabs(velocity.x) <= 20.0f) {
        frame = "duck";
    } else if (std::fabs(velocity.x) > 20.0f) {
        animTimer += dt;
        if (animTimer >= walkFrameTime) {
            animTimer -= walkFrameTime;
            walkFrame = (walkFrame + 1) % 2;
        }
        frame = walkFrame == 0 ? "walk_a" : "walk_b";
    } else {
        animTimer = 0.0f;
        walkFrame = 0;
    }

    applyFrame(frame);
}

void PlayerController::hurt(float knockDirection) {
    if (dead || invulnerableTimer > 0.0f) return;
    if (!ensureBody2D(getScene(), getEntity())) return;

    GameState::lives = std::max(0, GameState::lives - 1);
    invulnerableTimer = hurtInvulnerability;
    knockbackTimer = 0.25f;

    Body2D body = getBody2D();
    body.setLinearVelocity(Vector2(knockDirection * 260.0f, 420.0f));
    playSound(getScene(), "Hurt Sound");
    GameState::shake(0.25f, 8.0f);

    if (GameState::lives <= 0) {
        die();
    }
}

void PlayerController::die() {
    if (dead || !ensureBody2D(getScene(), getEntity())) return;
    dead = true;
    deathTimer = 0.0f;
    setAlpha(1.0f);
    playSound(getScene(), "Hurt Sound");

    // no more contacts once dead, but keep falling
    Body2D body = getBody2D();
    body.setLinearVelocity(Vector2(0.0f, 380.0f));
    body.setShapeSensorEvents(false);
}

void PlayerController::touchPickup(Entity entity) {
    Collectible* item = findScript<Collectible>(getScene(), entity, "Collectible");
    if (item) {
        item->collect();
    }
}

void PlayerController::touchDoor(Entity entity) {
    ExitDoor* door = findScript<ExitDoor>(getScene(), entity, "ExitDoor");
    if (!door) return;
    if (door->tryEnter()) {
        GameState::levelCompleteRequested = true;
    }
}

void PlayerController::touchEnemy(Entity entity) {
    Enemy* enemy = findScript<Enemy>(getScene(), entity, "Enemy");
    if (!enemy || enemy->isSquashed()) return;

    Vector3 feet = getWorldPosition();
    Vector3 enemyPos = Object(getScene(), entity).getWorldPosition();
    Vector2 velocity = getBody2D().getLinearVelocity();

    bool stomp = enemy->canBeStomped() && velocity.y < 0.0f && feet.y > enemyPos.y + enemy->getStompHeight();
    if (stomp) {
        enemy->squash();
        Body2D body = getBody2D();
        body.setLinearVelocity(Vector2(velocity.x, stompBounce));
        jumpCutApplied = true;
        GameState::score += enemy->scoreValue;
        playSound(getScene(), "Squash Sound");
    } else {
        hurt(feet.x < enemyPos.x ? -1.0f : 1.0f);
    }
}

void PlayerController::onBeginSensorContact(Body2D sensorBody, unsigned long sensorShape, Body2D visitorBody, unsigned long visitorShape) {
    if (dead || GameState::paused) return;
    if (visitorBody.getEntity() != getEntity()) return;

    Entity other = sensorBody.getEntity();
    uint16_t category = sensorBody.getCategoryBitsFilter(sensorShape);

    if (category & Layer::PICKUP) {
        touchPickup(other);
    } else if (category & Layer::DOOR) {
        touchDoor(other);
    } else if (category & Layer::ENEMY) {
        touchEnemy(other);
    } else if (category & Layer::HAZARD) {
        // spike rows are one body, so push away from the touched shape
        Vector3 hazardPos = Object(getScene(), other).getWorldPosition();
        Body2DComponent* hazard = getScene()->findComponent<Body2DComponent>(other);
        if (hazard && sensorShape < hazard->numShapes) {
            const Shape2D& shape = hazard->shapes[sensorShape];
            Vector2 center = shape.pointA;
            if (shape.numVertices > 0) {
                center = Vector2::ZERO;
                for (size_t i = 0; i < shape.numVertices; i++) center += shape.vertices[i];
                center = center / (float)shape.numVertices;
            }
            hazardPos.x += center.x;
        }
        hurt(getWorldPosition().x < hazardPos.x ? -1.0f : 1.0f);
    }
}
