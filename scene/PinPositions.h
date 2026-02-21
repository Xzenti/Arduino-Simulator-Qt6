#ifndef PINPOSITIONS_H
#define PINPOSITIONS_H

/**
 * Hardcoded pin and leg positions for the simulator.
 * Arduino Uno board image: 560×420 px (board-local coordinates, origin top-left).
 */

namespace PinPositions {

// Arduino Uno board (560×420)
constexpr int BOARD_WIDTH = 560;
constexpr int BOARD_HEIGHT = 420;

// Digital pins D0–D13 (right side, top row)
constexpr double D0_X = 443,  D0_Y = 83;
constexpr double D1_X = 430,  D1_Y = 83;
constexpr double D2_X = 416,  D2_Y = 83;
constexpr double D3_X = 403,  D3_Y = 83;
constexpr double D4_X = 389,  D4_Y = 83;
constexpr double D5_X = 376,  D5_Y = 83;
constexpr double D6_X = 362,  D6_Y = 83;
constexpr double D7_X = 349,  D7_Y = 83;
constexpr double D8_X = 326,  D8_Y = 83;
constexpr double D9_X = 312,  D9_Y = 83;
constexpr double D10_X = 299, D10_Y = 83;
constexpr double D11_X = 286, D11_Y = 83;
constexpr double D12_X = 272, D12_Y = 83;
constexpr double D13_X = 259, D13_Y = 83;

// Right side: GND (near D13), AREF, SDA, SCL
constexpr double GND_D13_X = 246, GND_D13_Y = 83;
constexpr double AREF_X = 232, AREF_Y = 83;
constexpr double SDA_X = 219, SDA_Y = 83;
constexpr double SCL_X = 205, SCL_Y = 83;

// Analog A0–A5 (right side, bottom row)
constexpr double A0_X = 369, A0_Y = 332;
constexpr double A1_X = 383, A1_Y = 332;
constexpr double A2_X = 396, A2_Y = 332;
constexpr double A3_X = 410, A3_Y = 332;
constexpr double A4_X = 423, A4_Y = 332;
constexpr double A5_X = 436, A5_Y = 332;

// Power / bottom row (left side)
constexpr double POWER_3V3_X = 292, POWER_3V3_Y = 332;
constexpr double POWER_5V_X  = 306, POWER_5V_Y  = 332;
constexpr double POWER_GND_X = 320, POWER_GND_Y = 332;
constexpr double GND_X = 334, GND_Y = 332;
constexpr double VIN_X = 346, VIN_Y = 332;
constexpr double RESET_X = 279, RESET_Y = 332;
constexpr double IOREF_X = 265, IOREF_Y = 332;

//  LED (48×48) anode and cathode leg positions (positive = anode, negative = cathode)
constexpr int LED_WIDTH = 48;
constexpr int LED_HEIGHT = 48;
constexpr double LED_ANODE_X = 28,  LED_ANODE_Y = 47;   // anode (positive) leg
constexpr double LED_CATHODE_X = 19, LED_CATHODE_Y = 47; // cathode (negative) leg

// Resistor (60×40) leg positions
constexpr int RESISTOR_WIDTH = 60;
constexpr int RESISTOR_HEIGHT = 40;
constexpr double RESISTOR_LEG1_X = 3,  RESISTOR_LEG1_Y = 20;
constexpr double RESISTOR_LEG2_X = 58, RESISTOR_LEG2_Y = 20;

// Button (50×50) pin positions : top-left, top-right, bottom-left, bottom-right
constexpr int BUTTON_WIDTH = 50;
constexpr int BUTTON_HEIGHT = 50;
constexpr double BUTTON_PIN1_X = 9,  BUTTON_PIN1_Y = 13;  // top-left
constexpr double BUTTON_PIN2_X = 40, BUTTON_PIN2_Y = 13;  // top-right
constexpr double BUTTON_PIN3_X = 9,  BUTTON_PIN3_Y = 35;  // bottom-left
constexpr double BUTTON_PIN4_X = 40, BUTTON_PIN4_Y = 35;  // bottom-right

} // namespace PinPositions

#include <QPointF>
#include <QString>
#include <QStringList>

namespace PinPositions {
// Board-local (x,y) for dropdown pin id, e.g. "D13" -> (259,83)
QPointF getBoardPinPosition(const QString &pinId);
// List of all pin ids for dropdown: Digital D0-D13, Analog A0-A5, Power/other
QStringList getPinIdList();
// Digital pin number 0-13 for "D0"-"D13", else -1
int pinIdToDigitalNumber(const QString &pinId);
// True for pins on the bottom row (A0-A5, 3V3, 5V, VIN, RESET, IOREF)
bool isBottomRowPin(const QString &pinId);
} // namespace PinPositions

#endif // PINPOSITIONS_H
