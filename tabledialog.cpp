#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>

#include "tabledialog.h"
#include "ui_tabledialog.h"

Data dataInfo;
Table tableInfo;
vector<Attribute> column;

TableDialog::TableDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TableDialog)
{
    ui->setupUi(this);
}

TableDialog::~TableDialog()
{
    delete ui;
}

void TableDialog::setup(Data _data, Table _tableInfo, vector<Attribute> _column)
{
    dataInfo = _data;
    tableInfo = _tableInfo;
    column = _column;
    create_table();
}

void TableDialog::create_table()
{
    QStandardItemModel *model = new QStandardItemModel();
    for(int i = 0; i < tableInfo.attriNum; i++)
    {
        QString attriName = QString::fromStdString(tableInfo.attributes[i].name);
        model->setHorizontalHeaderItem(i, new QStandardItem(attriName));
    }
    ui->tableView->setModel(model);
    //设置选中时为整行选中
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    //设置表格的单元为只读属性，即不能编辑
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //如果你用在QTableView中使用右键菜单，需启用该属性
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    for(unsigned int i = 0; i < dataInfo.rows.size(); i++)
    {
        for(int j = 0; j < tableInfo.attriNum; j++)
        {
            QString colValue = QString::fromStdString(dataInfo.rows[i].columns[j]);
            model->setItem(i, j, new QStandardItem(colValue));
        }
    }
}
