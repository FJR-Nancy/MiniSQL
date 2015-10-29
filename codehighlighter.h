#ifndef CODEHIGHLIGHTER_H
#define CODEHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextBlockUserData>

class CodeHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    CodeHighlighter(const QString &type, const bool isHighlightable, QTextDocument *parent);

protected:
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    void SqlHighlighting();

    bool highlightable;
    bool multiLineCheck;
    int blockSeparatorType;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat digitFormat;
    QTextCharFormat singleLineCommentFormat;
};

class TextBlockData : public QTextBlockUserData
{
public:
    int getParenthesesCount()
    {
        return parenthesesNumber;
    }
    void setParenthesesCount(int number)
    {
        parenthesesNumber = number;
    }
    bool isFolded()
    {
        return foldindStatus;
    }
    void setFolded(bool status)
    {
        foldindStatus = status;
    }

private:
    int parenthesesNumber;
    bool foldindStatus;
};

#endif // CODEHIGHLIGHTER_H
