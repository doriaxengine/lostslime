#pragma once

#include "ScriptBase.h"
#include "ScriptProperty.h"
#include "Image.h"
#include "Text.h"

#include <string>

class HudController : public doriax::ScriptBase {
public:
    DPROPERTY("Heart 1")
    doriax::Image* heart1 = nullptr;

    DPROPERTY("Heart 2")
    doriax::Image* heart2 = nullptr;

    DPROPERTY("Heart 3")
    doriax::Image* heart3 = nullptr;

    DPROPERTY("Coin Text")
    doriax::Text* coinText = nullptr;

    DPROPERTY("Score Text")
    doriax::Text* scoreText = nullptr;

    DPROPERTY("Key Icon")
    doriax::Image* keyIcon = nullptr;

    DPROPERTY("Level Text")
    doriax::Text* levelText = nullptr;

    DPROPERTY("Full Heart Texture")
    std::string fullHeart = "ui/hud_heart.png";

    DPROPERTY("Empty Heart Texture")
    std::string emptyHeart = "ui/hud_heart_empty.png";

    HudController(doriax::Scene* scene, doriax::Entity entity);
    virtual ~HudController();

    void onUpdate();

private:
    int shownLives = -1;
    int shownCoins = -1;
    int shownScore = -1;
    int shownKey = -1;
    std::string shownLevel;
};
