#include "PinPositions.h"

QPointF PinPositions::getBoardPinPosition(const QString &pinId) {
    if (pinId == "D0")  return QPointF(D0_X, D0_Y);
    if (pinId == "D1")  return QPointF(D1_X, D1_Y);
    if (pinId == "D2")  return QPointF(D2_X, D2_Y);
    if (pinId == "D3")  return QPointF(D3_X, D3_Y);
    if (pinId == "D4")  return QPointF(D4_X, D4_Y);
    if (pinId == "D5")  return QPointF(D5_X, D5_Y);
    if (pinId == "D6")  return QPointF(D6_X, D6_Y);
    if (pinId == "D7")  return QPointF(D7_X, D7_Y);
    if (pinId == "D8")  return QPointF(D8_X, D8_Y);
    if (pinId == "D9")  return QPointF(D9_X, D9_Y);
    if (pinId == "D10") return QPointF(D10_X, D10_Y);
    if (pinId == "D11") return QPointF(D11_X, D11_Y);
    if (pinId == "D12") return QPointF(D12_X, D12_Y);
    if (pinId == "D13") return QPointF(D13_X, D13_Y);
    if (pinId == "GND1") return QPointF(GND_D13_X, GND_D13_Y);
    if (pinId == "AREF") return QPointF(AREF_X, AREF_Y);
    if (pinId == "SDA")  return QPointF(SDA_X, SDA_Y);
    if (pinId == "SCL")  return QPointF(SCL_X, SCL_Y);
    if (pinId == "A0")  return QPointF(A0_X, A0_Y);
    if (pinId == "A1")  return QPointF(A1_X, A1_Y);
    if (pinId == "A2")  return QPointF(A2_X, A2_Y);
    if (pinId == "A3")  return QPointF(A3_X, A3_Y);
    if (pinId == "A4")  return QPointF(A4_X, A4_Y);
    if (pinId == "A5")  return QPointF(A5_X, A5_Y);
    if (pinId == "3V3") return QPointF(POWER_3V3_X, POWER_3V3_Y);
    if (pinId == "5V")  return QPointF(POWER_5V_X, POWER_5V_Y);
    if (pinId == "GND") return QPointF(GND_D13_X, GND_D13_Y);  // GND near D13 (digital side)
    if (pinId == "GND2") return QPointF(GND_X, GND_Y);
    if (pinId == "VIN") return QPointF(VIN_X, VIN_Y);
    if (pinId == "RESET") return QPointF(RESET_X, RESET_Y);
    if (pinId == "IOREF") return QPointF(IOREF_X, IOREF_Y);
    return QPointF(0, 0);
}

QStringList PinPositions::getPinIdList() {
    return QStringList{
        "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7",
        "D8", "D9", "D10", "D11", "D12", "D13",
        "A0", "A1", "A2", "A3", "A4", "A5",
        "GND", "3V3", "5V", "VIN", "RESET", "IOREF", "AREF", "SDA", "SCL"
    };
}

int PinPositions::pinIdToDigitalNumber(const QString &pinId) {
    if (pinId == "D0")  return 0;
    if (pinId == "D1")  return 1;
    if (pinId == "D2")  return 2;
    if (pinId == "D3")  return 3;
    if (pinId == "D4")  return 4;
    if (pinId == "D5")  return 5;
    if (pinId == "D6")  return 6;
    if (pinId == "D7")  return 7;
    if (pinId == "D8")  return 8;
    if (pinId == "D9")  return 9;
    if (pinId == "D10") return 10;
    if (pinId == "D11") return 11;
    if (pinId == "D12") return 12;
    if (pinId == "D13") return 13;
    return -1;
}

bool PinPositions::isBottomRowPin(const QString &pinId) {
    return pinId == "A0" || pinId == "A1" || pinId == "A2" || pinId == "A3" || pinId == "A4" || pinId == "A5"
           || pinId == "3V3" || pinId == "5V" || pinId == "VIN" || pinId == "RESET" || pinId == "IOREF";
}
