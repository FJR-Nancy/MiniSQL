#ifndef MINISQL_H
#define MINISQL_H

#include <QMainWindow>

#include "Main.h"

QT_BEGIN_NAMESPACE
class QLineEdit;
class QLabel;
class FindDialog;
class ReplaceDialog;
class GotoDialog;
class LinkDialog;
class TableDialog;
QT_END_NAMESPACE

namespace Ui {
class MiniSQL;
}

class MiniSQL : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MiniSQL(QWidget *parent = 0);
    ~MiniSQL();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSave_As_triggered();
    void on_actionQuit_triggered();

    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionCopy_triggered();
    void on_actionCut_triggered();
    void on_actionPaste_triggered();

    void removeSubTab(int index);
    void changeSubTab(int index);

    void on_actionFont_triggered();
    void on_actionFont_Color_triggered();
    void on_actionFont_Highlight_triggered();
    void on_actionCode_Highlighting_triggered();
    void on_actionLine_Wrapping_triggered();
    void on_actionAuto_Adjust_triggered();
    void on_actionAuto_Indent_triggered();
    void on_actionBackground_Color_triggered();

    void on_actionFind_triggered();
    void on_actionReplace_triggered();
    void on_actionGoto_triggered();

    void cursorChanged();
    void timerUpDate();

    void on_actionAbout_triggered();

    void on_actionInsert_From_File_triggered();

    void on_actionTool_Bar_triggered();
    void on_actionStatus_Bar_triggered();
    void on_actionSide_Bar_triggered();

    void on_actionLink_triggered();

    void on_actionRun_triggered();
    void ShowResult(Data data, Table tableInfo, vector<Attribute> column);
    void Execute();

private:
    Ui::MiniSQL *ui;

    void setCurrentFile(const QString &fileName);
    QString getCurrentFile();

    bool maybeSave();

    QLabel *statusLabel;
    QLabel *cursorLabel;
    QLabel *timeLabel;
    QTimer *timer;
    void initialize_statusBar();

    FindDialog *findDialog;
    ReplaceDialog *replaceDialog;
    GotoDialog *gotoDialog;
    LinkDialog *linkDialog;
    TableDialog *tableDialog;
    void initialize_dialogs();

    bool autoIndentable;
    bool lineWrappable;
    bool codeHighlightable;
    bool sideBarVisible;

    bool status;
    bool *pStatus;
};

#endif // MINISQL_H
