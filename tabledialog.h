#ifndef TABLEDIALOG_H
#define TABLEDIALOG_H

#include <QDialog>
#include "Main.h"

namespace Ui {
class TableDialog;
}

class TableDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit TableDialog(QWidget *parent = 0);
    ~TableDialog();
    void setup(Data _data, Table _tableInfo, vector<Attribute> _column);
    
private:
    Ui::TableDialog *ui;

    void create_table();
};

#endif // TABLEDIALOG_H
