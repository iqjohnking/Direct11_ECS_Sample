#pragma once

#include "Component.h"

struct BoxColliderComponent : public IComponent
{
    float width = 0.0f;
    float height = 0.0f;
    bool isStatic = true;
};
