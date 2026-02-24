#ifndef PARSER_H
#define PARSER_H

#include <QString>
#include <QVector>
#include "Action.h"

class Parser {
public:
    Parser();
    ~Parser();

    QVector<Action> parse(const QString &sketchCode);

private:
    Action parseAction(const QString &line);
};

#endif // PARSER_H
