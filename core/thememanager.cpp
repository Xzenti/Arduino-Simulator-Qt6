#include "ThemeManager.h"
#include <QApplication>

ThemeManager::ThemeManager() : m_currentTheme(Light) {
}

ThemeManager::~ThemeManager() = default;

void ThemeManager::setTheme(Theme theme) {
    if (m_currentTheme != theme) {
        m_currentTheme = theme;

        // Apply palette to application
        qApp->setPalette(getApplicationPalette());

        emit themeChanged(theme);
    }
}

void ThemeManager::toggleTheme() {
    setTheme(m_currentTheme == Light ? Dark : Light);
}

QPalette ThemeManager::getApplicationPalette() const {
    QPalette palette;

    if (m_currentTheme == Light) {
        // GitHub Light theme
        palette.setColor(QPalette::Window, QColor(255, 255, 255));        // #ffffff
        palette.setColor(QPalette::WindowText, QColor(36, 41, 46));       // #24292e
        palette.setColor(QPalette::Base, QColor(255, 255, 255));          // #ffffff
        palette.setColor(QPalette::AlternateBase, QColor(246, 248, 250)); // #f6f8fa
        palette.setColor(QPalette::Text, QColor(36, 41, 46));             // #24292e
        palette.setColor(QPalette::Button, QColor(246, 248, 250));        // #f6f8fa
        palette.setColor(QPalette::ButtonText, QColor(36, 41, 46));       // #24292e
        palette.setColor(QPalette::Highlight, QColor(3, 102, 214));       // #0366d6
        palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
        palette.setColor(QPalette::Link, QColor(3, 102, 214));            // #0366d6
    } else {
        // GitHub Dark theme
        palette.setColor(QPalette::Window, QColor(13, 17, 23));           // #0d1117
        palette.setColor(QPalette::WindowText, QColor(201, 209, 217));    // #c9d1d9
        palette.setColor(QPalette::Base, QColor(13, 17, 23));             // #0d1117
        palette.setColor(QPalette::AlternateBase, QColor(22, 27, 34));    // #161b22
        palette.setColor(QPalette::Text, QColor(201, 209, 217));          // #c9d1d9
        palette.setColor(QPalette::Button, QColor(33, 38, 45));           // #21262d
        palette.setColor(QPalette::ButtonText, QColor(201, 209, 217));    // #c9d1d9
        palette.setColor(QPalette::Highlight, QColor(88, 166, 255));      // #58a6ff
        palette.setColor(QPalette::HighlightedText, QColor(13, 17, 23));
        palette.setColor(QPalette::Link, QColor(88, 166, 255));           // #58a6ff
    }

    return palette;
}

QString ThemeManager::getEditorStyleSheet() const {
    if (m_currentTheme == Light) {
        return R"(
            QPlainTextEdit {
                background-color: #ffffff;
                color: #24292e;
                border: 1px solid #e1e4e8;
                selection-background-color: #0366d6;
                selection-color: #ffffff;
            }
            QScrollBar:vertical {
                border: none;
                background: #f6f8fa;
                width: 12px;
            }
            QScrollBar::handle:vertical {
                background: #d1d5da;
                min-height: 20px;
                border-radius: 6px;
            }
            QScrollBar::handle:vertical:hover {
                background: #959da5;
            }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                height: 0px;
            }
            QScrollBar:horizontal {
                border: none;
                background: #f6f8fa;
                height: 12px;
            }
            QScrollBar::handle:horizontal {
                background: #d1d5da;
                min-width: 20px;
                border-radius: 6px;
            }
            QScrollBar::handle:horizontal:hover {
                background: #959da5;
            }
            QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
                width: 0px;
            }
        )";
    } else {
        return R"(
            QPlainTextEdit {
                background-color: #0d1117;
                color: #c9d1d9;
                border: 1px solid #30363d;
                selection-background-color: #58a6ff;
                selection-color: #0d1117;
            }
            QScrollBar:vertical {
                border: none;
                background: #161b22;
                width: 12px;
            }
            QScrollBar::handle:vertical {
                background: #484f58;
                min-height: 20px;
                border-radius: 6px;
            }
            QScrollBar::handle:vertical:hover {
                background: #6e7681;
            }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                height: 0px;
            }
            QScrollBar:horizontal {
                border: none;
                background: #161b22;
                height: 12px;
            }
            QScrollBar::handle:horizontal {
                background: #484f58;
                min-width: 20px;
                border-radius: 6px;
            }
            QScrollBar::handle:horizontal:hover {
                background: #6e7681;
            }
            QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
                width: 0px;
            }
        )";
    }
}

QString ThemeManager::getOutputPaneStyleSheet() const {
    if (m_currentTheme == Light) {
        return R"(
            QTextEdit {
                background-color: #ffffff;
                color: #24292e;
                border: 1px solid #e1e4e8;
                font-family: 'Consolas', monospace;
                font-size: 9pt;
            }
        )";
    } else {
        return R"(
            QTextEdit {
                background-color: #0d1117;
                color: #c9d1d9;
                border: 1px solid #30363d;
                font-family: 'Consolas', monospace;
                font-size: 9pt;
            }
        )";
    }
}

QString ThemeManager::getButtonStyleSheet() const {
    if (m_currentTheme == Light) {
        return R"(
            QPushButton {
                background-color: #2ea043;
                color: #ffffff;
                border: none;
                padding: 6px 12px;
                border-radius: 3px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #2c974b;
            }
            QPushButton:pressed {
                background-color: #1a7f37;
            }
            QPushButton:disabled {
                background-color: #d1d5da;
                color: #6a737d;
            }
        )";
    } else {
        return R"(
            QPushButton {
                background-color: #238636;
                color: #ffffff;
                border: 1px solid #30363d;
                padding: 6px 12px;
                border-radius: 3px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #2ea043;
            }
            QPushButton:pressed {
                background-color: #1a7f37;
            }
            QPushButton:disabled {
                background-color: #21262d;
                color: #6e7681;
            }
        )";
    }
}

QString ThemeManager::getLineNumberStyleSheet() const {
    if (m_currentTheme == Light) {
        return "background-color: #f6f8fa; color: #6a737d; border-right: 1px solid #e1e4e8;";
    } else {
        return "background-color: #161b22; color: #8b949e; border-right: 1px solid #30363d;";
    }
}

QColor ThemeManager::backgroundColor() const {
    return m_currentTheme == Light ? QColor(255, 255, 255) : QColor(13, 17, 23);
}

QColor ThemeManager::foregroundColor() const {
    return m_currentTheme == Light ? QColor(36, 41, 46) : QColor(201, 209, 217);
}

QColor ThemeManager::borderColor() const {
    return m_currentTheme == Light ? QColor(225, 228, 232) : QColor(48, 54, 61);
}

QColor ThemeManager::highlightColor() const {
    return m_currentTheme == Light ? QColor(3, 102, 214) : QColor(88, 166, 255);
}

QColor ThemeManager::lineHighlightColor() const {
    return m_currentTheme == Light ? QColor(246, 248, 250) : QColor(22, 27, 34);
}

QColor ThemeManager::selectionColor() const {
    return m_currentTheme == Light ? QColor(3, 102, 214) : QColor(88, 166, 255);
}

QColor ThemeManager::logDebugColor() const {
    return m_currentTheme == Light ? QColor(106, 115, 125) : QColor(139, 148, 158); // Gray
}

QColor ThemeManager::logInfoColor() const {
    return m_currentTheme == Light ? QColor(3, 102, 214) : QColor(88, 166, 255); // Blue
}

QColor ThemeManager::logWarningColor() const {
    return m_currentTheme == Light ? QColor(210, 142, 31) : QColor(235, 192, 52); // Orange/Yellow
}

QColor ThemeManager::logErrorColor() const {
    return m_currentTheme == Light ? QColor(215, 58, 73) : QColor(248, 81, 73); // Red
}
