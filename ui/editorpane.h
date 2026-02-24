
#pragma once
#include "ArduinoHighlighter.h"
#include "LineNumberArea.h"
#include "EditorTextEdit.h"
#include <QWidget>
#include <QLabel>

#include <QPlainTextEdit>
#include <memory>

class EditorPane : public QWidget {
    Q_OBJECT

public:
    explicit EditorPane(QWidget *parent = nullptr);
    ~EditorPane();

    QString getCode() const;
    void setCode(const QString &code);
    void clear();
    bool loadFromFile(const QString &filePath);
    bool saveToFile(const QString &filePath);
    bool isSaved() const { return m_currentContent == m_savedContent; }
    void markAsSaved() { m_savedContent = codeEditor->toPlainText(); updateUnsavedIndicator(); }

    void applyTheme(bool darkMode);   // UPDATED

private slots:
    void onContentChanged();

private:
    void setupUI();
    void updateUnsavedIndicator();

    std::unique_ptr<QPlainTextEdit> codeEditor;
    std::unique_ptr<ArduinoHighlighter> highlighter;
    QLabel *titleLabel = nullptr;
    QString m_savedContent;
    QString m_currentContent;
    bool m_darkMode = true;

};

