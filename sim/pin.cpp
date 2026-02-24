#include "Pin.h"

Pin::Pin(int number)
    : number_(number), mode_(Mode::INPUT), state_(State::LOW) {
}

Pin::~Pin() {
}

void Pin::setMode(Mode mode) {
    mode_ = mode;
}

Pin::Mode Pin::getMode() const {
    return mode_;
}

void Pin::setState(State state) {
    state_ = state;
}

Pin::State Pin::getState() const {
    return state_;
}

int Pin::getNumber() const {
    return number_;
}

bool Pin::isDigitalCapable() const {
    // Arduino Uno has 14 digital pins (0-13) plus 6 analog pins (14-19)
    return number_ >= 0 && number_ < 20;
}
