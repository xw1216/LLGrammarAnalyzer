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
    if(ui->ViewGrammarBtn->isEnabled()) {
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

}


void MainWindow::on_AnalyAllBtn_clicked()
{

}


void MainWindow::on_ResetAnalyBtn_clicked()
{
    toggleBtns(false);
    util.resetGrammarAnaly();
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

void MainWindow::toggleBtns(bool enable)
{
    ui->AnalyStepBtn->setEnabled(enable);
    ui->AnalyAllBtn->setEnabled(enable);
    ui->ResetAnalyBtn->setEnabled(enable);
    ui->ViewGrammarBtn->setEnabled(enable);
    ui->ViewAnalyStackBtn->setEnabled(enable);
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

