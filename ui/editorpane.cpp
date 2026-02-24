#include "EditorPane.h"
#include <QVBoxLayout>
#include <QLabel>
#include "../core/ThemeManager.h"

#include <QFile>
#include <QTextStream>

EditorPane::EditorPane(QWidget *parent)
    : QWidget(parent),
    codeEditor(std::make_unique<QPlainTextEdit>()),highlighter(nullptr)
{
    setupUI();
    applyTheme(true);  // default dark mode
}

EditorPane::~EditorPane() = default;

//////////////////////////////////////////////////////////////
// UI SETUP
//////////////////////////////////////////////////////////////

void EditorPane::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    QLabel *titleLabel = new QLabel("Arduino Code Editor");
    titleLabel->setObjectName("paneTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "QLabel#paneTitle {"
        "   font-weight: bold;"
        "   font-size: 14px;"
        "   padding: 4px;"
        "   border-radius: 4px;"
        "}"
        );

    codeEditor->setPlaceholderText(
        "// Enter your Arduino sketch here\n"
        "void setup() {\n"
        "  // Setup code\n"
        "}\n\n"
        "void loop() {\n"
        "  digitalWrite(13, HIGH);\n"
        "  delay(500);\n"
        "  digitalWrite(13, LOW);\n"
        "  delay(500);\n"
        "}"
        );

    codeEditor->setLineWrapMode(QPlainTextEdit::NoWrap);
    codeEditor->setFrameShape(QFrame::NoFrame);

    layout->addWidget(titleLabel);
    layout->addWidget(codeEditor.get());

    setLayout(layout);
}

//////////////////////////////////////////////////////////////
// BASIC OPERATIONS
//////////////////////////////////////////////////////////////

QString EditorPane::getCode() const
{
    return codeEditor->toPlainText();
}

void EditorPane::setCode(const QString &code)
{
    codeEditor->setPlainText(code);
    m_currentContent = code;
    m_savedContent = code;  // When setting code, consider it as saved state
    updateUnsavedIndicator();
}

void EditorPane::clear()
{
    codeEditor->clear();
}

bool EditorPane::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    codeEditor->setPlainText(in.readAll());
    file.close();
    return true;
}

bool EditorPane::saveToFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << codeEditor->toPlainText();
    file.close();
    return true;
}

void EditorPane::onContentChanged() {
    // Update current content and check if it differs from saved
    m_currentContent = codeEditor->toPlainText();
    updateUnsavedIndicator();
}

void EditorPane::updateUnsavedIndicator() {
    // Update the dirty indicator in the line number area
    // Show ribbon only if current content differs from saved content
    //LineNumberArea *lineNumberArea = codeEditor->getLineNumberArea();
    /*bool isDifferent = (m_currentContent != m_savedContent);
    lineNumberArea->setDirty(isDifferent); */ // Show ribbon only if content differs from saved
}
//////////////////////////////////////////////////////////////
// THEME SYSTEM
//////////////////////////////////////////////////////////////

void EditorPane::applyTheme(bool darkMode)
{
    m_darkMode = darkMode;

    if (m_darkMode)
    {
        // 🌃 Neon Blue Dark Theme with updated font
        setStyleSheet(R"(
            EditorPane {
                background-color: #0a0c10;
            }
            QLabel#paneTitle {
                background-color: #14161a;
                color: #00b7ff;
                border: 1px solid #00b7ff;
            }
            QPlainTextEdit {
                background-color: #14161a;
                color: #b0e0ff;
                border: 1px solid #00b7ff;
                font-family: 'JetBrains Mono', 'Courier New', monospace;
                font-size: 11pt;
                selection-background-color: #007acc;
                selection-color: #ffffff;
            }
        )");
    }
    else
    {
        // ❄️ Neon Blue Light Theme with updated font
        setStyleSheet(R"(
            EditorPane {
                background-color: #e6f0ff;
            }
            QLabel#paneTitle {
                background-color: #ffffff;
                color: #00264d;
                border: 1px solid #0099ff;
            }
            QPlainTextEdit {
                background-color: #ffffff;
                color: #00264d;
                border: 1px solid #0099ff;
                font-family: 'JetBrains Mono', 'Courier New', monospace;
                font-size: 11pt;
                selection-background-color: #b3d9ff;
                selection-color: #00264d;
            }
        )");
    }
}
