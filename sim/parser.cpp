#include "Parser.h"
#include <QRegularExpression>
#include <QStringList>

Parser::Parser() = default;

Parser::~Parser() = default;

QVector<Action> Parser::parse(const QString &sketchCode) {
    QVector<Action> actions;
    QStringList lines = sketchCode.split('\n');

    for (const QString &line : lines) {
        Action action = parseAction(line.trimmed());
        if (action.type != ActionType::Unknown) {
            actions.append(action);
        }
    }

    return actions;
}

Action Parser::parseAction(const QString &line) {
    Action action;
    action.rawLine = line;

    // Skip comments and empty lines
    if (line.isEmpty() || line.startsWith("//")) {
        return action;
    }
    QRegularExpression pinModeRx("pinMode\\s*\\(\\s*(\\d+)\\s*,\\s*(OUTPUT|INPUT_PULLUP|INPUT)\\s*\\)");
    QRegularExpressionMatch match = pinModeRx.match(line);
    if (match.hasMatch()) {
        action.type = ActionType::DigitalWrite;
        action.pin = match.captured(1).toInt();
        action.value = (match.captured(2) == "OUTPUT") ? 1 : 0;
        return action;
    }
    QRegularExpression digitalWriteRx("digitalWrite\\s*\\(\\s*(\\d+)\\s*,\\s*(HIGH|LOW)\\s*\\)");
    match = digitalWriteRx.match(line);
    if (match.hasMatch()) {
        action.type = ActionType::DigitalWrite;
        action.pin = match.captured(1).toInt();
        action.value = (match.captured(2) == "HIGH") ? 1 : 0;
        return action;
    }

    // delay(ms)
    QRegularExpression delayRx("delay\\s*\\(\\s*(\\d+)\\s*\\)");
    match = delayRx.match(line);
    if (match.hasMatch()) {
        action.type = ActionType::Delay;
        action.value = match.captured(1).toInt();
        return action;
    }

    return action;
}
