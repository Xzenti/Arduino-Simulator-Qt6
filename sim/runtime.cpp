#include "Runtime.h"
#include <QTimer>

Runtime::Runtime()
    : boardModel(std::make_unique<BoardModel>()) {
    executionTimer = new QTimer(this);
    connect(executionTimer, &QTimer::timeout, this, &Runtime::executeNextAction);
}

Runtime::~Runtime() = default;

void Runtime::executeActions(const QVector<Action> &actions) {
    this->actions = actions;
    currentActionIndex = 0;
    if (!actions.isEmpty()) {
        executeNextAction();
    }
}

void Runtime::stop() {
    executionTimer->stop();
    currentActionIndex = 0;
    emit actionExecuted("Runtime stopped");
}

void Runtime::executeNextAction() {
    if (currentActionIndex >= actions.size()) {
        // Loop back
        currentActionIndex = 0;
    }

    const Action &action = actions[currentActionIndex];

    switch (action.type) {
    case ActionType::DigitalWrite:
        boardModel->setDigitalPin(action.pin, action.value);
        emit pinStateChanged(action.pin, action.value);
        emit actionExecuted(QString("digitalWrite(%1, %2)")
                                .arg(action.pin)
                                .arg(action.value ? "HIGH" : "LOW"));
        currentActionIndex++;
        executionTimer->start(0);
        break;

    case ActionType::Delay:
        emit actionExecuted(QString("delay(%1)").arg(action.value));
        currentActionIndex++;
        executionTimer->start(action.value);
        break;

    default:
        currentActionIndex++;
        executionTimer->start(0);
        break;
    }
}
