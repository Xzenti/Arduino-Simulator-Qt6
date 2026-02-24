#include "LineNumberArea.h"
#include "EditorTextEdit.h"
#include "../core/ThemeManager.h"
#include <QPainter>
#include <QMouseEvent>
#include <QTextBlock>
#include <QDebug>

LineNumberArea::LineNumberArea(QPlainTextEdit *editor)
    : QWidget(editor), codeEditor(editor) {
}

QSize LineNumberArea::sizeHint() const {
    // Calculate width based on number of lines and font metrics (compact layout)
    int digits = 1;
    int max_lines = qMax(1, codeEditor->document()->blockCount());
    int digits_needed = QString::number(max_lines).length();

    // Width = font width per character * number of digits + minimal padding
    int fontWidth = codeEditor->fontMetrics().horizontalAdvance("8");  // Use '8' as it's wide
    int width = fontWidth * digits_needed + 2;  // Tight fit with 2px padding

    return QSize(width, 0);
}

void LineNumberArea::setBreakpoint(int lineNumber, bool set) {
    if (set)
        breakpoints.insert(lineNumber);
    else
        breakpoints.remove(lineNumber);
    update();
}

bool LineNumberArea::hasBreakpoint(int lineNumber) const {
    return breakpoints.contains(lineNumber);
}

void LineNumberArea::clearAllBreakpoints() {
    breakpoints.clear();
    update();
}

void LineNumberArea::setDirty(bool dirty) {
    if (isDirty != dirty) {  // Only update if state changed
        isDirty = dirty;
        update();  // Redraw to show/hide dirty indicator
    }
}

void LineNumberArea::paintEvent(QPaintEvent *event) {
    // Use ThemeManager for background color
    QPainter painter(this);
    painter.fillRect(event->rect(), ThemeManager::instance().backgroundColor() == QColor(255, 255, 255)
                                        ? QColor("#f6f8fa")  // Light mode
                                        : QColor("#161b22")); // Dark mode

    // Draw dirty indicator ribbon on the left edge (like Notepad++)
    if (isDirty) {
        painter.fillRect(0, 0, 4, height(), QColor("#FF6B00"));  // Bright orange ribbon for unsaved (4px wide)
    }

    // Use the editor's public accessors
    EditorTextEdit *editorTextEdit = qobject_cast<EditorTextEdit*>(codeEditor);
    if (!editorTextEdit)
        return;

    QTextBlock block = editorTextEdit->getFirstVisibleBlock();
    int blockNumber = block.blockNumber();
    QRectF blockBoundingGeometry = editorTextEdit->getBlockBoundingGeometry(block);
    QPointF contentOffset = editorTextEdit->getContentOffset();
    int top = blockBoundingGeometry.translated(contentOffset).top();
    int bottom = top + editorTextEdit->getBlockBoundingRect(block).height();

    // Theme-aware line numbers text color
    QFont lineNumberFont = codeEditor->font();
    lineNumberFont.setPointSize(lineNumberFont.pointSize() - 1);  // Slightly smaller than code
    painter.setFont(lineNumberFont);

    // Use theme-aware color
    QColor lineNumberColor = ThemeManager::instance().backgroundColor() == QColor(255, 255, 255)
                                 ? QColor("#6a737d")  // Light mode gray
                                 : QColor("#8b949e"); // Dark mode light gray
    painter.setPen(lineNumberColor);

    // No right padding for tight layout (dirty indicator on left edge takes 2px)
    const int rightPadding = 1;

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString lineNum = QString::number(blockNumber + 1);

            // Draw line number right-aligned with tight spacing
            painter.drawText(0, top, width() - rightPadding,
                             codeEditor->fontMetrics().height(),
                             Qt::AlignRight | Qt::AlignVCenter, lineNum);

            // Breakpoints - better positioning with square indicator
            if (breakpoints.contains(blockNumber)) {
                int diameter = codeEditor->fontMetrics().height() - 6;
                int x = 2;  // Left-aligned with small margin
                int y = top + (codeEditor->fontMetrics().height() - diameter) / 2;
                painter.fillRect(x, y, diameter, diameter, QColor("#d73a49"));
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + editorTextEdit->getBlockBoundingRect(block).height();
        ++blockNumber;
    }

    // Draw right border divider
    painter.setPen(QColor("#e1e4e8"));
    painter.drawLine(width() - 1, event->rect().top(), width() - 1, event->rect().bottom());
}

void LineNumberArea::mousePressEvent(QMouseEvent *event) {
    EditorTextEdit *editorTextEdit = qobject_cast<EditorTextEdit*>(codeEditor);
    if (!editorTextEdit)
        return;

    QTextBlock block = editorTextEdit->getFirstVisibleBlock();
    int blockNumber = block.blockNumber();
    QRectF blockBoundingGeometry = editorTextEdit->getBlockBoundingGeometry(block);
    QPointF contentOffset = editorTextEdit->getContentOffset();
    int top = blockBoundingGeometry.translated(contentOffset).top();
    int bottom = top + editorTextEdit->getBlockBoundingRect(block).height();

    while (block.isValid()) {
        if (top <= event->position().y() && event->position().y() < bottom) {
            bool set = !breakpoints.contains(blockNumber);
            setBreakpoint(blockNumber, set);
            emit breakpointToggled(blockNumber, set);
            break;
        }

        block = block.next();
        top = bottom;
        bottom = top + editorTextEdit->getBlockBoundingRect(block).height();
        ++blockNumber;
    }
}

int LineNumberArea::lineNumberAreaWidth() const {
    return 50;
}

int LineNumberArea::breakpointMarginWidth() const {
    return 16;
}
