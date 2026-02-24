#pragma once

#include <QPlainTextEdit>
#include <QPixmap>
#include <QTextBlock>

class LineNumberArea;
class QPaintEvent;
class QResizeEvent;

// Custom QPlainTextEdit with background image support
class EditorTextEdit : public QPlainTextEdit {
    Q_OBJECT
    friend class LineNumberArea;

public:
    explicit EditorTextEdit(QWidget *parent = nullptr);
    void setBackgroundImage(const QString &imagePath);    void initializeWithText(const QString &text);  // Set initial text without triggering dirty
    LineNumberArea *getLineNumberArea() const { return lineNumberArea; }

    // Public accessors for LineNumberArea to use protected QPlainTextEdit methods
    QTextBlock getFirstVisibleBlock() const { return firstVisibleBlock(); }
    QRectF getBlockBoundingGeometry(const QTextBlock &block) const { return blockBoundingGeometry(block); }
    QPointF getContentOffset() const { return contentOffset(); }
    QRectF getBlockBoundingRect(const QTextBlock &block) const { return blockBoundingRect(block); }

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    LineNumberArea *lineNumberArea;
    QPixmap backgroundImage;
};
