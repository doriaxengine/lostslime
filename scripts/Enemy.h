#pragma once

#include "Sprite.h"
#include "ScriptProperty.h"

class Enemy : public doriax::Sprite {
public:
    DPROPERTY("Speed")
    float speed = 90.0f;

    DPROPERTY("Patrol Distance")
    float patrolDistance = 160.0f;   // each side of the start

    DPROPERTY("Flying")
    bool flying = false;

    DPROPERTY("Hover Height")
    float hoverHeight = 24.0f;

    DPROPERTY("Hover Speed")
    float hoverSpeed = 4.0f;

    DPROPERTY("Can Be Stomped")
    bool canStomp = true;

    DPROPERTY("Stomp Height")
    float stompHeight = 30.0f;       // above the enemy origin

    DPROPERTY("Score Value")
    int scoreValue = 50;

    DPROPERTY("Faces Right")
    bool spriteFacesRight = false;

    Enemy(doriax::Scene* scene, doriax::Entity entity);
    virtual ~Enemy();

    void onUpdate();
    void onPostUpdate();
    void onFixedUpdate();

    void squash();
    bool isSquashed() const { return squashed; }
    bool canBeStomped() const { return canStomp; }
    float getStompHeight() const { return stompHeight; }

private:
    void applyFacing();

    bool started = false;
    bool squashed = false;
    bool movingRight = true;
    bool lastFacingRight = true;
    float time = 0.0f;
    float squashTimer = 0.0f;
    doriax::Vector3 origin;
};
