#include "replacedialog.h"
#include "ui_replacedialog.h"
#include "texteditarea.h"
#include <QMessageBox>

ReplaceDialog::ReplaceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReplaceDialog)
{
    ui->setupUi(this);
    setWindowTitle("Replace");
    ui->downRadioButton->setChecked(true);
}

ReplaceDialog::~ReplaceDialog()
{
    delete ui;
}

void ReplaceDialog::setup(TextEditArea *editArea)
{
    edit = editArea;
}

void ReplaceDialog::on_findPushButton_clicked()
{
    QString strFind = ui->findLineEdit->text();
    if (strFind.isEmpty())
        QMessageBox::information(this, tr("Empty Find Field"),
                                 "The find field is empty. Please enter a word.");
    else
    {
        QTextDocument::FindFlags flag = 0;
        if (ui->upRadioButton->isChecked()) flag = flag | QTextDocument::FindBackward;
        if (ui->caseSensitiveBox->isChecked()) flag = flag | QTextDocument::FindCaseSensitively;
        if (ui->wholeWordBox->isChecked()) flag = flag | QTextDocument::FindWholeWords;

        if(!edit->find(strFind, flag))
        {
            QMessageBox::information(this,tr("Words Not Find"),
                                    tr("Sorry, cannot find \"%1\"").arg(strFind));
        }
    }
}

void ReplaceDialog::on_replacePushButton_clicked()
{
    QString strReplace = ui->replaceLineEdit->text();
    if (strReplace.isEmpty())
        QMessageBox::information(this, tr("Empty Replace Field"),
                                 "The replace field is empty. Please enter a word.");
    else
    {
        if (!edit->textCursor().hasSelection())
            on_findPushButton_clicked();
        else
        {
            edit->textCursor().insertText(strReplace);
            on_findPushButton_clicked();
        }
    }
}

void ReplaceDialog::on_replaceAllPushButton_clicked()
{
    QString strFind = ui->findLineEdit->text();
    QString strReplace = ui->replaceLineEdit->text();
    if (strFind.isEmpty())
        QMessageBox::information(this, tr("Empty Find Field"),
                                 "The find field is empty. Please enter a word.");
    else if (strReplace.isEmpty())
        QMessageBox::information(this, tr("Empty Replace Field"),
                                 "The replace field is empty. Please enter a word.");
    else
    {
        int i=0;
        QTextCursor editCursor;
        editCursor.setPosition(0);
        edit->setTextCursor(editCursor);
        ui->downRadioButton->setChecked(true);
        on_findPushButton_clicked();
        while (edit->textCursor().hasSelection()){
            edit->textCursor().insertText(strReplace);
            i++;
            edit->find(strFind);
        }
        QMessageBox::information(this,tr("Replace complete"),
                                     tr("Replaced %1 occurrence(s)").arg(i));

    }
}
