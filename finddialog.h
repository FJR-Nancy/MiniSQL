#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class TextEditArea;
QT_END_NAMESPACE

namespace Ui {
class FindDialog;
}

class FindDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit FindDialog(QWidget *parent = 0);
    ~FindDialog();

    void setup(TextEditArea *editArea);
    
private slots:
    void on_findPushButton_clicked();

private:
    Ui::FindDialog *ui;
    TextEditArea *edit;
};

#endif // FINDDIALOG_H
