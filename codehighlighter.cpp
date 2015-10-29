#include "codehighlighter.h"

CodeHighlighter::CodeHighlighter(const QString &type, const bool isHighlightable, QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    highlightable = false;
    multiLineCheck = false;
    blockSeparatorType = 0;

    if (type.compare(QString("sql"),Qt::CaseInsensitive)==0)
    {
        highlightable = isHighlightable;
        SqlHighlighting();
    }
}

void CodeHighlighter::SqlHighlighting()
{
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::blue);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\baction\\b" << "\\badd\\b" << "\\ball\\b" << "\\band\\b"
                    << "\\bbefore\\b" << "\\bbetween\\b" << "\\bbinary\\b" << "\\bbit\\b"
                    << "\\bblob\\b"<< "\\bboth\\b"
                    << "\\bcall\\b" << "\\bcase\\b" << "\\bchange\\b" << "\\bchar\\b"
                    << "\\bcharacter\\b" << "\\bcheck\\b" << "\\bcollate\\b" << "\\bcolumn\\b"
                    << "\\bcondition\\b" << "\\bconnection\\b" << "\\bconstraint\\b" << "\\bcontinue\\b"
                    << "\\bcreate\\b" << "\\bcross\\b" << "\\bcurrent_date\\b"<< "\\bcurrent_time\\b"
                    << "\\bcurrent_timestamp\\b" << "\\bcursor\\b"
                    << "\\bdatabase\\b" << "\\bdatabases\\b" << "\\bdate\\b"<< "\\bdec\\b"
                    << "\\bdecimal\\b" << "\\bdeclare\\b" << "\\bdefault\\b"<< "\\bdelayed\\b"
                    << "\\bdelete\\b" << "\\bdesc\\b" << "\\bdescribe\\b" << "\\bdistinct\\b"
                    << "\\bdiv\\b" << "\\bdouble\\b" << "\\bdrop\\b"
                    << "\\beach\\b" << "\\belse\\b" << "\\belseif\\b"<< "\\benum\\b"
                    << "\\bescaped\\b" << "\\bexists\\b"<< "\\bexit\\b"<< "\\bexplain\\b"
                    << "\\bfalse\\b" << "\\bfetch\\b" << "\\bfloat\\b" << "\\bfloat4\\b"
                    << "\\bfloat8\\b" << "\\bfor\\b" << "\\bforce\\b" << "\\bforeign\\b"
                    << "\\bfrom\\b"
                    << "\\bgrant\\b" << "\\bgroup\\b"
                    << "\\bhaving\\b"
                    << "\\bif\\b" << "\\bignore\\b" << "\\bin\\b"
                    << "\\bindex\\b" << "\\binsert\\b" << "\\bint\\b" << "\\bint1\\b"
                    << "\\bint2\\b" << "\\bint3\\b" << "\\bint4\\b" << "\\bint8\\b"
                    << "\\binteger\\b" << "\\binto\\b" << "\\bis\\b"
                    << "\\bjoin\\b"
                    << "\\bkey\\b" << "\\bkeys\\b" << "\\bkill\\b"
                    << "\\bleading\\b" << "\\bleave\\b"<< "\\bleft\\b"<< "\\blike\\b"
                    << "\\blimit\\b" << "\\blinear\\b" << "\\blines\\b" << "\\bload\\b"
                    << "\\blocaltime\\b" << "\\blocaltimestamp\\b" << "\\block\\b" << "\\blong\\b"
                    << "\\blongblob\\b" << "\\bloop\\b"
                    << "\\bmatch\\b" << "\\bmodifies\\b"
                    << "\\bnatural\\b" << "\\bno\\b" << "\\bnot\\b" << "\\bnull\\b"
                    << "\\bnumeric\\b"
                    << "\\bon\\b" << "\\boption\\b" << "\\boptinally\\b" << "\\bor\\b"
                    << "\\border\\b"<< "\\bout\\b"
                    << "\\bprimary\\b"
                    << "\\brange\\b" << "\\bread\\b" << "\\breads\\b" << "\\breal\\b"
                    << "\\breferences\\b" << "\\brelease\\b" << "\\brename\\b"<< "\\breplace\\b"
                    << "\\brequire\\b" << "\\brestrict\\b"<< "\\breturn\\b"<< "\\brevoke\\b"
                    << "\\bright\\b"
                    << "\\bschema\\b" << "\\bschemas\\b" << "\\bset\\b" << "\\bselect\\b"<< "\\bshow\\b"
                    << "\\bsmallint\\b" << "\\bspecific\\b" << "\\bsql\\b"<< "\\bsqlexception\\b"
                    << "\\btable\\b" << "\\bterminated\\b" << "\\btext\\b" << "\\bthen\\b"
                    << "\\btime\\b" << "\\btimestamp\\b" << "\\btinyblob\\b" << "\\btinyint\\b"
                    << "\\btinytext\\b" << "\\bto\\b" << "\\btrigger\\b" << "\\btrue\\b"
                    << "\\bunion\\b" << "\\bunique\\b" << "\\bunlock\\b" << "\\bunsigned\\b"
                    << "\\bupdate\\b" << "\\busage\\b" << "\\buse\\b"<< "\\busing\\b"
                    << "\\bvalues\\b" << "\\bvarbinary\\b" << "\\bvarchar\\b" << "\\bvarcharacter\\b"
                    << "\\bvarying\\b"
                    << "\\bwhen\\b" << "\\bwhere\\b" << "\\bwhile\\b" << "\\bwith\\b"
                    << "\\bwrite\\b"
                    << "\\bxor\\b";

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern,Qt::CaseInsensitive);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    digitFormat.setForeground(QColor(255,123,0));
    rule.pattern = QRegExp("\\b[0-9]+\\b");
    rule.format = digitFormat;
    highlightingRules.append(rule);

    singleLineCommentFormat.setForeground(QColor(0,150,0));
    rule.pattern = QRegExp("--[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);
}

void CodeHighlighter::highlightBlock(const QString &text)
{
    //matching and calculating { and }
    if (blockSeparatorType == 1)
    {
        QString pureText = text;

        //不算的：注释 引用。进行先行剔除
        QRegExp *singleLineComment = new QRegExp("//[^\n]*");
        QRegExp *quotation = new QRegExp("\"([^\"]|\\\\+\")*\"");
        if (singleLineComment->indexIn(pureText)>=0)
            pureText.remove(singleLineComment->indexIn(pureText),singleLineComment->matchedLength());
        while (quotation->indexIn(pureText)!=-1)
        {
            pureText.remove(quotation->indexIn(pureText),quotation->matchedLength());
        }

        int leftPos = pureText.indexOf('{');
        int leftCount = 0;
        while (leftPos >= 0)
        {
            leftCount++;
            leftPos = pureText.indexOf('(', leftPos + 1);
        }

        int rightPos = pureText.indexOf('}');
        int rightCount = 0;
        while (rightPos >= 0)
        {
            rightCount++;
            rightPos = pureText.indexOf('}', rightPos + 1);
        }

        TextBlockData *data = new TextBlockData;
        data->setParenthesesCount(leftCount-rightCount);
        data->setFolded(false);

        setCurrentBlockUserData(data);
    }

    //highlighting part
    if (!highlightable) return;

    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }

}

