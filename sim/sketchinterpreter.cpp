#include "SketchInterpreter.h"
#include "BoardModel.h"
#include "../core/Logger.h"
#include <QRegularExpression>
#include <QDateTime>
#include <QCoreApplication>
#include <QThread>
#include <QTimer>
#include <QEventLoop>

SketchInterpreter::SketchInterpreter(BoardModel *boardModel,
                                     std::function<void(int, int)> onDigitalWrite,
                                     QObject *parent)
    : QObject(parent), boardModel_(boardModel), onDigitalWrite_(std::move(onDigitalWrite)) {
    delayTimer_ = new QTimer(this);
    delayTimer_->setSingleShot(true);
    connect(delayTimer_, &QTimer::timeout, this, [this]() {
        if (running_ && savedLoopLineIndex_ >= 0)
            runLoopFrom(savedLoopLineIndex_);
    });
}

SketchInterpreter::~SketchInterpreter() = default;

void SketchInterpreter::setCode(const QString &code) {
    code_ = code;
}

static int findBlockStart(const QString &code, const QString &marker) {
    int idx = code.indexOf(marker);
    if (idx < 0)
        return -1;
    int brace = code.indexOf('{', idx);
    return brace >= 0 ? brace : -1;
}

static QStringList extractBlock(const QString &code, const QString &marker) {
    int brace = findBlockStart(code, marker);
    if (brace < 0)
        return QStringList();
    int depth = 1;
    int start = brace + 1;
    for (int i = start; i < code.size(); ++i) {
        QChar c = code.at(i);
        if (c == '{') ++depth;
        else if (c == '}') {
            --depth;
            if (depth == 0)
                return code.mid(start, i - start).trimmed().split('\n', Qt::SkipEmptyParts);
        }
    }
    return QStringList();
}

bool SketchInterpreter::extractSetupAndLoop() {
    setupLines_.clear();
    loopLines_.clear();
    // Allow "void setup()", "void setup ()", etc.
    setupLines_ = extractBlock(code_, "void setup()");
    if (setupLines_.isEmpty())
        setupLines_ = extractBlock(code_, "void setup (");
    loopLines_ = extractBlock(code_, "void loop()");
    if (loopLines_.isEmpty())
        loopLines_ = extractBlock(code_, "void loop (");
    for (QString &line : setupLines_)
        line = line.trimmed();
    for (QString &line : loopLines_)
        line = line.trimmed();
    // Pre-fill variables from global declarations outside setup()/loop()
    // Handles: const int ledPin = 9; int counter = 0; const int led0 = 9, led1 = 10, led2 = 11;
    QRegularExpression constRx("(?:const\\s+)?(?:unsigned\\s+long|long\\s+long|long|int|bool)\\s+(\\w+)\\s*=\\s*(.+?)\\s*;");
    for (const QString &line : code_.split('\n')) {
        QString trimmed = line.trimmed();
        // Skip lines inside setup/loop (they'll be executed normally)
        if (trimmed.startsWith("void ") || trimmed == "{" || trimmed == "}")
            continue;
        QRegularExpressionMatch m = constRx.match(trimmed);
        if (m.hasMatch()) {
            QString varName = m.captured(1).trimmed();
            QString rhs = m.captured(2).trimmed();
            if (rhs.contains(',')) {
                QStringList parts = rhs.split(',');
                if (!parts.isEmpty())
                    variables_[varName] = evalExpr(parts[0].trimmed());
                for (int i = 1; i < parts.size(); ++i) {
                    QString part = parts[i].trimmed();
                    int eq = part.indexOf('=');
                    if (eq > 0) {
                        QString v = part.left(eq).trimmed();
                        QString val = part.mid(eq + 1).trimmed();
                        variables_[v] = evalExpr(val);
                    }
                }
            } else {
                variables_[varName] = evalExpr(rhs);
            }
        }
    }
    return !setupLines_.isEmpty() || !loopLines_.isEmpty();
}

void SketchInterpreter::start() {
    stop();
    if (!boardModel_ || !onDigitalWrite_)
        return;
    boardModel_->reset();
    variables_.clear();
    startTimeMs_ = static_cast<qint64>(QDateTime::currentMSecsSinceEpoch());
    if (!extractSetupAndLoop()) {
        Logger::instance().error("Sketch parse failed: void setup() and void loop() not found");
        emit finished();
        return;
    }
    running_ = true;

    // Log parsed variables
    if (!variables_.isEmpty()) {
        QStringList varList;
        for (auto it = variables_.cbegin(); it != variables_.cend(); ++it)
            varList << QString("%1=%2").arg(it.key()).arg(it.value());
        Logger::instance().info(QString("Global variables: %1").arg(varList.join(", ")));
    }

    Logger::instance().info(QString("Running setup() — %1 line%2")
                                .arg(setupLines_.size()).arg(setupLines_.size() == 1 ? "" : "s"));
    runSetup();
    savedLoopLineIndex_ = -1;
    if (loopLines_.isEmpty()) {
        Logger::instance().warning("loop() is empty — nothing to execute");
        running_ = false;
        emit finished();
        return;
    }
    Logger::instance().info(QString("Entering loop() — %1 line%2, running continuously")
                                .arg(loopLines_.size()).arg(loopLines_.size() == 1 ? "" : "s"));
    QTimer::singleShot(0, this, [this]() { runLoopFrom(0); });
}

void SketchInterpreter::stop() {
    running_ = false;
    paused_ = false;
    delayTimer_->stop();
    savedLoopLineIndex_ = -1;
    pausedLineIndex_ = -1;
    pausedDelayRemaining_ = 0;
}

void SketchInterpreter::pause() {
    if (!running_ || paused_) return;
    paused_ = true;
    // If a delay timer is running, capture remaining time and stop it
    if (delayTimer_->isActive()) {
        pausedDelayRemaining_ = delayTimer_->remainingTime();
        if (pausedDelayRemaining_ < 0) pausedDelayRemaining_ = 0;
        delayTimer_->stop();
        pausedLineIndex_ = savedLoopLineIndex_;
    }
    Logger::instance().info("Interpreter paused");
}

void SketchInterpreter::resume() {
    if (!running_ || !paused_) return;
    paused_ = false;
    Logger::instance().info("Interpreter resumed");
    // If we paused mid-delay, restart the remaining delay
    if (pausedLineIndex_ >= 0 && pausedDelayRemaining_ > 0) {
        savedLoopLineIndex_ = pausedLineIndex_;
        delayTimer_->start(static_cast<int>(pausedDelayRemaining_));
    } else if (pausedLineIndex_ >= 0) {
        // Delay had finished or was 0 — resume from saved line
        int idx = pausedLineIndex_;
        pausedLineIndex_ = -1;
        QTimer::singleShot(0, this, [this, idx]() { runLoopFrom(idx); });
    } else {
        // Not mid-delay — restart loop
        QTimer::singleShot(0, this, [this]() { runLoopFrom(0); });
    }
    pausedLineIndex_ = -1;
    pausedDelayRemaining_ = 0;
}

void SketchInterpreter::runSetup() {
    for (int i = 0; i < setupLines_.size() && running_; ++i) {
        executeLine(setupLines_.at(i), setupLines_, i, false);
    }
}

void SketchInterpreter::runLoopFrom(int lineIndex) {
    if (!running_ || paused_) return;
    if (lineIndex < 0 || lineIndex >= loopLines_.size()) {
        if (running_ && !paused_ && !loopLines_.isEmpty() && !delayTimer_->isActive())
            QTimer::singleShot(0, this, [this]() { runLoopFrom(0); });
        return;
    }
    int idx = lineIndex;
    while (idx < loopLines_.size() && running_ && !paused_) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
        if (paused_) {
            // Save position so resume() can continue from here
            pausedLineIndex_ = idx;
            return;
        }
        executeLine(loopLines_.at(idx), loopLines_, idx, true);
        if (idx >= loopLines_.size())
            break;
        ++idx;
    }
    if (running_ && !paused_ && idx >= loopLines_.size() && !delayTimer_->isActive()) {

        // and button/UI can update. Prevents "same speed" timing
        const int throttleMs = 1;
        QTimer::singleShot(throttleMs, this, [this]() { runLoopFrom(0); });
    }
}

qint64 SketchInterpreter::evalExpr(const QString &expr) {
    QString e = expr.trimmed();
    if (e.isEmpty())
        return 0;
    // one level of outer parentheses so (sum - 4) etc
    if (e.startsWith('(') && e.size() > 1) {
        int depth = 0;
        for (int i = 0; i < e.size(); ++i) {
            if (e[i] == '(') ++depth;
            else if (e[i] == ')') { --depth; if (depth == 0) { if (i == e.size() - 1) return evalExpr(e.mid(1, i - 1).trimmed()); break; } }
        }
    }
    if (e.startsWith('!'))
        return evalExpr(e.mid(1).trimmed()) != 0 ? 0 : 1;
    if (e == "HIGH") return 1;
    if (e == "LOW") return 0;
    if (e == "true") return 1;
    if (e == "false") return 0;
    if (variables_.contains(e))
        return variables_.value(e);
    bool ok = false;
    qint64 v = e.toLongLong(&ok);
    if (ok) return v;
    // Arithmetic: additive (+, -) then multiplicative (*, /, %)
    int addPos = e.lastIndexOf(" + ");
    int subPos = e.lastIndexOf(" - ");
    int addSubPos = (addPos > subPos) ? addPos : subPos;
    if (addSubPos > 0) {
        QString leftStr = e.left(addSubPos).trimmed();
        QString rightStr = e.mid(addSubPos + 3).trimmed();
        qint64 leftVal = evalExpr(leftStr);
        qint64 rightVal = evalExpr(rightStr);
        if (addPos > subPos)
            return leftVal + rightVal;
        return leftVal - rightVal;
    }
    int mulPos = e.lastIndexOf(" * ");
    int divPos = e.lastIndexOf(" / ");
    int modPos = e.lastIndexOf(" % ");
    int mulDivModPos = qMax(qMax(mulPos, divPos), modPos);
    if (mulDivModPos > 0) {
        QString leftStr = e.left(mulDivModPos).trimmed();
        QString rightStr = e.mid(mulDivModPos + 3).trimmed();
        qint64 leftVal = evalExpr(leftStr);
        qint64 rightVal = evalExpr(rightStr);
        if (mulPos == mulDivModPos)
            return leftVal * rightVal;
        if (divPos == mulDivModPos) {
            if (rightVal == 0) return 0;
            return leftVal / rightVal;
        }
        if (rightVal == 0) return 0;
        return leftVal % rightVal;
    }
    // Bitwise: & (lower precedence) then >> — common for (count >> i) & 1 binary output
    int andPos = e.lastIndexOf(" & ");
    if (andPos > 0) {
        qint64 leftVal = evalExpr(e.left(andPos).trimmed());
        qint64 rightVal = evalExpr(e.mid(andPos + 3).trimmed());
        return (leftVal & rightVal);
    }
    int shiftPos = e.lastIndexOf(" >> ");
    if (shiftPos > 0) {
        qint64 leftVal = evalExpr(e.left(shiftPos).trimmed());
        qint64 rightVal = evalExpr(e.mid(shiftPos + 4).trimmed());
        int sh = static_cast<int>(rightVal);
        if (sh < 0 || sh > 63) return 0;
        return (leftVal >> sh);
    }
    // digitalRead(pin) - so conditions like "digitalRead(2) == LOW" work
    QRegularExpression digitalReadRx("digitalRead\\s*\\(\\s*(\\w+)\\s*\\)");
    auto drMatch = digitalReadRx.match(e);
    if (drMatch.hasMatch() && boardModel_)
        return boardModel_->getDigitalPin(static_cast<int>(evalExpr(drMatch.captured(1).trimmed())));
    QRegularExpression millisRx("millis\\s*\\(\\s*\\)");
    if (millisRx.match(e).hasMatch())
        return static_cast<qint64>(QDateTime::currentMSecsSinceEpoch()) - startTimeMs_;
    QRegularExpression subRx("\\(\\s*millis\\s*\\(\\s*\\)\\s*-\\s*(\\w+)\\s*\\)");
    auto m = subRx.match(e);
    if (m.hasMatch()) {
        qint64 varVal = variables_.value(m.captured(1).trimmed(), 0);
        return (static_cast<qint64>(QDateTime::currentMSecsSinceEpoch()) - startTimeMs_) - varVal;
    }
    return 0;
}

bool SketchInterpreter::evalCondition(const QString &cond) {
    QString c = cond.trimmed();
    if (c.isEmpty())
        return false;
    // Match compound conditions with && / || by taking the first part for now
    int andPos = c.indexOf(" && ");
    int orPos = c.indexOf(" || ");
    if (andPos > 0) {
        bool left = evalCondition(c.left(andPos).trimmed());
        bool right = evalCondition(c.mid(andPos + 4).trimmed());
        return left && right;
    }
    if (orPos > 0) {
        bool left = evalCondition(c.left(orPos).trimmed());
        bool right = evalCondition(c.mid(orPos + 4).trimmed());
        return left || right;
    }
    QRegularExpression neRx("(.+?)\\s*!=\\s*(.+)");
    auto m = neRx.match(c);
    if (m.hasMatch())
        return evalExpr(m.captured(1).trimmed()) != evalExpr(m.captured(2).trimmed());
    QRegularExpression eqRx("(.+?)\\s*==\\s*(.+)");
    m = eqRx.match(c);
    if (m.hasMatch())
        return evalExpr(m.captured(1).trimmed()) == evalExpr(m.captured(2).trimmed());
    QRegularExpression gteRx("(.+?)\\s*>=\\s*(.+)");
    m = gteRx.match(c);
    if (m.hasMatch())
        return evalExpr(m.captured(1).trimmed()) >= evalExpr(m.captured(2).trimmed());
    QRegularExpression lteRx("(.+?)\\s*<=\\s*(.+)");
    m = lteRx.match(c);
    if (m.hasMatch())
        return evalExpr(m.captured(1).trimmed()) <= evalExpr(m.captured(2).trimmed());
    QRegularExpression gtRx("(.+?)\\s*>\\s*(.+)");
    m = gtRx.match(c);
    if (m.hasMatch())
        return evalExpr(m.captured(1).trimmed()) > evalExpr(m.captured(2).trimmed());
    QRegularExpression ltRx("(.+?)\\s*<\\s*(.+)");
    m = ltRx.match(c);
    if (m.hasMatch())
        return evalExpr(m.captured(1).trimmed()) < evalExpr(m.captured(2).trimmed());
    return evalExpr(c) != 0;
}

int SketchInterpreter::findMatchingBrace(const QStringList &lines, int startIndex) const {
    if (startIndex < 0 || startIndex >= lines.size())
        return static_cast<int>(lines.size());
    int depth = 1;  // Opening '{' is on the if/else line (before startIndex), so start at depth 1
    for (int i = startIndex; i < lines.size(); ++i) {
        for (const QChar &ch : lines.at(i)) {
            if (ch == '{') ++depth;
            else if (ch == '}') {
                --depth;
                if (depth == 0) return i;
            }
        }
    }
    return lines.size();
}

// Static regexes — compiled once, reused every call (avoid per-line overhead)
static const QRegularExpression pinModeRx("pinMode\\s*\\(\\s*(\\w+)\\s*,\\s*(OUTPUT|INPUT_PULLUP|INPUT)\\s*\\)");
static const QRegularExpression digitalWriteRx("digitalWrite\\s*\\(\\s*([^,]+)\\s*,\\s*(.+)\\s*\\)");
static const QRegularExpression delayRx("delay\\s*\\(\\s*([^)]+)\\s*\\)");
// Typed declarations first (e.g. "int state = digitalRead(ledPin);", "unsigned long currentMillis = millis();")
static const QRegularExpression assignDigitalReadTypedRx("(?:int|unsigned\\s+long|long|bool)\\s+(\\w+)\\s*=\\s*digitalRead\\s*\\(\\s*(\\w+)\\s*\\)");
static const QRegularExpression assignMillisTypedRx("(?:unsigned\\s+long|long|int|bool)\\s+(\\w+)\\s*=\\s*millis\\s*\\(\\s*\\)");
static const QRegularExpression assignDigitalReadRx("(\\w+)\\s*=\\s*digitalRead\\s*\\(\\s*(\\w+)\\s*\\)");
static const QRegularExpression assignMillisRx("(\\w+)\\s*=\\s*millis\\s*\\(\\s*\\)");
static const QRegularExpression incRx("(\\w+)\\s*\\+\\+\\s*;");
static const QRegularExpression decRx("(\\w+)\\s*--\\s*;");
static const QRegularExpression assignPlusEqRx("(\\w+)\\s*\\+=\\s*(.+);");
static const QRegularExpression assignMinusEqRx("(\\w+)\\s*-=\\s*(.+);");
static const QRegularExpression ifNoBraceRx("if\\s*\\(\\s*(.+)\\)\\s*(.+;)\\s*");
// General typed assignment (e.g. "int x = 5;", "bool flag = true;")
static const QRegularExpression assignTypedRx("(?:const\\s+)?(?:unsigned\\s+long|long\\s+long|long|int|bool)\\s+(\\w+)\\s*=\\s*(.+);");
static const QRegularExpression assignRx("(\\w+)\\s*=\\s*(.+);");
static const QRegularExpression ifRx("if\\s*\\(\\s*(.+)\\)\\s*\\{");
static const QRegularExpression elseIfRx("\\}\\s*else\\s+if\\s*\\(\\s*(.+)\\)\\s*\\{");
static const QRegularExpression elseRx("\\}\\s*else\\s*\\{");
static const QRegularExpression switchRx("switch\\s*\\(\\s*(.+)\\)\\s*\\{");
static const QRegularExpression caseRx("case\\s+(\\w+)\\s*:");
static const QRegularExpression defaultRx("default\\s*:");
static const QRegularExpression breakRx("break\\s*;");

static constexpr int MAX_PIN = 19;   // Arduino Uno: D0-D13 + A0-A5
static constexpr int MAX_DELAY_MS = 60000;  // Cap delay at 60 seconds

void SketchInterpreter::executeLine(const QString &line, const QStringList &lines, int &lineIndex, bool inLoop, bool inNestedBlock) {
    QString l = line.trimmed();
    // Strip inline comments and trailing semicolons
    int commentPos = l.indexOf("//");
    if (commentPos == 0) return;          // full-line comment
    if (commentPos > 0) l = l.left(commentPos).trimmed();
    if (l.isEmpty()) return;
    // braces are no-ops (e.g. from if/else blocks); skip so they don't match other patterns
    if (l == "{" || l == "}")
        return;

    // for (init; condition; update) { ... }(check first so for-line is never treated as something else)
    if (l.startsWith(QLatin1String("for")) && (l.size() <= 3 || !l.at(3).isLetterOrNumber())) {
        int parenStart = l.indexOf('(');
        if (parenStart >= 0) {
            int depth = 1;
            int semicolon1 = -1, semicolon2 = -1, parenEnd = -1;
            for (int i = parenStart + 1; i < l.size(); ++i) {
                QChar c = l.at(i);
                if (c == '(') ++depth;
                else if (c == ')') {
                    --depth;
                    if (depth == 0) { parenEnd = i; break; }
                } else if (depth == 1 && c == ';') {
                    if (semicolon1 < 0) semicolon1 = i;
                    else if (semicolon2 < 0) semicolon2 = i;
                }
            }
            if (semicolon1 >= 0 && semicolon2 > semicolon1 && parenEnd > semicolon2) {
                QString initPart = l.mid(parenStart + 1, semicolon1 - parenStart - 1).trimmed();
                QString condPart = l.mid(semicolon1 + 1, semicolon2 - semicolon1 - 1).trimmed();
                QString updatePart = l.mid(semicolon2 + 1, parenEnd - semicolon2 - 1).trimmed();
                int blockStart = lineIndex + 1;
                int bodyStart = blockStart;
                int blockEnd;
                if (blockStart < lines.size() && lines.at(blockStart).trimmed() == QLatin1String("{")) {
                    bodyStart = blockStart + 1;
                    blockEnd = findMatchingBrace(lines, bodyStart);
                } else {
                    blockEnd = findMatchingBrace(lines, blockStart);
                }
                if (blockEnd < static_cast<int>(lines.size())) {
                    if (!initPart.isEmpty()) {
                        QString initStmt = initPart;
                        if (!initStmt.endsWith(';')) initStmt += ";";
                        int dummyIdx = 0;
                        executeLine(initStmt, lines, dummyIdx, inLoop, true);
                    }
                    while (running_ && !paused_ && evalCondition(condPart)) {
                        for (int i = bodyStart; i < blockEnd && running_ && !paused_; ++i) {
                            int lineIdx = i;
                            executeLine(lines.at(i), lines, lineIdx, inLoop, true);
                            if (lineIdx > i) i = lineIdx - 1;
                        }
                        if (!updatePart.isEmpty()) {
                            QString updateStmt = updatePart;
                            if (!updateStmt.endsWith(';')) updateStmt += ";";
                            int dummyIdx = 0;
                            executeLine(updateStmt, lines, dummyIdx, inLoop, true);
                        }
                    }
                    lineIndex = blockEnd;
                    return;
                }
            }
        }
    }

    // while (cond) { ... } (e.g. wait for button release)
    if (l.startsWith(QLatin1String("while")) && (l.size() <= 5 || !l.at(5).isLetterOrNumber())) {
        int parenStart = l.indexOf('(');
        if (parenStart >= 0) {
            int depth = 1;
            int parenEnd = -1;
            for (int i = parenStart + 1; i < l.size(); ++i) {
                QChar c = l.at(i);
                if (c == '(') ++depth;
                else if (c == ')') {
                    --depth;
                    if (depth == 0) { parenEnd = i; break; }
                }
            }
            if (parenEnd > parenStart) {
                QString condPart = l.mid(parenStart + 1, parenEnd - parenStart - 1).trimmed();
                int blockStart = lineIndex + 1;
                int bodyStart = blockStart;
                int blockEnd;
                if (blockStart < lines.size() && lines.at(blockStart).trimmed() == QLatin1String("{")) {
                    bodyStart = blockStart + 1;
                    blockEnd = findMatchingBrace(lines, bodyStart);
                } else {
                    blockEnd = findMatchingBrace(lines, blockStart);
                }
                if (blockEnd < static_cast<int>(lines.size())) {
                    while (running_ && !paused_ && evalCondition(condPart)) {
                        for (int i = bodyStart; i < blockEnd && running_ && !paused_; ++i) {
                            int lineIdx = i;
                            executeLine(lines.at(i), lines, lineIdx, inLoop, true);
                            if (lineIdx > i) i = lineIdx - 1;
                        }
                    }
                    lineIndex = blockEnd;
                    return;
                }
            }
        }
    }

    //  pinMode(pin, mode)
    auto match = pinModeRx.match(l);
    if (match.hasMatch()) {
        int pin = static_cast<int>(evalExpr(match.captured(1).trimmed()));
        if (pin < 0 || pin > MAX_PIN) {
            Logger::instance().warning(QString("pinMode: invalid pin %1 (valid: 0-%2)").arg(pin).arg(MAX_PIN));
            return;
        }
        boardModel_->setPinMode(pin, (match.captured(2) == "OUTPUT"));
        return;
    }

    // digitalWrite(pin, value)
    match = digitalWriteRx.match(l);
    if (match.hasMatch()) {
        int pin = static_cast<int>(evalExpr(match.captured(1).trimmed()));
        if (pin < 0 || pin > MAX_PIN) {
            Logger::instance().warning(QString("digitalWrite: invalid pin %1").arg(pin));
            return;
        }
        int val = static_cast<int>(evalExpr(match.captured(2).trimmed())) ? 1 : 0;
        boardModel_->setDigitalPin(pin, val);
        if (onDigitalWrite_)
            onDigitalWrite_(pin, val);
        return;
    }

    // delay(ms)
    match = delayRx.match(l);
    if (match.hasMatch()) {
        int ms = static_cast<int>(evalExpr(match.captured(1).trimmed()));
        if (ms < 0) ms = 0;
        if (ms > MAX_DELAY_MS) {
            Logger::instance().warning(QString("delay(%1) capped to %2 ms").arg(ms).arg(MAX_DELAY_MS));
            ms = MAX_DELAY_MS;
        }
        // Inside for/while (inNestedBlock): block so loop runs to completion, but process events so UI updates.
        if (inLoop && !inNestedBlock) {
            savedLoopLineIndex_ = lineIndex + 1;
            delayTimer_->start(ms);
            lineIndex = lines.size();  // break out of while loop
            return;
        }
        // Blocking delay: sleep in small chunks and process events so LED/UI repaints during for-loop
        const int chunkMs = 20;
        int remaining = ms;
        while (remaining > 0 && running_ && !paused_) {
            int sleepMs = qMin(chunkMs, remaining);
            QThread::msleep(static_cast<unsigned long>(sleepMs));
            QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
            remaining -= sleepMs;
        }
        return;
    }

    // type var = digitalRead(pin); (e.g. "int state = digitalRead(ledPin);")
    match = assignDigitalReadTypedRx.match(l);
    if (match.hasMatch()) {
        QString varName = match.captured(1).trimmed();
        int pin = static_cast<int>(evalExpr(match.captured(2).trimmed()));
        if (pin < 0 || pin > MAX_PIN) {
            Logger::instance().warning(QString("digitalRead: invalid pin %1").arg(pin));
            return;
        }
        variables_[varName] = boardModel_->getDigitalPin(pin);
        return;
    }

    // var = digitalRead(pin)
    match = assignDigitalReadRx.match(l);
    if (match.hasMatch()) {
        QString varName = match.captured(1).trimmed();
        int pin = static_cast<int>(evalExpr(match.captured(2).trimmed()));
        if (pin < 0 || pin > MAX_PIN) {
            Logger::instance().warning(QString("digitalRead: invalid pin %1").arg(pin));
            return;
        }
        variables_[varName] = boardModel_->getDigitalPin(pin);
        return;
    }

    // type var = millis(); (e.g. "unsigned long currentMillis = millis();")
    match = assignMillisTypedRx.match(l);
    if (match.hasMatch()) {
        QString varName = match.captured(1).trimmed();
        variables_[varName] = static_cast<qint64>(QDateTime::currentMSecsSinceEpoch()) - startTimeMs_;
        return;
    }

    // var = millis()
    match = assignMillisRx.match(l);
    if (match.hasMatch()) {
        QString varName = match.captured(1).trimmed();
        variables_[varName] = static_cast<qint64>(QDateTime::currentMSecsSinceEpoch()) - startTimeMs_;
        return;
    }

    // var++;
    match = incRx.match(l);
    if (match.hasMatch()) {
        QString varName = match.captured(1).trimmed();
        variables_[varName] = variables_.value(varName, 0) + 1;
        return;
    }

    // var--;
    match = decRx.match(l);
    if (match.hasMatch()) {
        QString varName = match.captured(1).trimmed();
        variables_[varName] = variables_.value(varName, 0) - 1;
        return;
    }

    // var += expr; / var -= expr;
    match = assignPlusEqRx.match(l);
    if (match.hasMatch()) {
        QString varName = match.captured(1).trimmed();
        QString rhs = match.captured(2).trimmed();
        if (rhs.endsWith(';')) rhs.chop(1);
        rhs = rhs.trimmed();
        variables_[varName] = variables_.value(varName, 0) + evalExpr(rhs);
        return;
    }
    match = assignMinusEqRx.match(l);
    if (match.hasMatch()) {
        QString varName = match.captured(1).trimmed();
        QString rhs = match.captured(2).trimmed();
        if (rhs.endsWith(';')) rhs.chop(1);
        rhs = rhs.trimmed();
        variables_[varName] = variables_.value(varName, 0) - evalExpr(rhs);
        return;
    }

    // if (cond) singleStatement;
    match = ifNoBraceRx.match(l);
    if (match.hasMatch()) {
        if (evalCondition(match.captured(1).trimmed())) {
            QString stmt = match.captured(2).trimmed();
            if (!stmt.isEmpty()) {
                int dummyIdx = 0;
                executeLine(stmt, lines, dummyIdx, inLoop);
            }
        }
        return;
    }

    // type var = expression; (e.g. "int x = 5;", "bool on = true;", "const int a=1, b=2;")
    match = assignTypedRx.match(l);
    if (match.hasMatch()) {
        QString varName = match.captured(1).trimmed();
        QString rhs = match.captured(2).trimmed();
        if (rhs.endsWith(';')) rhs.chop(1);
        rhs = rhs.trimmed();
        if (rhs.contains(',')) {
            // Comma-separated declarations: first value for varName, then "name = value" for the rest
            QStringList parts = rhs.split(',');
            if (!parts.isEmpty())
                variables_[varName] = evalExpr(parts[0].trimmed());
            for (int i = 1; i < parts.size(); ++i) {
                QString part = parts[i].trimmed();
                int eq = part.indexOf('=');
                if (eq > 0) {
                    QString v = part.left(eq).trimmed();
                    QString val = part.mid(eq + 1).trimmed();
                    variables_[v] = evalExpr(val);
                }
            }
        } else {
            variables_[varName] = evalExpr(rhs);
        }
        return;
    }

    // var = expression;
    match = assignRx.match(l);
    if (match.hasMatch()) {
        QString varName = match.captured(1).trimmed();
        QString rhs = match.captured(2).trimmed();
        if (rhs.endsWith(';')) rhs.chop(1);
        rhs = rhs.trimmed();
        variables_[varName] = evalExpr(rhs);
        return;
    }

    // switch (expr) { case 0: ... break; ... }
    match = switchRx.match(l);
    if (match.hasMatch()) {
        qint64 value = evalExpr(match.captured(1).trimmed());
        int blockStart = lineIndex + 1;
        int blockEnd = findMatchingBrace(lines, blockStart);
        if (blockEnd >= static_cast<int>(lines.size()) && !lines.isEmpty())
            blockEnd = lines.size() - 1;
        int runFrom = -1;
        for (int i = blockStart; i <= blockEnd; ++i) {
            QString bline = lines.at(i).trimmed();
            int commentPos = bline.indexOf("//");
            if (commentPos == 0) continue;
            if (commentPos > 0) bline = bline.left(commentPos).trimmed();
            auto caseMatch = caseRx.match(bline);
            if (caseMatch.hasMatch()) {
                if (evalExpr(caseMatch.captured(1).trimmed()) == value) {
                    runFrom = i + 1;
                    break;
                }
            }
            if (defaultRx.match(bline).hasMatch() && runFrom < 0) {
                runFrom = i + 1;
                break;
            }
        }
        if (runFrom >= 0) {
            for (int i = runFrom; i <= blockEnd && running_; ++i) {
                QString bline = lines.at(i).trimmed();
                int commentPos = bline.indexOf("//");
                if (commentPos == 0) continue;
                if (commentPos > 0) bline = bline.left(commentPos).trimmed();
                if (bline.isEmpty()) continue;
                if (breakRx.match(bline).hasMatch())
                    break;
                if (caseRx.match(bline).hasMatch() || defaultRx.match(bline).hasMatch())
                    break;  // next case/default ends this block
                int lineIdx = i;
                executeLine(lines.at(i), lines, lineIdx, inLoop);
                // Propagate so delay() inside switch breaks out of runLoopFrom
                lineIndex = lineIdx;
                if (lineIdx >= static_cast<int>(lines.size()))
                    return;  // delay() was called — stop immediately
                // Skip any block we executed (e.g. if/else sets lineIdx to block end)
                if (lineIdx > i)
                    i = lineIdx - 1;  // -1 so for loop ++i lands on next line after block
            }
        }
        lineIndex = blockEnd;
        return;
    }

    // if / else if / else
    match = ifRx.match(l);
    if (match.hasMatch()) {
        QString cond = match.captured(1).trimmed();
        int blockStart = lineIndex + 1;
        int blockEnd = findMatchingBrace(lines, blockStart);
        bool taken = evalCondition(cond);

        if (taken) {
            for (int i = blockStart; i < blockEnd && running_; ++i) {
                executeLine(lines.at(i), lines, i, inLoop);
                lineIndex = i;  // propagate so delay() inside if breaks out of runLoopFrom
                if (i >= static_cast<int>(lines.size()))
                    return;  // delay() was called — stop immediately
            }
        }
        lineIndex = blockEnd;

        // Check for else if / else after the closing brace
        while (lineIndex + 1 < lines.size()) {
            QString nextLine = lines.at(lineIndex + 1).trimmed();
            // Merge "} else ..." that may be split across the brace line and the next
            QString combined = lines.at(lineIndex).trimmed() + " " + nextLine;

            // else if (cond) {
            auto elseIfMatch = elseIfRx.match(combined);
            if (!elseIfMatch.hasMatch())
                elseIfMatch = elseIfRx.match(nextLine);
            if (elseIfMatch.hasMatch()) {
                lineIndex += 1;  // skip to else-if line (contains "} else if (cond) {")
                int eiBlockStart = lineIndex + 1;  // body starts on next line
                int eiBlockEnd = findMatchingBrace(lines, eiBlockStart);
                if (!taken && evalCondition(elseIfMatch.captured(1).trimmed())) {
                    taken = true;
                    for (int i = eiBlockStart; i < eiBlockEnd && running_; ++i) {
                        executeLine(lines.at(i), lines, i, inLoop);
                        lineIndex = i;
                        if (i >= static_cast<int>(lines.size()))
                            return;
                    }
                }
                lineIndex = eiBlockEnd;
                continue;
            }

            // else {
            auto elseMatch = elseRx.match(combined);
            if (!elseMatch.hasMatch())
                elseMatch = elseRx.match(nextLine);
            if (elseMatch.hasMatch()) {
                lineIndex += 1;  // skip to line after "} else {" — that line is the first line of the else body
                int elseBlockStart = lineIndex;
                int elseBlockEnd = findMatchingBrace(lines, elseBlockStart);
                if (!taken) {
                    for (int i = elseBlockStart; i < elseBlockEnd && running_; ++i) {
                        executeLine(lines.at(i), lines, i, inLoop);
                        lineIndex = i;
                        if (i >= static_cast<int>(lines.size()))
                            return;
                    }
                }
                lineIndex = elseBlockEnd;
            }
            break;  // done with if/else chain
        }
        return;
    }

    // Serial.* as no-op so common Arduino code does not break the simulator
    if (l.startsWith(QLatin1String("Serial.")))
        return;

    // Unknown lines (e.g. analogRead, other APIs) are skipped so the simulator keeps running
}
