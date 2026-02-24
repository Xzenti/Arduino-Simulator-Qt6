#ifndef PIN_H
#define PIN_H

/**
 * @class Pin
 * @brief Represents a single digital pin on an Arduino board
 *
 * This class models the behavior of an Arduino digital pin including:
 * - Pin state (HIGH/LOW)
 * - Pin mode (INPUT/OUTPUT)
 * - Pin number and capabilities
 */
class Pin {
public:
    /// Pin mode enumeration
    enum class Mode {
        INPUT = 0,      ///< Input mode
        OUTPUT = 1      ///< Output mode
    };

    /// Pin state enumeration
    enum class State {
        LOW = 0,        ///< Logic 0
        HIGH = 1        ///< Logic 1
    };

    /**
     * @brief Constructor
     * @param number The pin number (0-19 for Arduino Uno)
     */
    explicit Pin(int number);

    /**
     * @brief Destructor
     */
    ~Pin();

    /**
     * @brief Set the pin mode
     * @param mode INPUT or OUTPUT mode
     */
    void setMode(Mode mode);

    /**
     * @brief Get the current pin mode
     * @return Current pin mode
     */
    Mode getMode() const;

    /**
     * @brief Set the pin state
     * @param state HIGH or LOW state
     */
    void setState(State state);

    /**
     * @brief Get the current pin state
     * @return Current pin state
     */
    State getState() const;

    /**
     * @brief Get the pin number
     * @return Pin number
     */
    int getNumber() const;

    /**
     * @brief Check if pin is digital capable
     * @return true if this is a digital pin
     */
    bool isDigitalCapable() const;

private:
    int number_;         ///< Pin number
    Mode mode_;          ///< Current pin mode
    State state_;        ///< Current pin state
};

#endif // PIN_H
