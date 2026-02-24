#ifndef BOARDMODEL_H
#define BOARDMODEL_H

#include <map>
#include <functional>


class BoardModel {
public:
    BoardModel();
    ~BoardModel();

    void setDigitalPin(int pin, int value);  // value: 0=LOW, 1=HIGH
    int getDigitalPin(int pin) const;
    void setPinMode(int pin, bool isOutput);
    bool getPinMode(int pin) const;
    void reset();

    void setInputPinProvider(std::function<int(int)> provider);
private:
    std::map<int, int> digitalPins;
    std::map<int, bool> pinModes;  // true=OUTPUT, false=INPUT
    std::function<int(int)> inputPinProvider;
};

#endif // BOARDMODEL_H
