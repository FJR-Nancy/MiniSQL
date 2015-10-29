#ifndef TEXTEDITAREA_H
#define TEXTEDITAREA_H

#include <QPlainTextEdit>
#include <QAbstractButton>

QT_BEGIN_NAMESPACE
class CodeHighlighter;
QT_END_NAMESPACE

class TextEditArea : public QPlainTextEdit
{
    Q_OBJECT

public:
    TextEditArea(const QString &typeName, const bool isAutoIndentable, const bool isCodeHighlightable, const bool isSideBarVisible,
                 QWidget *parent = 0);
    ~TextEditArea();

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    void codeFoldingAreaPaintEvent(QPaintEvent *event);
    void codeFoldingAreaMousePressEvent(QMouseEvent *event);
    int lineNumberAreaWidth();
    int codeFoldingAreaWidth();
    void fontUpdate(const QFont &font);
    void typeUpdate(const QString &typeName);
    void autoAdjustIndent();
    void setAutoIndentable(const bool isAutoIndentable);
    void setCodeHighlightable(bool isHighlightable);
    void setSideBarVisible(bool isVisible);

protected:
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *event);

private slots:
    void updateTextAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &, int);
    void repaintCodeFoldingArea();
    void updateCodeFoldingArea(const QRect &, int);

private:
    QWidget *lineNumberArea;
    QWidget *codeFoldingArea;
    QFont lineNumberFont;
    QString type;
    CodeHighlighter *highlighter;

    bool autoIndentable;
    bool sideBarVisible;
    void addIndent();

    bool codeHighlightable;
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(TextEditArea *edit) : QWidget(edit)
    {
        textEdit = edit;
    }

    QSize sizeHint() const
    {
        return QSize(textEdit->lineNumberAreaWidth(),0);
    }

protected:
    void paintEvent(QPaintEvent *event)
    {
        textEdit->lineNumberAreaPaintEvent(event);
    }

private:
    TextEditArea *textEdit;
};

class CodeFoldingArea : public QWidget
{
public:
    CodeFoldingArea(TextEditArea *edit) : QWidget(edit)
    {
        textEdit = edit;
    }

    QSize sizeHint() const
    {
        return QSize(textEdit->codeFoldingAreaWidth(),0);
    }

protected:
    void paintEvent(QPaintEvent *event)
    {
        textEdit->codeFoldingAreaPaintEvent(event);
    }
    void mousePressEvent(QMouseEvent *event)
    {
        textEdit->codeFoldingAreaMousePressEvent(event);
    }

private:
    TextEditArea *textEdit;
};

#endif // TEXTEDITAREA_H
