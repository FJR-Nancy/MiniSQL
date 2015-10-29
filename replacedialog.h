#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class TextEditArea;
QT_END_NAMESPACE

namespace Ui {
class ReplaceDialog;
}

class ReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReplaceDialog(QWidget *parent = 0);
    ~ReplaceDialog();
    void setup(TextEditArea *editArea);

private slots:
    void on_findPushButton_clicked();
    void on_replacePushButton_clicked();
    void on_replaceAllPushButton_clicked();

private:
    Ui::ReplaceDialog *ui;
    TextEditArea *edit;
};

#endif // REPLACEDIALOG_H
