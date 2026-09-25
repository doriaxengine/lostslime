#include "LoadingScreen.h"

#include "GameState.h"

#include "Engine.h"
#include "SceneManager.h"
#include "util/FunctionSubscribe.h"

#include <algorithm>
#include <cmath>

using namespace doriax;

LoadingScreen::LoadingScreen(Scene* scene, Entity entity): ScriptBase(scene, entity) {
    REGISTER_ENGINE_EVENT(onUpdate);
}

LoadingScreen::~LoadingScreen() {
    UNREGISTER_ENGINE_EVENT(onUpdate);
}

void LoadingScreen::setAlpha(float alpha) {
    if (backdrop) backdrop->setAlpha(alpha);
    if (titleText) titleText->setAlpha(alpha);
    if (statusText) statusText->setAlpha(alpha);
    if (coin) coin->setAlpha(alpha);
    if (bar) bar->setAlpha(alpha * 0.15f);
    if (barFill) barFill->setAlpha(alpha);
}

void LoadingScreen::onUpdate() {
    // also called while off screen
    if (!SceneManager::isLoading()) {
        active = false;
        return;
    }

    if (!active) {
        active = true;
        timer = 0.0f;
        shownDots = -1;
        if (titleText) titleText->setText(GameState::loadingTitle);
    }

    timer += (float)Engine::getDeltatime();

    float delay = SceneManager::getLoadingDelay();
    setAlpha(delay > 0.0f ? std::min(1.0f, timer / delay) : 1.0f);

    int dots = (int)(timer * 3.0f) % 4;
    if (statusText && dots != shownDots) {
        shownDots = dots;
        statusText->setText("LOADING" + std::string(dots, '.'));
    }

    if (coin) {
        float width = coinSize * std::fabs(std::cos(timer * 4.0f));
        coin->setWidth((unsigned int)std::max(1.0f, width));
    }

    if (barFill) {
        barFill->setWidth((unsigned int)std::max(1.0f, barWidth * SceneManager::getLoadingProgress()));
    }
}
