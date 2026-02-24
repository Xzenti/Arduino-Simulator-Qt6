#include "EditorTextEdit.h"
#include "LineNumberArea.h"
#include "../core/ThemeManager.h"
#include <QPainter>
#include <QResizeEvent>
#include <QTextBlock>
#include <QDebug>

EditorTextEdit::EditorTextEdit(QWidget *parent)
    : QPlainTextEdit(parent), lineNumberArea(nullptr), backgroundImage() {
    lineNumberArea = new LineNumberArea(this);

    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &EditorTextEdit::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest,
            this, &EditorTextEdit::updateLineNumberArea);

    // Connect document modification tracking
    connect(document(), &QTextDocument::modificationChanged,
            this, [this](bool changed) {
                lineNumberArea->setDirty(changed);
            });

    updateLineNumberAreaWidth(0);
}

void EditorTextEdit::setBackgroundImage(const QString &imagePath) {
    QPixmap originalImage(imagePath);
    if (!originalImage.isNull()) {
        int newWidth = originalImage.width() * 0.8;
        int newHeight = originalImage.height() * 0.8;
        backgroundImage = originalImage.scaled(newWidth, newHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        qDebug() << "Background image loaded:" << imagePath;
        viewport()->update();
    } else {
        qDebug() << "Failed to load background image:" << imagePath;
    }
}

void EditorTextEdit::initializeWithText(const QString &text) {
    // Temporarily disconnect modification signal to avoid triggering dirty state
    disconnect(document(), &QTextDocument::modificationChanged, nullptr, nullptr);

    setPlainText(text);
    document()->setModified(false);

    // Reconnect modification signal
    connect(document(), &QTextDocument::modificationChanged,
            this, [this](bool changed) {
                lineNumberArea->setDirty(changed);
            });
}

void EditorTextEdit::paintEvent(QPaintEvent *event) {
    // Draw whole-line highlighting for current line BEFORE text (Notepad++ style)
    // This needs to be drawn before parent's paintEvent so text appears on top
    QPainter highlightPainter(viewport());
    QTextBlock currentBlock = textCursor().block();
    if (currentBlock.isValid()) {
        QRectF blockRect = blockBoundingGeometry(currentBlock).translated(contentOffset());
        QRect highlightRect(0, (int)blockRect.y(), viewport()->width(), (int)blockRect.height());

        // Theme-aware highlight color
        QColor highlightColor;
        if (ThemeManager::instance().backgroundColor() == QColor(255, 255, 255)) {
            // Light theme: light blue
            highlightColor = QColor(200, 230, 255, 80);  // Light blue with semi-transparency
        } else {
            // Dark theme: darker blue
            highlightColor = QColor(50, 80, 120, 80);    // Dark blue with semi-transparency
        }

        highlightPainter.fillRect(highlightRect, highlightColor);
    }

    // Call parent paint event to draw text
    QPlainTextEdit::paintEvent(event);

    // Draw background image (after text so it doesn't interfere)
    QPainter painter(viewport());
    if (!backgroundImage.isNull()) {
        painter.setOpacity(0.10);

        int margin = qMax(viewport()->width(), viewport()->height()) * 0.1;
        int availWidth = viewport()->width() - (margin * 2);
        int availHeight = viewport()->height() - (margin * 2);
        int targetWidth = availWidth * 0.4;
        int targetHeight = availHeight * 0.4;

        QPixmap scaledImage = backgroundImage.scaledToWidth(targetWidth, Qt::SmoothTransformation);
        if (scaledImage.height() > targetHeight) {
            scaledImage = backgroundImage.scaledToHeight(targetHeight, Qt::SmoothTransformation);
        }

        int x = (viewport()->width() - scaledImage.width()) / 2;
        int y = (viewport()->height() - scaledImage.height()) / 2;
        painter.setOpacity(0.10);
        painter.drawPixmap(x, y, scaledImage);
    }
}

void EditorTextEdit::resizeEvent(QResizeEvent *event) {
    QPlainTextEdit::resizeEvent(event);

    // Calculate line number area width dynamically
    int digits = 1;
    int maxLines = qMax(1, document()->blockCount());
    int digitsNeeded = QString::number(maxLines).length();
    int fontWidth = fontMetrics().horizontalAdvance("8");
    int lineNumberWidth = fontWidth * digitsNeeded + 4;  // Tight fit

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(cr.left(), cr.top(), lineNumberWidth, cr.height());
}

void EditorTextEdit::updateLineNumberAreaWidth(int) {
    // Calculate line number area width dynamically based on line count
    int digits = 1;
    int maxLines = qMax(1, document()->blockCount());
    int digitsNeeded = QString::number(maxLines).length();
    int fontWidth = fontMetrics().horizontalAdvance("8");
    int lineNumberWidth = fontWidth * digitsNeeded + 4;  // Tight fit with minimal padding

    setViewportMargins(lineNumberWidth, 0, 0, 0);
}

void EditorTextEdit::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
}
