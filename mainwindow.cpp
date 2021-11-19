#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initUI();
    initUtil();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_OpenSrcFileBtn_clicked()
{
    QString srcCode = getOpenFileCont(this);
    if(srcCode.isEmpty()) {
        sendMsg("文件内容为空");
    }
    sendMsg("成功打开文件");
    ui->SrcTextEditor->setPlainText(srcCode);
}


void MainWindow::on_PreProcessBtn_clicked()
{
    QString srcCode = ui->SrcTextEditor->toPlainText();
    if(srcCode.isEmpty()) {
        sendMsg("预处理文件内容为空");
        tipWindow( "非法输入", "输入不能为空");
        return;
    }
    if(!(util.startProProcess(srcCode))) {
        sendMsg(util.getPreProcessErrMsg());
        tipWindow("预处理出错", util.getPreProcessErrMsg());
    }
    ui->SrcTextEditor->setPlainText(util.getSrcCode());
    sendMsg("预处理成功");
    isLexReady = true;

    // 如果语法已经建立完毕 解禁后续按钮
    if(isGrammarReady) {
        toggleBtns(true);
    }
}


void MainWindow::on_EstablishGrammarBtn_clicked()
{
    if(!(util.initGrammarAnaly())) {
        sendMsg(util.getErrMsg());
        tipWindow("语法出错","无法读取语法文件");
    }
    if(!(util.establishGrammar())) {
        sendMsg(util.getGrammarAnalyErrMsg());
        tipWindow("建立语法出错", util.getGrammarAnalyErrMsg());
    } else {
        ui->ViewGrammarBtn->setEnabled(true);
    }
    if(isLexReady) {
        toggleBtns(true);
    }
}


void MainWindow::on_ViewGrammarBtn_clicked()
{
    tableWindow(Form::ShowMode::Grammar);
}


void MainWindow::on_AnalyStepBtn_clicked()
{
    int analyStatus = util.grammarAnalyStep();
    if(analyStatus < 0) {
        sendMsg(util.getGrammarAnalyErrMsg());
        return;
    } else if (analyStatus == 1) {
        sendMsg("分析成功");
        ui->AnalyStepBtn->setEnabled(false);
        ui->AnalyAllBtn->setEnabled(false);
    }
    GrammarAnalyzer::OutMsg outMsg = util.getGrammarAnalyStepMsg();
    pushMsgTableItem(outMsg);
    analyStepCnt++;
}


void MainWindow::on_AnalyAllBtn_clicked()
{
    bool analyStatus = util.grammarAnalyAll();
    if(!analyStatus) {
        sendMsg(util.getErrMsg());
        return;
    } else {
        sendMsg("分析成功");
        ui->AnalyStepBtn->setEnabled(false);
        ui->AnalyAllBtn->setEnabled(false);
        Controller::MsgIter msgsIter = util.getAnalyMsgBegin();
        Controller::MsgIter msgsEnd = util.getAnalyMsgEnd();
        for(int i = 0; msgsIter != msgsEnd; msgsIter++) {
            if(i < analyStepCnt) {continue; }
            pushMsgTableItem(*msgsIter);
        }
    }
}


void MainWindow::on_ResetAnalyBtn_clicked()
{
    resetGrammarAnalyStatus();
}


void MainWindow::on_ViewAnalyTableBtn_clicked()
{
    tableWindow(Form::ShowMode::Table);
}


void MainWindow::on_ViewAnalyStackBtn_clicked()
{
    tableWindow(Form::ShowMode::Stack);
}


void MainWindow::on_SrcTextEditor_textChanged()
{
    toggleBtns(false);
    isLexReady = false;
    resetGrammarAnalyStatus();

}

void MainWindow::initUI()
{
    ui->centralHorizSplit->setStretchFactor(0, 2);
    ui->centralHorizSplit->setStretchFactor(1, 3);
    ui->grammarVertiSplit->setStretchFactor(0, 2);
    ui->grammarVertiSplit->setStretchFactor(1, 5);
    toggleBtns(false);
}

void MainWindow::initUtil()
{

}

QString MainWindow::getOpenFileCont(QWidget* parent)
{
    QString curPath = QDir::currentPath();
    QString dialogTitle = "请选择一个类C代码文件";
    QString filter = "文本文件(*.txt);;类C代码(*.c)";
    QString filename = QFileDialog::getOpenFileName(parent, dialogTitle, curPath, filter);
    if(filename.isEmpty()) {
        return QString();
    }
     QFile file(filename);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    QTextStream stream(&file);
    stream.setAutoDetectUnicode(true);
    QString text = stream.readAll();
    file.close();
    return text;
}

void MainWindow::resetGrammarAnalyStatus()
{
    analyStepCnt = 0;
    ui->ViewAnalyStackBtn->setEnabled(false);
    util.resetGrammarAnalyStatus();
    resetProcTable();
}

void MainWindow::resetProcTable()
{
    ui->ProcStatusTable->clear();
    ui->ProcStatusTable->setRowCount(0);
    ui->ProcStatusTable->setColumnCount(0);
}

void MainWindow::toggleBtns(bool enable)
{
    ui->AnalyStepBtn->setEnabled(enable);
    ui->AnalyAllBtn->setEnabled(enable);
    ui->ResetAnalyBtn->setEnabled(enable);
    ui->ViewGrammarBtn->setEnabled(enable);
//    ui->ViewAnalyStackBtn->setEnabled(enable);
    ui->ViewAnalyTableBtn->setEnabled(enable);
}

void MainWindow::sendMsg(QString msg)
{
    ui->WarningLabel->setText(msg);
}

void MainWindow::tipWindow(QString title, QString content)
{
    QMessageBox::information(this, title, content, QMessageBox::Ok, QMessageBox::NoButton);
}

void MainWindow::tableWindow(Form::ShowMode mode)
{
    Form * form = new Form();
    form->setController(&util);
    form->setContMode(mode);
    form->init();
    form->setAttribute(Qt::WA_DeleteOnClose);
    form->setWindowFlag(Qt::Window, true);
    form->setVisible(true);
    form->show();
}

void MainWindow::setProcTableHeader()
{
    ui->ProcStatusTable->setColumnCount(4);
    insertTableHeader("栈顶符号", 0);
    insertTableHeader("输入符号", 1);
    insertTableHeader("输入内容", 2);
    insertTableHeader("执行动作", 3);
    ui->ProcStatusTable->setColumnWidth(0, 25);
    ui->ProcStatusTable->setColumnWidth(1, 25);
    ui->ProcStatusTable->setColumnWidth(2, 50);
    ui->ProcStatusTable->setColumnWidth(3, 100);
}

void MainWindow::insertTableHeader(QString title, int index)
{
    QTableWidgetItem *item = (QTableWidgetItem *) new QTableWidgetItem(title);
    Form::setHeaderFontPlat(item);
    ui->ProcStatusTable->setHorizontalHeaderItem(index, item);
}

void MainWindow::insertTableItem(int row, int col, QString cont) {
    QTableWidgetItem *item = (QTableWidgetItem *) new QTableWidgetItem(cont);
    Form::setFontPlat(item, 9, false);
    ui->ProcStatusTable->setItem(row, col, item);
}

void MainWindow::pushMsgTableItem(GrammarAnalyzer::OutMsg &outMsg)
{
    int targetRow = ui->ProcStatusTable->rowCount();
    ui->ProcStatusTable->setRowCount(targetRow + 1);
    insertTableItem(targetRow, 0, outMsg.stackTop);
    insertTableItem(targetRow, 1, outMsg.inputType);
    insertTableItem(targetRow, 2, outMsg.inputCont);
    insertTableItem(targetRow, 3, outMsg.action);
}


void MainWindow::on_ResetGrammarBtn_clicked()
{
    toggleBtns(false);
    util.resetGrammar();
    isGrammarReady = false;
}

