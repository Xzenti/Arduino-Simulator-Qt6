#ifndef ACTION_H
#define ACTION_H

#include <QString>

enum class ActionType {
    DigitalWrite,
    Delay,
    PinMode,
    DigitalRead,
    Unknown
};

struct Action {
    ActionType type = ActionType::Unknown;
    int pin = -1;
    int value = -1;
    QString rawLine;
};

#endif
