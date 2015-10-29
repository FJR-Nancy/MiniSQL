#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QTextCharFormat>
#include <QFont>
#include <QFontDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QDateTime>
#include <QTimer>
#include <QtWebKitWidgets/QWebView>
#include <QColorDialog>
#include <QLabel>
#include <QDialog>
#include <QTableView>

#include "minisql.h"
#include "ui_minisql.h"
#include "texteditarea.h"
#include "finddialog.h"
#include "replacedialog.h"
#include "gotodialog.h"
#include "linkdialog.h"
#include "tabledialog.h"

#include <stdlib.h>
#include <string>
#include <string.h>

#include "Main.h"
#include "Interpreter.h"
#include "APIManager.h"

Interpreter parsetree;
extern APIManager API;

MiniSQL::MiniSQL(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MiniSQL)
{
    ui->setupUi(this);
    connect(ui->tabWidget,SIGNAL(tabCloseRequested(int)),this,SLOT(removeSubTab(int)));
    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(changeSubTab(int)));

    initialize_statusBar();
    initialize_dialogs();
    autoIndentable = true;
    lineWrappable = false;
    codeHighlightable = true;
    sideBarVisible = true;
    status = false;
    pStatus = &status;
    ui->plainTextEdit->setReadOnly(true);
}

MiniSQL::~MiniSQL()
{
    delete ui;
    delete timer;
    delete statusLabel;
    delete cursorLabel;
    delete timeLabel;
    delete findDialog;
    delete replaceDialog;
    delete gotoDialog;
    delete linkDialog;
    delete tableDialog;
}

void MiniSQL::initialize_dialogs()
{
    findDialog = new FindDialog(this);
    replaceDialog = new ReplaceDialog(this);
    gotoDialog = new GotoDialog(this);
    linkDialog = new LinkDialog(this);
    tableDialog = new TableDialog(this);
}

void MiniSQL::removeSubTab(int index)
{
    if (maybeSave()) ui->tabWidget->removeTab(index);
}

void MiniSQL::changeSubTab(int index)
{
    if (index<0) return;
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    edit->setAutoIndentable(autoIndentable);
    edit->setCodeHighlightable(codeHighlightable);
    edit->setSideBarVisible(sideBarVisible);

    if (lineWrappable)
        edit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    else
        edit->setLineWrapMode(QPlainTextEdit::NoWrap);
}

void MiniSQL::setCurrentFile(const QString &fileName)
{
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    if (!edit)
    {
        QMessageBox::critical(this, tr("Error"), tr("You are not working in any file."));
        return;// error message
    }
    edit->setDocumentTitle(fileName);
}

QString MiniSQL::getCurrentFile()
{
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    if (!edit)
    {
        QMessageBox::critical(this, tr("Error"), tr("You are not working in any file."));
        return QString();// error message
    }
    QString fileName=edit->documentTitle();
    return fileName;
}

void MiniSQL::on_actionNew_triggered()
{
    if (!status)
        QMessageBox::information(this, tr("Error"), tr("Please connect to DataBase first."));
    else{
        TextEditArea *edit=new TextEditArea(QString("NaF"), autoIndentable, codeHighlightable, sideBarVisible);
        setUpdatesEnabled(false); /*To avoid screen flicker*/
        ui->tabWidget->addTab(edit,tr("Noname"));
        ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
        setUpdatesEnabled(true);
        setCurrentFile(QString());

        connect(edit,SIGNAL(cursorPositionChanged()),this,SLOT(cursorChanged()));
    }
}

void MiniSQL::on_actionOpen_triggered()
{
    if (!status)
        QMessageBox::information(this, tr("Error"), tr("Please connect to DataBase first."));
    else{
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(),
                           tr("All Files (*.txt *.sql);;Text Files (*.txt);;SQL Files (*.sql)"));

        if (!fileName.isEmpty())
        {
            QFile file(fileName);
            if (!file.open(QIODevice::ReadOnly))
            {
                QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
                return;
            }

            QTextStream in(&file);
            TextEditArea *edit=new TextEditArea(fileName.section('.',-1,-1), autoIndentable, codeHighlightable, sideBarVisible);
            setUpdatesEnabled(false); /*To avoid screen flicker*/
            ui->tabWidget->addTab(edit,QFileInfo(fileName).fileName());
            ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
            setUpdatesEnabled(true);

            edit->setPlainText(in.readAll()); /*must read-in after setting format*/
            setCurrentFile(fileName); /*must put after edit is set*/

            file.close();
            connect(edit,SIGNAL(cursorPositionChanged()),this,SLOT(cursorChanged()));
        }
    }
}

void MiniSQL::on_actionSave_triggered()
{
    QString fileName=getCurrentFile();

    if (!fileName.isEmpty())
    {
         QFile file(fileName);
         if (!file.open(QIODevice::WriteOnly))
         {
             QMessageBox::critical(this, tr("Error"), tr("Could not save file"));
             return;// error message
         }
         else
         {
             QTextStream stream(&file);
             TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
             if (!edit)
             {
                 QMessageBox::critical(this, tr("Error"), tr("You are not working in any file."));
                 return;// error message
             }
             stream << edit->toPlainText();
             stream.flush();
             ui->statusBar->showMessage(tr("File saved."),3000);
             file.close();
             edit->document()->setModified(false);
         }
    }
    else if(fileName == "Noname") on_actionSave_As_triggered();
    else return;
}

void MiniSQL::on_actionSave_As_triggered() //注意新文件被保存成不同类型后要增加高亮等等
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QString(),
         tr("All Files (*.txt *.sql);;Text File (*.txt);;SQL File (*.sql)"));

    if (!fileName.isEmpty())
    {
         QFile file(fileName);
         if (!file.open(QIODevice::WriteOnly))
         {
             QMessageBox::critical(this, tr("Error"), tr("Could not save file"));
             return;// error message
         }
         else
         {
             TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
             if (!edit)
             {
                 QMessageBox::critical(this, tr("Error"), tr("You are not working in any file."));
                 return;// error message
             }

             setCurrentFile(fileName);

             QTextStream stream(&file);
             stream << edit->toPlainText();
             stream.flush();
             file.close();
             ui->tabWidget->setTabText(ui->tabWidget->currentIndex(),QFileInfo(fileName).fileName());
             ui->statusBar->showMessage(tr("File saved."),3000);

             edit->typeUpdate(fileName.section('.',-1,-1));

             edit->document()->setModified(false);
         }
    }
}

void MiniSQL::on_actionQuit_triggered()
{
    if (maybeSave()) qApp->quit();
}

void MiniSQL::on_actionUndo_triggered()
{
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    if (!edit)
    {
        QMessageBox::critical(this, tr("Error"), tr("You are not working in any file."));
        return;// error message
    }
    edit->undo();
}

void MiniSQL::on_actionRedo_triggered()
{
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    if (!edit)
    {
        QMessageBox::critical(this, tr("Error"), tr("You are not working in any file."));
        return;// error message
    }
    edit->redo();
}

void MiniSQL::on_actionCopy_triggered()
{
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    if (!edit)
    {
        QMessageBox::critical(this, tr("Error"), tr("You are not working in any file."));
        return;// error message
    }
    edit->copy();
}

void MiniSQL::on_actionCut_triggered()
{
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    if (!edit)
    {
        QMessageBox::critical(this, tr("Error"), tr("You are not working in any file."));
        return;// error message
    }
    edit->cut();
}

void MiniSQL::on_actionPaste_triggered()
{
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    if (!edit)
    {
        QMessageBox::critical(this, tr("Error"), tr("You are not working in any file."));
        return;// error message
    }
    edit->paste();
}

void MiniSQL::on_actionFont_triggered()
{
    bool ok;
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    QFont newfont=QFontDialog::getFont(&ok, edit->currentCharFormat().font(), this, tr("Font Option"));
    if (!ok)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No change has been done."));
        return;// error message
    }
    edit->fontUpdate(newfont);
}

void MiniSQL::on_actionFind_triggered()
{
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    if (!edit)
    {
        QMessageBox::critical(this, tr("Error"), tr("You are not working in any file."));
        return;// error message
    }
    findDialog->setup(edit);
    findDialog->show();
}

void MiniSQL::on_actionReplace_triggered()
{
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    if (!edit)
    {
        QMessageBox::critical(this, tr("Error"), tr("You are not working in any file."));
        return;// error message
    }
    replaceDialog->setup(edit);
    replaceDialog->show();
}

void MiniSQL::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

bool MiniSQL::maybeSave()
{
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    if (edit && edit->document()->isModified())
    {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Application"),
                                   tr("The document has been modified.\n"
                                      "Do you want to save your changes?"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
        {
            QString fileName=getCurrentFile();
            if (!fileName.isEmpty())
                on_actionSave_triggered();
            else
                on_actionSave_As_triggered();
            return true;
        }
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void MiniSQL::initialize_statusBar()
{
    QStatusBar *bar = ui->statusBar;
    statusLabel = new QLabel;
    statusLabel->setFrameShape(QFrame::WinPanel);
    statusLabel->setFrameShadow(QFrame::Sunken);
    statusLabel->setAlignment(Qt::AlignHCenter);
    cursorLabel = new QLabel;
    cursorLabel->setFrameShape(QFrame::WinPanel);
    cursorLabel->setFrameShadow(QFrame::Sunken);
    cursorLabel->setAlignment(Qt::AlignHCenter);
    timeLabel = new QLabel;
    timeLabel->setFrameShape(QFrame::WinPanel);
    timeLabel->setFrameShadow(QFrame::Sunken);
    timeLabel->setAlignment(Qt::AlignHCenter);
    bar->addWidget(statusLabel,4);
    bar->addWidget(cursorLabel,3);
    bar->addWidget(timeLabel,3);
    statusLabel->setText(tr("Welcome to use our MiniSQL!"));

    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(timerUpDate()));
    timer->start(1000);
}

void MiniSQL::cursorChanged()
{
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    if (edit)
    {
        const QTextCursor cursor = edit->textCursor();
        int colNum = cursor.columnNumber();
        int rowNum = cursor.blockNumber()+1;
        cursorLabel->setText(tr("Row  %1    Column  %2").arg(rowNum).arg(colNum));
    }
}

void MiniSQL::timerUpDate()
{
    QDateTime time = QDateTime::currentDateTime();
    QString strTime = time.toString("yyyy-MM-dd hh:mm:ss dddd");
    timeLabel->setText(strTime);
}

void MiniSQL::on_actionAbout_triggered()
{
    QMessageBox::information(this, tr("About"), tr("Thanks for using our MiniSQL!\n")+
                                                tr("Please Contact us for any questions.\n")+
                                                tr("Mail: hfanglei@hotmail.com"));
}

void MiniSQL::on_actionFont_Color_triggered()
{
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    if (!edit)
    {
        QMessageBox::critical(this, tr("Error"), tr("You are not working in any file."));
        return;// error message
    }
    QColor col = QColorDialog::getColor(Qt::black, this);
    if (!col.isValid())
        return;
    QTextCharFormat format=edit->currentCharFormat();
    format.setForeground(col);
    if (edit->textCursor().hasSelection())
    {
        edit->setCurrentCharFormat(format);
    }
    else
    {
        QTextCursor cursor = edit->textCursor();
        edit->selectAll();
        edit->setCurrentCharFormat(format);
        edit->setTextCursor(cursor);
    }
}

void MiniSQL::on_actionLine_Wrapping_triggered()
{
    lineWrappable = !lineWrappable;
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    if (!edit) return;

    if (lineWrappable)
        edit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    else
        edit->setLineWrapMode(QPlainTextEdit::NoWrap);
}

void MiniSQL::on_actionAuto_Adjust_triggered()
{
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    if (!edit)
    {
        QMessageBox::critical(this, tr("Error"), tr("You are not working in any file."));
        return;// error message
    }

    edit->autoAdjustIndent();
}

void MiniSQL::on_actionAuto_Indent_triggered()
{
    autoIndentable = !autoIndentable;
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    if (!edit) return;
    edit->setAutoIndentable(autoIndentable);
}

void MiniSQL::on_actionGoto_triggered()
{
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    if (!edit)
    {
        QMessageBox::critical(this, tr("Error"), tr("You are not working in any file."));
        return;// error message
    }
    gotoDialog->setup(edit);
    gotoDialog->show();
}

void MiniSQL::on_actionBackground_Color_triggered()
{
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    if (!edit)
    {
        QMessageBox::critical(this, tr("Error"), tr("You are not working in any file."));
        return;// error message
    }
    QColor col = QColorDialog::getColor(Qt::black, this);
    if (!col.isValid())
        return;

    QPalette p = edit->palette();

    p.setColor(QPalette::Active, QPalette::Base, col);
    p.setColor(QPalette::Inactive, QPalette::Base, col);

    edit->setPalette(p);
}

void MiniSQL::on_actionFont_Highlight_triggered()
{
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    if (!edit)
    {
        QMessageBox::critical(this, tr("Error"), tr("You are not working in any file."));
        return;// error message
    }
    QColor col = QColorDialog::getColor(Qt::black, this);
    if (!col.isValid())
        return;
    QTextCharFormat format=edit->currentCharFormat();
    format.setBackground(col);
    edit->setCurrentCharFormat(format);
}


void MiniSQL::on_actionInsert_From_File_triggered()
{
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    if (!edit)
    {
        QMessageBox::critical(this, tr("Error"), tr("You are not working in any file."));
        return;// error message
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(),
                       tr("All Files (*.txt *.sql);;Text Files (*.txt);;SQL Files (*.sql)"));

    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }

        QTextStream in(&file);
        edit->insertPlainText(in.readAll());
    }
}

void MiniSQL::on_actionTool_Bar_triggered()
{
    if (ui->mainToolBar->isVisible())
        ui->mainToolBar->hide();
    else
        ui->mainToolBar->show();
}

void MiniSQL::on_actionStatus_Bar_triggered()
{
    if (ui->statusBar->isVisible())
        ui->statusBar->hide();
    else
        ui->statusBar->show();
}

void MiniSQL::on_actionCode_Highlighting_triggered()
{
    codeHighlightable = !codeHighlightable;
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    if (!edit) return;

    edit->setCodeHighlightable(codeHighlightable);
}

void MiniSQL::on_actionSide_Bar_triggered()
{
    sideBarVisible = !sideBarVisible;
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    if (!edit) return;

    edit->setSideBarVisible(sideBarVisible);
}

void MiniSQL::on_actionLink_triggered()
{
    linkDialog->setup(pStatus);
    linkDialog->show();
}

void MiniSQL::on_actionRun_triggered()
{
    TextEditArea *edit=qobject_cast<TextEditArea *>(ui->tabWidget->currentWidget());
    if (!edit)
    {
        QMessageBox::critical(this, tr("Error"), tr("You are not working in any file."));
        return;// error message
    }

        char command[400] = "";
        char input[200] = "";

        QString get = edit->toPlainText();
        string str = get.toStdString();
        const char* ch = str.c_str();
        unsigned int size = get.size();
        unsigned int i, j, k;
        i = 0;

        while(i<size)
        {
            strcpy_s(command, "");//command清零
            strcpy_s(input, "");//input清零
            for(j=i,k=0;j<size;j++,k++)
            {
                if(get[i]!=';')
                {
                    input[k]=ch[i];
                    i++;
                }
                else
                {
                    input[k]=' ';
                    input[k+1]='\0';
                    strcat_s(command, input);
                    i++;
                    break;
                }
            }
            parsetree.Parse(command);
            Execute();
            //string ch2str(command);
            //QString str2qstr = QString::fromStdString(ch2str);
            //ui->plainTextEdit->insertPlainText("test:\n"+str2qstr+"\n");
        }

}

void MiniSQL::ShowResult(Data data, Table tableInfo, vector<Attribute> column)
{
    if(column[0].name == "*"){
        tableDialog->setup(data, tableInfo, column);
        tableDialog->show();
    }
    else{
        for(unsigned int i = 0; i < column.size(); i++){
            unsigned int col;
            for(col = 0; col < tableInfo.attributes.size(); col++){
                if(tableInfo.attributes[col].name == column[i].name) break;
            }
            QString str2qstr = QString::fromStdString(tableInfo.attributes[col].name);
            ui->plainTextEdit->insertPlainText(str2qstr);
            QString str2qstr_2 = QString::fromStdString(tableInfo.attributes[col].name);
            ui->plainTextEdit->insertPlainText(str2qstr_2);
            //cout << tableInfo.attributes[col].name;
            int lengthLeft = tableInfo.attributes[col].length - tableInfo.attributes[col].name.length();
            for(int j = 0; j < lengthLeft; j++){
                ui->plainTextEdit->insertPlainText(" ");
                //cout << ' ';
            }
            ui->plainTextEdit->insertPlainText("| ");
            //cout << "| ";
        }
        ui->plainTextEdit->insertPlainText("\n");
        //cout << endl;
        ui->plainTextEdit->insertPlainText("+");
        //cout << "+";
        for(unsigned int i = 0; i < column.size(); i++){
            unsigned int col;
            for(col = 0; col < tableInfo.attributes.size(); col++){
                if(tableInfo.attributes[col].name == column[i].name) break;
            }
            for(int j = 0; j < tableInfo.attributes[col].length + 1; j++){
                ui->plainTextEdit->insertPlainText("-");
                //cout << "-";
            }
            ui->plainTextEdit->insertPlainText("+");
            //cout << "+";
        }
        ui->plainTextEdit->insertPlainText("\n");
        //cout << endl;

        //内容
        for(unsigned int i = 0; i < data.rows.size(); i++){
            ui->plainTextEdit->insertPlainText("| ");
            //cout << "| ";
            for(unsigned int j = 0; j < column.size(); j++){
                unsigned int col;
                for(col = 0; col < tableInfo.attributes.size(); col++){
                    if(tableInfo.attributes[col].name == column[j].name) break;
                }
                int lengthLeft = tableInfo.attributes[col].length;
                for(unsigned int k =0; k < data.rows[i].columns[col].length(); k++){
                    if(data.rows[i].columns[col].c_str()[k] == EMPTY) break;
                    else{
                        //string ch2str(data.rows[i].columns[col].c_str()[k]);
                        //QString str2qstr = QString::fromStdString(ch2str);
                        //ui->plainTextEdit->insertPlainText(str2qstr);
                        //cout << data.rows[i].columns[col].c_str()[k];
                        lengthLeft--;
                    }
                }
                for(int k = 0; k < lengthLeft; k++) {
                    ui->plainTextEdit->insertPlainText(" ");
                    //cout << " ";
                }
                ui->plainTextEdit->insertPlainText("| ");
                //cout << "| ";
            }
            ui->plainTextEdit->insertPlainText("\n");
            //cout << endl;
        }
    }
    //string ch2str;
    //itoa(data.rows.size(), ch2str, 10);
    //QString str2qstr_3 = QString::fromStdString(ch2str);
    //ui->plainTextEdit->insertPlainText(str2qstr_3+" rows have been found.\n");
    //cout << data.rows.size() << " rows have been found."<< endl;
}

void MiniSQL::Execute()
{
    Index indexInfo;
    int rowCount=0;
    Data data;
    QString str2qstr;
    switch(parsetree.m_operation)
    {
    case CRETAB:
        parsetree.getTableInfo.attriNum=parsetree.getTableInfo.attributes.size();
        API.createTable(parsetree.getTableInfo);
        str2qstr = QString::fromStdString(parsetree.getTableInfo.name);
        ui->plainTextEdit->insertPlainText("Table "+str2qstr+" has been created successfully.\n");
        //cout<<"Table "<<parsetree.getTableInfo.name<<" has been created successfully."<<endl;
        break;
    case TABLEEXISTED:
        ui->plainTextEdit->insertPlainText("The table has been created, please check the database.\n");
        //cout<<"The table has been created, please check the database."<<endl;
        break;
    case DRPTAB:
        API.dropTable(parsetree.getTableInfo);
        str2qstr = QString::fromStdString(parsetree.getTableInfo.name);
        ui->plainTextEdit->insertPlainText("Table "+str2qstr+" has been dropped successfully.\n");
        //cout<<"Table "<<parsetree.getTableInfo.name<<" has been dropped successfully."<<endl;
        break;
    case INSERT:
        /*
        if(parsetree.PrimaryKeyPosition==-1 && parsetree.UniquePostion==-1){
            API.insertValue(parsetree.getTableInfo, parsetree.row);
            ui->plainTextEdit->insertPlainText("One record has been inserted successfully.\n");
            //cout<<"One record has been inserted successfully."<<endl;
            break;
        }
        if(parsetree.PrimaryKeyPosition!=-1)
        {
            data=API.select(parsetree.getTableInfo, parsetree.condition);
            if(data.rows.size()>0){
                ui->plainTextEdit->insertPlainText("Primary Key Redundancy occurs, thus insertion failed.\n");
                //cout<<"Primary Key Redundancy occurs, thus insertion failed."<<endl;
                break;
            }
        }
        if(parsetree.UniquePostion!=-1){

            data=API.select(parsetree.getTableInfo, parsetree.UniqueCondition);
            if(data.rows.size()>0){
                ui->plainTextEdit->insertPlainText("Unique Value Redundancy occurs, thus insertion failed.\n");
                //cout<<"Unique Value Redundancy occurs, thus insertion failed."<<endl;
                break;
            }
        }
        */
        API.insertValue(parsetree.getTableInfo,parsetree.row);
        ui->plainTextEdit->insertPlainText("One record has been inserted successfully.\n");
        //cout<<"One record has been inserted successfully."<<endl;
        break;
    case INSERTERR:
        ui->plainTextEdit->insertPlainText("Incorrect usage of \"insert\" query! Please check your input!\n");
        //cout << "Incorrect usage of \"insert\" query! Please check your input!" << endl;
        break;
    case SELECT_NOWHERE_CAULSE:
        data=API.select(parsetree.getTableInfo);
        if(data.rows.size()!=0)
            ShowResult( data, parsetree.getTableInfo, parsetree.column);
        else{
            ui->plainTextEdit->insertPlainText("No data is found!\n");
            //cout << "No data is found!" << endl;
        }
        break;
    case SELECT_WHERE_CAULSE:
        data=API.select(parsetree.getTableInfo, parsetree.condition);
        break;
    case DELETE:
        rowCount = API.deleteValue(parsetree.getTableInfo,parsetree.condition);
        //cout<< rowCount <<"  rows have been deleted."<<endl;
        break;
    case CREIND:
        indexInfo = parsetree.getIndexInfo;
        if(!parsetree.getTableInfo.attributes[indexInfo.attriNum].isPrimaryKey&&!parsetree.getTableInfo.attributes[indexInfo.attriNum].isUnique){//不是primary key，不可以建index
            str2qstr = QString::fromStdString(parsetree.getTableInfo.attributes[indexInfo.attriNum].name);
            ui->plainTextEdit->insertPlainText("Column "+str2qstr+"  is not unique.\n");
            //cout << "Column " << parsetree.getTableInfo.attributes[indexInfo.attriNum].name <<"  is not unique."<< endl;
            break;
        }
        API.createIndex(indexInfo);
        str2qstr = QString::fromStdString(indexInfo.indexName);
        ui->plainTextEdit->insertPlainText("The index "+str2qstr+" has been created successfully.\n");
        //cout<<"The index "<< indexInfo.indexName << " has been created successfully."<<endl;
        break;
    case INDEXERROR:
        ui->plainTextEdit->insertPlainText("The index on primary key of table has been existed.\n");
        //cout<<"The index on primary key of table has been existed."<<endl;
        break;
    case DRPIND:
        indexInfo = API.GetIndexInformation(parsetree.m_indname);
        if(indexInfo.indexName == ""){
            str2qstr = QString::fromStdString(parsetree.m_indname);
            ui->plainTextEdit->insertPlainText("Index "+str2qstr+" does not exist!\n");
            //cout << "Index " << parsetree.m_indname << " does not exist!" << endl;
        }
        API.dropIndex(indexInfo);
        ui->plainTextEdit->insertPlainText("The index has been dropped successfully.\n");
        //cout<<"The index has been dropped successfully."<<endl;
        break;
    case CREINDERR:
        ui->plainTextEdit->insertPlainText("Incorrect usage of \"create index\" query! Please check your input!\n");
        //cout << "Incorrect usage of \"create index\" query! Please check your input!" << endl;
        break;
    case QUIT:
        ui->plainTextEdit->insertPlainText("Have a good day! Press any key to close this window.\n");
        //cout << "Have a good day! Press any key to close this window." << endl;
        //getchar();
        exit(0);
        break;
    case EMPTY:
        ui->plainTextEdit->insertPlainText("Empty query! Please enter your command!\n");
        //cout << "Empty query! Please enter your command!" << endl;
        break;
    case UNKNOWN:
        ui->plainTextEdit->insertPlainText("UNKNOW query! Please check your input!\n");
        //cout << "UNKNOW query! Please check your input!" << endl;
        break;
    case SELERR:
        ui->plainTextEdit->insertPlainText("Incorrect usage of \"select\" query! Please check your input!\n");
        //cout << "Incorrect usage of \"select\" query! Please check your input!" << endl;
        break;
    case CRETABERR:
        ui->plainTextEdit->insertPlainText("Incorrect usage of \"create table\" query! Please check your input!\n");
        //cout << "Incorrect usage of \"create table\" query! Please check your input!" << endl;
        break;
    case DELETEERR:
        ui->plainTextEdit->insertPlainText("Incorrect usage of \"delete from\" query! Please check your input!\n");
        //cout << "Incorrect usage of \"delete from\" query! Please check your input!" << endl;
        break;
    case DRPTABERR:
        ui->plainTextEdit->insertPlainText("Incorrect usage of \"drop table\" query! Please check your input!\n");
        //cout << "Incorrect usage of \"drop table\" query! Please check your input!" << endl;
        break;
    case DRPINDERR:
        ui->plainTextEdit->insertPlainText("Incorrect usage of \"drop index\" query! Please check your input!\n");
        //cout << "Incorrect usage of \"drop index\" query! Please check your input!" << endl;
        break;
    case VOIDPRI:
        ui->plainTextEdit->insertPlainText("Error: invalid primary key! Please check your input!\n");
        //cout << "Error: invalid primary key! Please check your input!" << endl;
        break;
    case VOIDUNI:
        ui->plainTextEdit->insertPlainText("Error: invalid unique key! Please check your input!\n" );
        //cout << "Error: invalid unique key! Please check your input!" << endl;
        break;
    case CHARBOUD:
        ui->plainTextEdit->insertPlainText("Error: only 1~255 charactors is allowed! Please check your input!\n" );
        //cout << "Error: only 1~255 charactors is allowed! Please check your input!" << endl;
        break;
    case NOPRIKEY:
        ui->plainTextEdit->insertPlainText("No primary key is defined! Please check your input!\n");
        //cout << "No primary key is defined! Please check your input!" << endl;
        break;
    case TABLEERROR:
        ui->plainTextEdit->insertPlainText("Table is not existed,please check the database.\n");
        //cout<<"Table is not existed,please check the database."<<endl;
        break;
    case INDEXEROR:
        ui->plainTextEdit->insertPlainText("Index is not existed,please check the database.\n");
        //cout<<"Index is not existed,please check the database."<<endl;
        break;
    case COLUMNERROR:
        ui->plainTextEdit->insertPlainText("One column is not existed !\n");
        //cout<<"One column is not existed !"<<endl;
        break;
    case INSERTNUMBERERROR:
        ui->plainTextEdit->insertPlainText("The column number is not according to the columns in our database.\n");
        //cout<<"The column number is not according to the columns in our database."<<endl;
        break;
    }

}
