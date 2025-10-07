#include "markdownhighlighter.h"

MarkdownHighlighter::MarkdownHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    // Überschriften (z. B. # Titel)
    headingFormat.setForeground(Qt::darkBlue);
    headingFormat.setFontWeight(QFont::Bold);
    rules.append({ QRegularExpression("^#{1,6} .+"), headingFormat });

    // Fett: **text**
    boldFormat.setFontWeight(QFont::Bold);
    rules.append({ QRegularExpression(R"(\*\*(.*?)\*\*)"), boldFormat });

    // Kursiv: *text* oder _text_
    italicFormat.setFontItalic(true);
    rules.append({ QRegularExpression(R"(\*(.*?)\*)"), italicFormat });
    rules.append({ QRegularExpression(R"(_(.*?)_)"), italicFormat });

    // Inline-Code: `code`
    codeFormat.setFontFamilies(QStringList("Courier"));
    codeFormat.setBackground(QColor("#f0f0f0"));
    rules.append({ QRegularExpression(R"(`[^`]+`)"), codeFormat });

    // Zitat: > Text
    quoteFormat.setForeground(Qt::darkGreen);
    rules.append({ QRegularExpression(R"(^>.+)"), quoteFormat });

    // Liste: - Text oder * Text oder 1. Text
    listFormat.setForeground(Qt::darkCyan);
    rules.append({ QRegularExpression(R"(^(\s*)([-*+]|\d+\.)\s)"), listFormat });

    // Link: [Text](URL)
    linkFormat.setForeground(Qt::blue);
    linkFormat.setFontUnderline(true);
    rules.append({ QRegularExpression(R"(\[.*?\]\(.*?\))"), linkFormat });

    // Leere Checkbox: - [ ]
    checkboxOpenFormat.setForeground(Qt::darkGreen);
    checkboxOpenFormat.setFontWeight(QFont::Bold);
    rules.append({
        QRegularExpression(R"(^\s*\[\s?\]\s)"),
        checkboxOpenFormat
    });

    // Abgehakte Checkbox: - [x] oder - [X]
    checkboxDoneFormat.setForeground(Qt::darkGreen);
    checkboxDoneFormat.setFontWeight(QFont::Bold);
    checkboxDoneFormat.setFontStrikeOut(true);  // Durchgestrichen
    rules.append({
        QRegularExpression(R"(^\s*\[(x|X)\]\s.+)"),
        checkboxDoneFormat
    });
}

void MarkdownHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : rules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
