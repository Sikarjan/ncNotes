#ifndef MARKDOWNHIGHLIGHTER_H
#define MARKDOWNHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class MarkdownHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit MarkdownHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> rules;

    QTextCharFormat headingFormat;
    QTextCharFormat boldFormat;
    QTextCharFormat italicFormat;
    QTextCharFormat codeFormat;
    QTextCharFormat quoteFormat;
    QTextCharFormat listFormat;
    QTextCharFormat linkFormat;
    QTextCharFormat checkboxOpenFormat;
    QTextCharFormat checkboxDoneFormat;
};

#endif // MARKDOWNHIGHLIGHTER_H
