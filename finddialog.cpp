#include "finddialog.h"
#include "ui_finddialog.h"
#include "texteditarea.h"
#include <QMessageBox>

FindDialog::FindDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindDialog)
{
    ui->setupUi(this);
    setWindowTitle("Find");
    ui->downRadioButton->setChecked(true);
}

FindDialog::~FindDialog()
{
    delete ui;
}

void FindDialog::setup(TextEditArea *editArea)
{
    edit = editArea;
}

void FindDialog::on_findPushButton_clicked()
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

