#include "AnimationManager.h"

Animation::Animation(float start)
{
    currentValue = start;
    targetValue = start;
    speed = 0.18f;
}

void Animation::set(float value)
{
    currentValue = value;
    targetValue = value;
}

void Animation::moveTo(float value)
{
    targetValue = value;
}

void Animation::update()
{
    currentValue += (targetValue - currentValue) * speed;
}

bool Animation::isFinished()
{
    return abs(targetValue - currentValue) < 0.5;
}

float Animation::value()
{
    return currentValue;
}