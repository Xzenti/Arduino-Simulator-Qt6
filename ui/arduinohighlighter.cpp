#ifndef ARDUINOHIGHLIGHTER_H
#define ARDUINOHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QVector>
#include <QTextCharFormat>

class QTextDocument;

class ArduinoHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit ArduinoHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keywordFormat;
    QTextCharFormat typeFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat charFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat preprocessorFormat;
    QTextCharFormat functionFormat;
};

#endif // ARDUINOHIGHLIGHTER_H
