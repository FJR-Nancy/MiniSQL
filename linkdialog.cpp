#include "linkdialog.h"
#include "ui_linkdialog.h"
#include <QMessageBox>

LinkDialog::LinkDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LinkDialog)
{
    ui->setupUi(this);
    setWindowTitle("Connect to DataBase");
}

LinkDialog::~LinkDialog()
{
    delete ui;
}

void LinkDialog::setup(bool *connectStatus)
{
    pstatus = connectStatus;
}

void LinkDialog::on_connectButton_clicked()
{
    QString strUsername = ui->usernameLineEdit->text();
    QString strPassword = ui->passwordlineEdit->text();

    if (strUsername.isEmpty())
        QMessageBox::information(this, tr("Empty Username Field"), "Please enter the username.");
    else if (strPassword.isEmpty())
        QMessageBox::information(this, tr("Empty Password Fied"), "Please enter the password.");
    else if (strUsername != "root")
        QMessageBox::information(this, tr("Username Error"), "The username was incorrect.");
    else if (strPassword != "root")
        QMessageBox::information(this, tr("Password Error"), "The password was incorrect.");
    else{
        QMessageBox connectBox;
        connectBox.setText("You have connected to Database.");
        QPushButton *okButton = connectBox.addButton(tr("OK"), QMessageBox::ActionRole);
        connectBox.exec();
        if (connectBox.clickedButton() == okButton)
        {*pstatus = true;connectBox.reject();this->reject();}
    }
}
