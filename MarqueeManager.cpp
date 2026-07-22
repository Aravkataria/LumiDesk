#include "MarqueeManager.h"

MarqueeManager::MarqueeManager()
{
    displayWidth = 128;

    textWidth = 0;

    currentX = 0;

    scrolling = false;

    lastMove = 0;
    pauseStart = 0;

    state = PAUSE_AT_START;
}

void MarqueeManager::setDisplayWidth(int width)
{
    displayWidth = width;
}

void MarqueeManager::setText(const String& value, int width)
{
    text = value;

    textWidth = width;

    currentX = 0;

    scrolling = textWidth > displayWidth;

    pauseStart = millis();

    state = PAUSE_AT_START;
}

void MarqueeManager::update()
{
    if(!scrolling)
        return;

    unsigned long now = millis();

    switch(state)
    {
        case PAUSE_AT_START:

            if(now - pauseStart > 1000)
            {
                state = SCROLLING;
            }

            break;

        case SCROLLING:

            if(now - lastMove > 25)
            {
                lastMove = now;

                currentX--;

                if(currentX <= -(textWidth - displayWidth + 8))
                {
                    state = PAUSE_AT_END;
                    pauseStart = now;
                }
            }

            break;

        case PAUSE_AT_END:

            if(now - pauseStart > 1000)
            {
                currentX = 0;
                state = PAUSE_AT_START;
                pauseStart = now;
            }

            break;
    }
}

int MarqueeManager::offset() const
{
    return currentX;
}

const String& MarqueeManager::getText() const
{
    return text;
}

bool MarqueeManager::isScrolling() const
{
    return scrolling;
}