#ifndef GOTODIALOG_H
#define GOTODIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class TextEditArea;
QT_END_NAMESPACE

namespace Ui {
class GotoDialog;
}

class GotoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GotoDialog(QWidget *parent = 0);
    ~GotoDialog();

    void setup(TextEditArea *editArea);

private slots:
    void on_pushButton_clicked();

private:
    Ui::GotoDialog *ui;
    TextEditArea *edit;
};

#endif // GOTODIALOG_H
