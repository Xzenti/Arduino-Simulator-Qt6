#include "BoardModel.h"

BoardModel::BoardModel() = default;

BoardModel::~BoardModel() = default;

void BoardModel::setDigitalPin(int pin, int value) {
    if (pin < 0 || pin > 19) return;
    digitalPins[pin] = (value != 0) ? 1 : 0;
}

int BoardModel::getDigitalPin(int pin) const {
    auto it = pinModes.find(pin);
    bool isOutput = (it != pinModes.end()) && it->second;
    if (isOutput) {
        auto pit = digitalPins.find(pin);
        return (pit != digitalPins.end()) ? pit->second : 0;
    }
    if (inputPinProvider)
        return inputPinProvider(pin);
    return 0;
}

void BoardModel::setInputPinProvider(std::function<int(int)> provider) {
    inputPinProvider = std::move(provider);
}

void BoardModel::setPinMode(int pin, bool isOutput) {
    pinModes[pin] = isOutput;
}

bool BoardModel::getPinMode(int pin) const {
    auto it = pinModes.find(pin);
    return (it != pinModes.end()) ? it->second : false;
}

void BoardModel::reset() {
    digitalPins.clear();
    pinModes.clear();
}
