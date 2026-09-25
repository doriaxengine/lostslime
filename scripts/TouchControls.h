#pragma once

#include "ScriptBase.h"
#include "ScriptProperty.h"
#include "Image.h"

class TouchControls : public doriax::ScriptBase {
public:
    DPROPERTY("Left")
    doriax::Image* left = nullptr;

    DPROPERTY("Right")
    doriax::Image* right = nullptr;

    DPROPERTY("Down")
    doriax::Image* down = nullptr;

    DPROPERTY("Jump")
    doriax::Image* jump = nullptr;

    DPROPERTY("Pause")
    doriax::Image* pause = nullptr;

    DPROPERTY("Touch Margin")
    float touchMargin = 12.0f;   // extra area around each button

    TouchControls(doriax::Scene* scene, doriax::Entity entity);
    virtual ~TouchControls();

    void onUpdate();
    void onKeyDown(int key, bool repeat, int mods);
    void onGamepadButtonDown(int id, int button);

private:
    bool isTouched(doriax::Image* button) const;
    bool updateButton(doriax::Image* button);
    void setShown(bool value);

    bool shown = true;   // visible in the scene, hidden on the first update if not used
    bool pauseTouched = false;
};
