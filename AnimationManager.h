#ifndef ANIMATION_MANAGER_H
#define ANIMATION_MANAGER_H

#include <Arduino.h>

class Animation
{
private:

    float currentValue;
    float targetValue;
    float speed;

public:

    Animation(float start = 0.0f);

    void set(float value);

    void moveTo(float value);

    void update();

    bool isFinished();

    float value();
};

#endif