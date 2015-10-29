#include <QMessageBox>
#include "gotodialog.h"
#include "ui_gotodialog.h"
#include "texteditarea.h"

GotoDialog::GotoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GotoDialog)
{
    ui->setupUi(this);
}

GotoDialog::~GotoDialog()
{
    delete ui;
}

void GotoDialog::setup(TextEditArea *editArea)
{
    edit = editArea;
    ui->label->setText(QString("Goto line (1 - %1):").arg(edit->blockCount()));
}

void GotoDialog::on_pushButton_clicked()
{
    QString  inputText = ui->lineEdit->text();
    if(inputText.isEmpty()) return;
    bool ok = true;
    inputText.toInt(&ok);
    if (!ok)
    {
        QMessageBox::critical(this, tr("Error"), tr("Invalid input."));
        return;// error message
    }
    QTextCursor cursor = edit->textCursor();
    int currentLine = cursor.blockNumber();
    int targetLine = inputText.toInt();
    if (targetLine < 1 || targetLine > edit->blockCount())
    {
        QMessageBox::critical(this, tr("Error"), tr("Invalid input."));
        return;// error message
    }
    if (currentLine<targetLine)
    {
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, targetLine-currentLine-1);
    }
    else
    {
        cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor, currentLine-targetLine+1);
    }
    edit->setTextCursor(cursor);
    this->close();
}
