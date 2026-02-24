#pragma once

#include <QWidget>
#include <QSet>

class QPlainTextEdit;
class QMouseEvent;
class QPaintEvent;

// Line number area widget with breakpoint margin
class LineNumberArea : public QWidget {
    Q_OBJECT
public:
    explicit LineNumberArea(QPlainTextEdit *editor);

    QSize sizeHint() const override;

    // Breakpoint management
    void setBreakpoint(int lineNumber, bool set);
    bool hasBreakpoint(int lineNumber) const;
    void clearAllBreakpoints();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void breakpointToggled(int lineNumber, bool set);

private:
    QPlainTextEdit *codeEditor;
    QSet<int> breakpoints;
    bool isDirty = false;  // Track if document has unsaved changes

    int lineNumberAreaWidth() const;
    int breakpointMarginWidth() const;

public slots:
    void setDirty(bool dirty);  // Called when document is modified or saved
};
