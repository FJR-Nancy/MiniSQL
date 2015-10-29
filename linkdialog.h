#ifndef LINKDIALOG_H
#define LINKDIALOG_H

#include <QDialog>

namespace Ui {
class LinkDialog;
}

class LinkDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit LinkDialog(QWidget *parent = 0);
    ~LinkDialog();

    void setup(bool *connectStatus);
    
private slots:
    void on_connectButton_clicked();

private:
    Ui::LinkDialog *ui;
    bool *pstatus;
};

#endif // LINKDIALOG_H
