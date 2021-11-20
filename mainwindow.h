#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "form.h"
#include "controller.h"

#include <QPlainTextEdit>
#include <QMessageBox>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_OpenSrcFileBtn_clicked();
    void on_PreProcessBtn_clicked();
    void on_EstablishGrammarBtn_clicked();
    void on_ViewGrammarBtn_clicked();
    void on_AnalyStepBtn_clicked();
    void on_AnalyAllBtn_clicked();
    void on_ResetAnalyBtn_clicked();
    void on_ViewAnalyTableBtn_clicked();
    void on_ViewAnalyStackBtn_clicked();
    void on_SrcTextEditor_textChanged();
    void on_ResetGrammarBtn_clicked();

private:
    Ui::MainWindow *ui;
    Controller util;
    bool isLexReady = false;
    bool isGrammarReady = false;
    int analyStepCnt = 0;

private:
    void initUI();
    void initUtil();
    QString getOpenFileCont(QWidget* parent);
    void resetGrammarAnalyStatus();
    void resetProcTable();

    void toggleBtns(bool enable);
    void sendMsg(QString msg);
    void tipWindow(QString title, QString content);
    void tableWindow(Form::ShowMode mode);

    void setProcTableHeader();
    void insertTableHeader(QString title, int index);
    void insertTableItem(int row, int col, QString cont);
    void pushMsgTableItem(GrammarAnalyzer::OutMsg & outMsg);
};
#endif // MAINWINDOW_H
