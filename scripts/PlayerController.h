#pragma once

#include "Sprite.h"
#include "Body2D.h"
#include "Manifold2D.h"
#include "ScriptProperty.h"

#include <string>

namespace doriax { class PhysicsSystem; }

class PlayerController : public doriax::Sprite {
public:
    DPROPERTY("Move Speed")
    float moveSpeed = 380.0f;

    DPROPERTY("Jump Speed")
    float jumpSpeed = 900.0f;

    DPROPERTY("Max Fall Speed")
    float maxFallSpeed = 1400.0f;

    DPROPERTY("Coyote Time")
    float coyoteTime = 0.12f;

    DPROPERTY("Jump Buffer")
    float jumpBufferTime = 0.15f;

    DPROPERTY("Hurt Invulnerability")
    float hurtInvulnerability = 1.5f;

    DPROPERTY("Stomp Bounce")
    float stompBounce = 520.0f;

    DPROPERTY("Walk Frame Time")
    float walkFrameTime = 0.14f;

    DPROPERTY("Collider Width")
    float colliderWidth = 56.0f;

    DPROPERTY("Fall Death Y")
    float fallDeathY = -200.0f;

    PlayerController(doriax::Scene* scene, doriax::Entity entity);
    virtual ~PlayerController();

    void onUpdate();
    void onFixedUpdate();
    void onKeyDown(int key, bool repeat, int mods);
    void onKeyUp(int key, bool repeat, int mods);
    void onGamepadButtonDown(int id, int button);
    void onGamepadButtonUp(int id, int button);
    void onBeginSensorContact(doriax::Body2D sensorBody, unsigned long sensorShape, doriax::Body2D visitorBody, unsigned long visitorShape);
    // one-way platforms
    bool onPreSolve(doriax::Body2D bodyA, unsigned long shapeA, doriax::Body2D bodyB, unsigned long shapeB, doriax::Manifold2D manifold);

    bool isFacingRight() const { return facingRight; }

    void respawn(doriax::Vector3 worldPosition);

private:
    void applyFrame(const std::string& name);
    void requestJump();
    void hurt(float knockDirection);
    void fallOut();
    void die();
    // category under the feet, 0 in the air
    uint16_t checkGround();
    float platformTop(doriax::Body2D& platform, unsigned long shape);
    void emitDust();
    void touchPickup(doriax::Entity entity);
    void touchDoor(doriax::Entity entity);
    void touchEnemy(doriax::Entity entity);

    doriax::PhysicsSystem* physics = nullptr;

    bool facingRight = true;
    bool grounded = false;
    bool downHeld = false;
    uint16_t groundCategory = 0;
    float groundTopY = 0.0f;
    float lastVelocityY = 0.0f;   // before the physics step
    float stepFeetY = 0.0f;       // before the physics step
    float fallSpeed = 0.0f;
    bool dropping = false;
    float dropTopY = 0.0f;
    doriax::Entity dustEmitter = NULL_ENTITY;
    bool dead = false;
    bool jumpHeld = false;
    bool touchJump = false;
    bool jumpCutApplied = true;
    float coyoteTimer = 0.0f;
    float jumpBufferTimer = 0.0f;
    float invulnerableTimer = 0.0f;
    float knockbackTimer = 0.0f;
    float animTimer = 0.0f;
    int walkFrame = 0;
    float deathTimer = 0.0f;
    std::string currentFrame;
};
