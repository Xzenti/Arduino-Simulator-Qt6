#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QLabel>
#include "../core/Logger.h"

class OutputPane : public QWidget {
    Q_OBJECT

public:
    explicit OutputPane(QWidget* parent = nullptr, const QString& title = "Output");
    ~OutputPane() override;

    void appendMessage(const QString& text, Logger::LogLevel level = Logger::Info);
    void clearLogs();
    void setFilterLevel(Logger::LogLevel level);
    void applyTheme();

    // Optional: Enable/disable timestamps
    void enableTimestamps(bool enable);

public slots:
    void onLogMessage(const QString& text, int level);

signals:
    void messageReceived(const QString& text);

private:
    void setupUI();
    QColor getColorForLevel(Logger::LogLevel level) const;
    Logger::LogLevel detectLevelFromText(const QString& text) const;

private:
    QLabel* titleLabel;
    QTextEdit* outputText;
    Logger::LogLevel filterLevel;
    QString title;
    bool timestampsEnabled;
};
