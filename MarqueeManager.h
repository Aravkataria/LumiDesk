#ifndef MARQUEE_MANAGER_H
#define MARQUEE_MANAGER_H

#include <Arduino.h>

class MarqueeManager
{
private:

    String text;

    int textWidth;
    int displayWidth;

    int currentX;

    bool scrolling;

    unsigned long lastMove;
    unsigned long pauseStart;

    enum State
    {
        PAUSE_AT_START,
        SCROLLING,
        PAUSE_AT_END
    };

    State state;

public:

    MarqueeManager();

    void setDisplayWidth(int width);

    void setText(const String& value, int width);

    void update();

    int offset() const;

    const String& getText() const;

    bool isScrolling() const;
};

#endif