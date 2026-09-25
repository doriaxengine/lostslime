#pragma once

#include "ScriptBase.h"
#include "ScriptProperty.h"
#include "Image.h"
#include "Text.h"

class LoadingScreen : public doriax::ScriptBase {
public:
    DPROPERTY("Backdrop")
    doriax::Image* backdrop = nullptr;

    DPROPERTY("Title Text")
    doriax::Text* titleText = nullptr;

    DPROPERTY("Status Text")
    doriax::Text* statusText = nullptr;

    DPROPERTY("Coin")
    doriax::Image* coin = nullptr;

    DPROPERTY("Bar")
    doriax::Image* bar = nullptr;

    DPROPERTY("Bar Fill")
    doriax::Image* barFill = nullptr;

    DPROPERTY("Bar Width")
    float barWidth = 360.0f;

    DPROPERTY("Coin Size")
    float coinSize = 64.0f;

    LoadingScreen(doriax::Scene* scene, doriax::Entity entity);
    virtual ~LoadingScreen();

    void onUpdate();

private:
    void setAlpha(float alpha);

    bool active = false;
    float timer = 0.0f;
    int shownDots = -1;
};
