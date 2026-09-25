#include "HudController.h"

#include "GameState.h"

#include "util/FunctionSubscribe.h"

using namespace doriax;

HudController::HudController(Scene* scene, Entity entity): ScriptBase(scene, entity) {
    REGISTER_ENGINE_EVENT(onUpdate);
}

HudController::~HudController() {
    UNREGISTER_ENGINE_EVENT(onUpdate);
}

void HudController::onUpdate() {
    if (GameState::lives != shownLives) {
        shownLives = GameState::lives;
        Image* hearts[3] = {heart1, heart2, heart3};
        for (int i = 0; i < 3; i++) {
            if (!hearts[i]) continue;
            hearts[i]->setTexture(i < shownLives ? fullHeart : emptyHeart);
        }
    }

    if (coinText && GameState::coins != shownCoins) {
        shownCoins = GameState::coins;
        coinText->setText("x " + std::to_string(shownCoins));
    }

    if (scoreText && GameState::score != shownScore) {
        shownScore = GameState::score;
        scoreText->setText(std::to_string(shownScore));
    }

    int keyState = GameState::hasKey ? 1 : 0;
    if (keyIcon && keyState != shownKey) {
        shownKey = keyState;
        keyIcon->setColor(1.0f, 1.0f, 1.0f, GameState::hasKey ? 1.0f : 0.3f);
    }

    if (levelText && GameState::levelName != shownLevel) {
        shownLevel = GameState::levelName;
        levelText->setText(shownLevel);
    }
}
