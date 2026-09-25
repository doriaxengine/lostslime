#pragma once

#include "Sprite.h"
#include "ScriptProperty.h"

#include <string>

class Collectible : public doriax::Sprite {
public:
    DPROPERTY("Kind")
    std::string kind = "coin";   // coin, gem or key

    DPROPERTY("Score Value")
    int scoreValue = 10;

    DPROPERTY("Bob Height")
    float bobHeight = 6.0f;

    DPROPERTY("Bob Speed")
    float bobSpeed = 3.0f;

    Collectible(doriax::Scene* scene, doriax::Entity entity);
    virtual ~Collectible();

    void onUpdate();
    void collect();

private:
    bool collected = false;
    bool started = false;
    float time = 0.0f;
    doriax::Vector3 basePosition;
};
