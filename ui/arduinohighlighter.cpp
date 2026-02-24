#include "ArduinoHighlighter.h"
#include <QTextDocument>

ArduinoHighlighter::ArduinoHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent) {

    preprocessorFormat.setForeground(QColor(215, 58, 73));
    HighlightingRule preprocessorRule;
    preprocessorRule.pattern = QRegularExpression("^\\s*#.*");
    preprocessorRule.format = preprocessorFormat;
    highlightingRules.append(preprocessorRule);

    keywordFormat.setForeground(QColor(215, 58, 73));
    keywordFormat.setFontWeight(QFont::Bold);

    QStringList keywords;
    keywords << "\\bvoid\\b" << "\\bint\\b" << "\\bfloat\\b" << "\\bdouble\\b"
             << "\\bbool\\b" << "\\bchar\\b" << "\\breturn\\b" << "\\bif\\b"
             << "\\belse\\b" << "\\bfor\\b" << "\\bwhile\\b" << "\\bclass\\b"
             << "\\bstruct\\b" << "\\bpublic\\b" << "\\bprivate\\b" << "\\bsetup\\b"
             << "\\bloop\\b" << "\\bSerial\\b" << "\\bdelay\\b";

    for (const QString &pattern : keywords) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    commentFormat.setForeground(QColor(106, 115, 125));
    commentFormat.setFontItalic(true);

    HighlightingRule singleLineCommentRule;
    singleLineCommentRule.pattern = QRegularExpression("//[^\n]*");
    singleLineCommentRule.format = commentFormat;
    highlightingRules.append(singleLineCommentRule);

    stringFormat.setForeground(QColor(3, 47, 98));
    HighlightingRule stringRule;
    stringRule.pattern = QRegularExpression("\".*?\"");
    stringRule.format = stringFormat;
    highlightingRules.append(stringRule);

    numberFormat.setForeground(QColor(0, 92, 197));
    HighlightingRule numberRule;
    numberRule.pattern = QRegularExpression("\\b[0-9]+\\b");
    numberRule.format = numberFormat;
    highlightingRules.append(numberRule);
}

void ArduinoHighlighter::highlightBlock(const QString &text) {
    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(0);
}
