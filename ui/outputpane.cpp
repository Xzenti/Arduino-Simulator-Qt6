#include "OutputPane.h"
#include "../core/ThemeManager.h"
#include <QVBoxLayout>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QDateTime>
#include <QToolBar>
#include <QComboBox>
#include <QCheckBox>

OutputPane::OutputPane(QWidget* parent, const QString& titleText)
    : QWidget(parent),
    titleLabel(new QLabel(this)),
    outputText(new QTextEdit(this)),
    filterLevel(Logger::Debug),
    title(titleText),
    timestampsEnabled(true)
{
    setupUI();
    applyTheme();
}

OutputPane::~OutputPane() = default;

void OutputPane::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    QWidget* titleBar = new QWidget(this);
    QHBoxLayout* titleBarLayout = new QHBoxLayout(titleBar);
    titleBarLayout->setContentsMargins(4, 4, 4, 4);

    titleLabel->setText(title);
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");

    QComboBox* filterCombo = new QComboBox(this);
    filterCombo->addItem("Debug", Logger::Debug);
    filterCombo->addItem("Info", Logger::Info);
    filterCombo->addItem("Warning", Logger::Warning);
    filterCombo->addItem("Error", Logger::Error);
    filterCombo->setCurrentIndex(filterCombo->findData(filterLevel));
    connect(filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this, filterCombo](int index) {
                setFilterLevel(static_cast<Logger::LogLevel>(
                    filterCombo->itemData(index).toInt()));
            });

    QCheckBox* timestampCheck = new QCheckBox("Timestamps", this);
    timestampCheck->setChecked(timestampsEnabled);
    connect(timestampCheck, &QCheckBox::toggled, this, &OutputPane::enableTimestamps);

    titleBarLayout->addWidget(titleLabel, 1);
    titleBarLayout->addWidget(filterCombo);
    titleBarLayout->addWidget(timestampCheck);
    titleBar->setLayout(titleBarLayout);

    outputText->setReadOnly(true);
    outputText->setLineWrapMode(QTextEdit::NoWrap);
    outputText->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    outputText->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    outputText->setFrameShape(QFrame::NoFrame);
    // Font will be set via stylesheet in applyTheme()

    layout->addWidget(titleBar);
    layout->addWidget(outputText);
    setLayout(layout);
}

void OutputPane::appendMessage(const QString& text, Logger::LogLevel level) {
    if (level < filterLevel)
        return;

    QString message = text;
    if (timestampsEnabled) {
        QString timeStr = QDateTime::currentDateTime().toString("HH:mm:ss");
        message = QString("[%1] %2").arg(timeStr, text);
    }

    QTextCursor cursor(outputText->textCursor());
    cursor.movePosition(QTextCursor::End);

    QTextCharFormat format;
    format.setForeground(getColorForLevel(level));
    format.setFont(outputText->font());

    cursor.setCharFormat(format);
    cursor.insertText(message + "\n");
    outputText->ensureCursorVisible();

    emit messageReceived(message);
}

Logger::LogLevel OutputPane::detectLevelFromText(const QString& text) const {
    QString msg = text.toLower();
    if (msg.contains("error") || msg.contains("fail"))
        return Logger::Error;
    if (msg.contains("warn"))
        return Logger::Warning;
    if (msg.contains("debug"))
        return Logger::Debug;
    return Logger::Info;
}

QColor OutputPane::getColorForLevel(Logger::LogLevel level) const {
    switch (level) {
    case Logger::Debug:   return ThemeManager::instance().logDebugColor();
    case Logger::Info:    return ThemeManager::instance().logInfoColor();
    case Logger::Warning: return ThemeManager::instance().logWarningColor();
    case Logger::Error:   return ThemeManager::instance().logErrorColor();
    default:              return ThemeManager::instance().foregroundColor();
    }
}

void OutputPane::clearLogs() {
    outputText->clear();
}

void OutputPane::setFilterLevel(Logger::LogLevel level) {
    filterLevel = level;
}

void OutputPane::enableTimestamps(bool enable) {
    timestampsEnabled = enable;
}

void OutputPane::onLogMessage(const QString& text, int level) {
    appendMessage(text, static_cast<Logger::LogLevel>(level));
}

void OutputPane::applyTheme() {
    outputText->setStyleSheet(ThemeManager::instance().getOutputPaneStyleSheet());

    bool light = ThemeManager::instance().backgroundColor() == QColor(255, 255, 255);

    if (!light) {
        setStyleSheet(R"(
            OutputPane {
                background-color: #0a0c10;
            }
            QWidget {
                background-color: transparent;
                color: #b0e0ff;
            }
            QLabel {
                color: #00b7ff;
            }
            QComboBox, QCheckBox {
                background-color: #14161a;
                color: #b0e0ff;
                border: 1px solid #00b7ff;
                border-radius: 4px;
                padding: 2px 6px;
            }
            QComboBox::drop-down {
                border: none;
            }
            QComboBox QAbstractItemView {
                background-color: #14161a;
                color: #b0e0ff;
                selection-background-color: #007acc;
            }
            QCheckBox::indicator {
                width: 16px;
                height: 16px;
            }
            QCheckBox::indicator:unchecked {
                border: 1px solid #00b7ff;
                background-color: #14161a;
            }
            QCheckBox::indicator:checked {
                border: 1px solid #00b7ff;
                background-color: #00b7ff;
            }
            QTextEdit {
                font-family: 'JetBrains Mono', 'Courier New', monospace;
                font-size: 11pt;
            }
        )");
    } else {
        setStyleSheet(R"(
            OutputPane {
                background-color: #e6f0ff;
            }
            QWidget {
                background-color: transparent;
                color: #00264d;
            }
            QLabel {
                color: #0099ff;
            }
            QComboBox, QCheckBox {
                background-color: #ffffff;
                color: #00264d;
                border: 1px solid #0099ff;
                border-radius: 4px;
                padding: 2px 6px;
            }
            QComboBox::drop-down {
                border: none;
            }
            QComboBox QAbstractItemView {
                background-color: #ffffff;
                color: #00264d;
                selection-background-color: #b3d9ff;
            }
            QCheckBox::indicator {
                width: 16px;
                height: 16px;
            }
            QCheckBox::indicator:unchecked {
                border: 1px solid #0099ff;
                background-color: #ffffff;
            }
            QCheckBox::indicator:checked {
                border: 1px solid #0099ff;
                background-color: #0099ff;
            }
            QTextEdit {
                font-family: 'JetBrains Mono', 'Courier New', monospace;
                font-size: 11pt;
            }
        )");
    }

    titleLabel->setStyleSheet(light ?
                                  "font-weight: bold; font-size: 14px; color: #0099ff;" :
                                  "font-weight: bold; font-size: 14px; color: #00b7ff;");
}
