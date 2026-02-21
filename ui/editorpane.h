#ifndef EDITORPANE_H
#define EDITORPANE_H

#include <QWidget>
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

    void applyTheme(bool darkMode);   // UPDATED

private:
    void setupUI();

    std::unique_ptr<QPlainTextEdit> codeEditor;
    bool m_darkMode = true;
};

#endif // EDITORPANE_H
