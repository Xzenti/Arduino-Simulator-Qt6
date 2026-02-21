#include "Logger.h"
#include <QDateTime>

Logger::Logger() = default;

Logger::~Logger() = default;

QString Logger::formatMessage(LogLevel level, const QString &message) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    return QString("[%1] %2").arg(timestamp, message);
}

void Logger::log(const QString &message) {
    QString formattedMessage = formatMessage(LogLevel::Info, message);
    emit messageLogged(formattedMessage, LogLevel::Info);
    emit messageLegacy(formattedMessage);
}

void Logger::debug(const QString &message) {
    logWithLevel(LogLevel::Debug, message);
}

void Logger::info(const QString &message) {
    logWithLevel(LogLevel::Info, message);
}

void Logger::warning(const QString &message) {
    logWithLevel(LogLevel::Warning, message);
}

void Logger::error(const QString &message) {
    logWithLevel(LogLevel::Error, message);
}

void Logger::logWithLevel(LogLevel level, const QString &message) {
    QString formattedMessage = formatMessage(level, message);
    emit messageLogged(formattedMessage, level);
}

void Logger::clear() {
    // No-op unless UI is connected to clear display
}
