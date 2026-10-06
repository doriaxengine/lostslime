#pragma once

#include "Sprite.h"
#include "ScriptProperty.h"

// the patrol and a bee's hover are actions of the bundle
class Enemy : public doriax::Sprite {
public:
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

    void squash();
    bool isSquashed() const { return squashed; }
    bool canBeStomped() const { return canStomp; }
    float getStompHeight() const { return stompHeight; }

private:
    void applyFacing();

    bool squashed = false;
    bool movingRight = true;
    bool hasLastX = false;
    float lastX = 0.0f;
    float squashTimer = 0.0f;
};
