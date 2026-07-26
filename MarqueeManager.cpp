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

    if (textWidth <= displayWidth)
    {
        currentX = (displayWidth - textWidth) / 2;
        scrolling = false;
    }
    else
    {
        scrolling = true;

        // Start just inside the left edge
        currentX = 0;
    }

    lastMove = millis();
    pauseStart = millis();

    state = PAUSE_AT_START;
}

void MarqueeManager::update()
{
    if (!scrolling)
        return;

    unsigned long now = millis();

    switch (state)
    {
        case PAUSE_AT_START:
        {
            if (now - pauseStart >= 1200)
            {
                state = SCROLLING;
                lastMove = now;
            }
            break;
        }

        case SCROLLING:
        {
            if (now - lastMove >= 20)
            {
                lastMove = now;

                currentX--;

                // Scroll completely off-screen
                if (currentX <= -textWidth - 12)
                {
                    state = PAUSE_AT_END;
                    pauseStart = now;
                }
            }
            break;
        }

        case PAUSE_AT_END:
        {
            if (now - pauseStart >= 1200)
            {
                currentX = displayWidth;

                state = SCROLLING;
                lastMove = now;
            }
            break;
        }
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