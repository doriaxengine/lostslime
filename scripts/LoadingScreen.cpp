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
    REGISTER_ENGINE_EVENT(onSceneLoaded);
}

LoadingScreen::~LoadingScreen() {
    UNREGISTER_ENGINE_EVENT(onUpdate);
    UNREGISTER_ENGINE_EVENT(onSceneLoaded);
    if (holding) SceneManager::releaseLoading();
}

void LoadingScreen::setAlpha(float alpha) {
    if (backdrop) backdrop->setAlpha(alpha);
    if (titleText) titleText->setAlpha(alpha);
    if (statusText) statusText->setAlpha(alpha);
    if (coin) coin->setAlpha(alpha);
    if (bar) bar->setAlpha(alpha * 0.15f);
    if (barFill) barFill->setAlpha(alpha);
}

// stays up to fade out
void LoadingScreen::onSceneLoaded() {
    if (holding || !Engine::isSceneRunning(scene)) return;

    holding = SceneManager::holdLoading();
    fadeTimer = 0.0f;
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
        barProgress = 0.0f;
        shownDots = -1;
        if (titleText) titleText->setText(GameState::loadingTitle);
    }

    float dt = (float)Engine::getDeltatime();
    // the first frame after the switch carries its freeze
    float step = std::min(dt, 1.0f / 30.0f);
    timer += dt;

    if (holding) {
        fadeTimer += step;
        float alpha = (fadeOut > 0.0f) ? 1.0f - fadeTimer / fadeOut : 0.0f;
        setAlpha(std::max(0.0f, alpha));
        if (alpha <= 0.0f) {
            holding = false;
            SceneManager::releaseLoading();
        }
    } else {
        float delay = SceneManager::getLoadingDelay();
        setAlpha(delay > 0.0f ? std::min(1.0f, timer / delay) : 1.0f);
    }

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
        barProgress += (SceneManager::getLoadingProgress() - barProgress) * std::min(1.0f, step * 12.0f);
        barFill->setWidth((unsigned int)std::max(1.0f, barWidth * barProgress));
    }
}
