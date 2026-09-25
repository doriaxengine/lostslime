#pragma once

#include "Sprite.h"
#include "ScriptProperty.h"

class ExitDoor : public doriax::Sprite {
public:
    DPROPERTY("Needs Key")
    bool needsKey = true;

    ExitDoor(doriax::Scene* scene, doriax::Entity entity);
    virtual ~ExitDoor();

    void onUpdate();

    bool tryEnter();

private:
    void setOpen(bool value);

    bool open = false;
    bool entered = false;
    bool started = false;
    float lockedHintTimer = 0.0f;
};
