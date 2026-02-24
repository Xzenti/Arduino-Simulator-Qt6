#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QObject>
#include <QDateTime>

class Logger : public QObject {
    Q_OBJECT

public:
    enum LogLevel {
        Debug,
        Info,
        Warning,
        Error
    };
    Q_ENUM(LogLevel)

    static Logger& instance() {
        static Logger _instance;
        return _instance;
    }

    void log(const QString &message);

    // New severity-based logging methods
    void debug(const QString &message);
    void info(const QString &message);
    void warning(const QString &message);
    void error(const QString &message);
    void logWithLevel(LogLevel level, const QString &message);

    void clear();

signals:
    void messageLogged(const QString &message, LogLevel level);
    void messageLegacy(const QString &message);

private:
    Logger();
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    QString formatMessage(LogLevel level, const QString &message);
};

#endif // LOGGER_H
