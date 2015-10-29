#include <QPainter>
#include <QTextBlock>
#include <QMessageBox>
#include "texteditarea.h"
#include "codehighlighter.h"

TextEditArea::TextEditArea(const QString &typeName, const bool isAutoIndentable, const bool isCodeHighlightable, const bool isSideBarVisible, QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);
    codeFoldingArea = new CodeFoldingArea(this);

    //重要:这一行保证了行号的即时更新
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateTextAreaWidth(int)));
    connect(this, SIGNAL(textChanged()), this, SLOT(repaintCodeFoldingArea()));

    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateCodeFoldingArea(QRect,int)));

    QFont textFont = this->font();
    textFont.setPointSize(11);/*point size won't change across moniters*/
    lineNumberFont = textFont;
    this->setFont(textFont);
    type = typeName;

    codeHighlightable = isCodeHighlightable;
    highlighter = new CodeHighlighter(type, codeHighlightable, this->document());

    setLineWrapMode(QPlainTextEdit::NoWrap);
    autoIndentable = isAutoIndentable;
    sideBarVisible = isSideBarVisible;

    updateTextAreaWidth(0);
}

TextEditArea::~TextEditArea()
{
    delete lineNumberArea;
    delete codeFoldingArea;
    //delete type;
    delete highlighter;
}

int TextEditArea::lineNumberAreaWidth()
{
    if (!sideBarVisible) return 0;
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10)
    {
        max /= 10;
        ++digits;
    }

    int space =QFontMetrics(lineNumberFont).width(QLatin1Char('9')) * (digits+1);
    return space;
}

int TextEditArea::codeFoldingAreaWidth()
{
    if (!sideBarVisible) return 0;
    int space =QFontMetrics(lineNumberFont).width(QLatin1Char('9')) * (2);
    return space;
}

void TextEditArea::fontUpdate(const QFont &font)
{
    this->setFont(font);
    lineNumberFont.setPointSize(font.pointSize());
}

void TextEditArea::typeUpdate(const QString &typeName)
{
    if (type.compare(typeName)!=0)
    {
        //Because the highlighter has been recognized by its parent, the parent will do the delete automatically.
        highlighter = new CodeHighlighter(typeName, codeHighlightable, this->document());
        type = typeName;
    }
}

void TextEditArea::updateTextAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth()+codeFoldingAreaWidth(), 0, 0, 0);
}

void TextEditArea::repaintCodeFoldingArea()
{
    codeFoldingArea->update();
}

void TextEditArea::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateTextAreaWidth(0);
}

void TextEditArea::updateCodeFoldingArea(const QRect &rect, int dy)
{
    if (dy)
        codeFoldingArea->scroll(0, dy);
    else
        codeFoldingArea->update(lineNumberArea->width(), rect.y(), codeFoldingArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateTextAreaWidth(0);
}

void TextEditArea::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    codeFoldingArea->setGeometry(QRect(cr.left()+lineNumberAreaWidth(), cr.top(), codeFoldingAreaWidth(), cr.height()));
}

void TextEditArea::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    //[extraAreaPaintEvent_0]
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);
    painter.setFont(lineNumberFont);

    //[extraAreaPaintEvent_1]
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    //[extraAreaPaintEvent_2]
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), (int) blockBoundingRect(block).height(),
                             Qt::AlignTop | Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void TextEditArea::codeFoldingAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(codeFoldingArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    if (type != "cpp" && type != "h") return;

    QTextBlock block = firstVisibleBlock();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();
    int width = codeFoldingArea->width();
    int height = (int) blockBoundingRect(block).height();

    TextBlockData *data;

    while (block.isValid() && top <= event->rect().bottom())
    {

        //关于记录括号信息，要记住你只需要知道这行最外层的括号匹配到下面的哪一行。
        //因此只需要记录多出来多少左括号或者右括号
        //在点击事件发生的时候再进行具体计算

        data = static_cast<TextBlockData *>(block.userData());
        if (block.isVisible() && bottom >= event->rect().top() && data->getParenthesesCount()>0) {
            painter.setPen(Qt::black);
            int size = height*4/7;

            QRect box((width-size)/2, top+(height-size)/2, size, size);
            painter.fillRect(box, Qt::white);
            painter.drawRect(box);
            painter.drawLine((width-size)/2, top+height/2, (width+size)/2, top+height/2);
            if (data->isFolded())
                painter.drawLine(width/2, top+(height-size)/2, width/2, top+(height+size)/2);

        }

        block = block.next();
        top = bottom;
        height = (int) blockBoundingRect(block).height();
        bottom = top + height;
    }
}

void TextEditArea::codeFoldingAreaMousePressEvent(QMouseEvent *event)
{
    if (type != "cpp" && type != "h") return;

    int pos = event->y();

    QTextBlock block = firstVisibleBlock();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();
    while (bottom < pos)
    {
        block = block.next();
        if (block.isVisible()) bottom += (int) blockBoundingRect(block).height();
    }

    TextBlockData *data = static_cast<TextBlockData *>(block.userData());
    if (data->getParenthesesCount()>0)
    {
        int count = data->getParenthesesCount();
        if (data->isFolded())
        {
            data->setFolded(false);
            block = block.next();
            int innerFoldingCount;
            while (block.isValid())
            {
                data = static_cast<TextBlockData *>(block.userData());

                if (data->isFolded()) //TATICS: NOT TO EXPAND PREVIOUS FOLDED BLOCK.
                {
                    block.setVisible(true);
                    innerFoldingCount = data->getParenthesesCount();
                    count += data->getParenthesesCount();
                    block = block.next();
                    while (block.isValid())
                    {
                        data = static_cast<TextBlockData *>(block.userData());
                        innerFoldingCount += data->getParenthesesCount();
                        if (innerFoldingCount<=0) break;
                        count += data->getParenthesesCount();;
                        block = block.next();
                    }
                }

                count += data->getParenthesesCount();
                if (count<=0) break;
                block.setVisible(true);
                block = block.next();
            }
        }
        else
        {
            data->setFolded(true);
            block = block.next();
            while (block.isValid())
            {
                data = static_cast<TextBlockData *>(block.userData());
                count += data->getParenthesesCount();
                if (count<=0) break;
                block.setVisible(false);
                block = block.next();
            }
        }
        update();
        viewport()->update();
    }
}

void TextEditArea::autoAdjustIndent()
{
    if (type != "cpp" && type != "h")
    {
        QMessageBox::critical(this, tr("Error"), tr("Sorry, this function is not supported on this type of file. Please work on *.cpp or *.h"));
        return;// error message
    }
    QTextBlock block = firstVisibleBlock();
    QString text = QString();
    int count = 0;

    while (block.isValid())
    {
        TextBlockData *data = static_cast<TextBlockData *>(block.userData());
        int shift = data->getParenthesesCount();
        if (shift<0) count += shift;

        QString blockText = block.text();
        QRegExp expression = QRegExp("^\\s*");
        blockText.remove(expression);
        blockText = blockText.rightJustified(blockText.length() + (count << 2), ' ');

        if (shift>0) count += shift;
        text.append(blockText);
        text.append('\n');
        block = block.next();
    }

    setPlainText(text);
    document()->setModified(true);
}

void TextEditArea::addIndent()
{
    //adding Indent according to previous line.

    QTextBlock block = textCursor().block();
    block = block.previous();
    if (block.isValid())
    {
        QRegExp expression("^\\s*");
        QString text = QString();
        if (expression.indexIn(block.text())>=0)
        {
            text.append(expression.cap());
        }
        if (type == "cpp" || type == "h")//以防不是cpp没有data怎么办
        {
            TextBlockData *data = static_cast<TextBlockData *>(block.userData());
            if (data->getParenthesesCount()>0)
            {
                text.append("    ");
            }
        }
        insertPlainText(text);
    }
}

void TextEditArea::setAutoIndentable(const bool isAutoIndentable)
{
    autoIndentable = isAutoIndentable;
}

void TextEditArea::keyPressEvent(QKeyEvent *event)
{
    //implement key press event from the base class first, or common function of key pressing won't work.
    QPlainTextEdit::keyPressEvent(event);
    if (autoIndentable && (event->key()==Qt::Key_Enter || event->key()==Qt::Key_Return))
    {
        addIndent();
    }
}

void TextEditArea::setCodeHighlightable(bool isHighlightable)
{
    if (codeHighlightable==isHighlightable) return;
    codeHighlightable = isHighlightable;

    highlighter = new CodeHighlighter(type, codeHighlightable, this->document());
}

void TextEditArea::setSideBarVisible(bool isVisible)
{
    if (sideBarVisible==isVisible) return;
    sideBarVisible = isVisible;
    update();
    updateTextAreaWidth(0);
}
