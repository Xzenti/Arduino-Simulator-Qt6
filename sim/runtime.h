#ifndef RUNTIME_H
#define RUNTIME_H

#include "Action.h"
#include "BoardModel.h"
#include <QObject>
#include <QVector>
#include <QTimer>
#include <memory>

class Runtime : public QObject {
    Q_OBJECT

public:
    Runtime();
    ~Runtime();

    void executeActions(const QVector<Action> &actions);
    void stop();

signals:
    void actionExecuted(const QString &description);
    void pinStateChanged(int pin, int value);

private slots:
    void executeNextAction();

private:
    QVector<Action> actions;
    int currentActionIndex = 0;
    QTimer *executionTimer;
    std::unique_ptr<BoardModel> boardModel;
};

#endif // RUNTIME_H
