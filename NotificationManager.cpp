#include "NotificationManager.h"
#include "DisplayManager.h"

NotificationManager::NotificationManager()
{
    visible = false;
    leaving = false;

    showStart = 0;

    position = -28;
    target = -28;
}

void NotificationManager::show(const String& t,
                               const String& m)
{
    title = t;
    message = m;

    visible = true;
    leaving = false;

    showStart = 0;

    position = -28;
    target = 0;
}

void NotificationManager::update()
{
    if (!visible)
        return;

    position += (target - position) * speed;

    if (!leaving &&
        abs(target - position) < 0.5f)
    {
        if (showStart == 0)
            showStart = millis();

        if (millis() - showStart > 2000)
        {
            leaving = true;
            target = -28;
        }
    }

    if (leaving &&
        abs(target - position) < 0.5f)
    {
        visible = false;
    }
}

void NotificationManager::draw(DisplayManager& display)
{
    if (!visible)
        return;

    display.drawNotification(
        title,
        message,
        (int)position);
}

bool NotificationManager::isVisible()
{
    return visible;
}