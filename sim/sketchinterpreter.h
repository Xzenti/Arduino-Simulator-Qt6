#ifndef SKETCHINTERPRETER_H
#define SKETCHINTERPRETER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QTimer>
#include <functional>
#include <memory>

class BoardModel;

class SketchInterpreter : public QObject {
    Q_OBJECT

public:
    explicit SketchInterpreter(BoardModel *boardModel,
                               std::function<void(int, int)> onDigitalWrite,
                               QObject *parent = nullptr);
    ~SketchInterpreter();

    void setCode(const QString &code);
    void start();
    void stop();
    void pause();
    void resume();
    bool isRunning() const { return running_; }
    bool isPaused() const { return paused_; }

signals:
    void finished();

private:
    bool extractSetupAndLoop();
    void runSetup();
    void runLoopFrom(int lineIndex);
    void executeLine(const QString &line, const QStringList &lines, int &lineIndex, bool inLoop, bool inNestedBlock = false);
    qint64 evalExpr(const QString &expr);
    bool evalCondition(const QString &cond);
    int findMatchingBrace(const QStringList &lines, int startIndex) const;

    BoardModel *boardModel_ = nullptr;
    std::function<void(int, int)> onDigitalWrite_;
    QString code_;
    QStringList setupLines_;
    QStringList loopLines_;
    QMap<QString, qint64> variables_;
    qint64 startTimeMs_ = 0;
    bool running_ = false;
    bool paused_ = false;
    int pausedLineIndex_ = -1;       // line to resume from after unpause
    qint64 pausedDelayRemaining_ = 0; // remaining delay ms when paused mid-delay
    QTimer *delayTimer_ = nullptr;
    int savedLoopLineIndex_ = -1;
};

#endif // SKETCHINTERPRETER_H
