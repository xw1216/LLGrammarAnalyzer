#include "form.h"
#include "ui_form.h"

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
}

Form::~Form()
{
    delete ui;
}

void Form::setController(Controller *control)
{
    util = control;
}

void Form::setContMode(ShowMode mode)
{
    openMode = mode;
}

void Form::init()
{
    switch (openMode) {
    case ShowMode::Grammar:
        engageGrammar(); break;
    case ShowMode::Stack:
        engageAnalyStack(); break;
    case ShowMode::Table:
        engageAnalyTable(); break;
    }
}

void Form::setFontPlat(QTableWidgetItem *item, int size, bool isBold)
{
    QFont font = item->font();
    font.setBold(isBold);
    font.setPointSize(size);
    item->setForeground(Qt::black);
    item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    item->setFont(font);
}

void Form::setHeaderFontPlat(QTableWidgetItem *item)
{
    Form::setFontPlat(item, 10, true);
}

void Form::insertTableHeader(QString title, int index, bool isHoriz)
{
    QTableWidgetItem *item = (QTableWidgetItem *) new QTableWidgetItem(title);
    Form::setHeaderFontPlat(item);
    if(isHoriz) {
        ui->viewTable->setHorizontalHeaderItem(index, item);
    } else {
        ui->viewTable->setVerticalHeaderItem(index, item);
    }
}

void Form::insertTableItem(int row, int col, QString cont) {
    QTableWidgetItem *item = (QTableWidgetItem *) new QTableWidgetItem(cont);
    Form::setFontPlat(item, 9, false);
    ui->viewTable->setItem(row, col, item);
}

void Form::engageGrammar()
{
    QStringList strList = util->getGrammar();
    ui->viewTable->setColumnCount(1);
    ui->viewTable->setRowCount(strList.size());
    insertTableHeader("语法产生式", 0);
    for(int i = 0; i < strList.size(); i++) {
        insertTableItem(0, i, strList[i]);
    }
    ui->viewTable->resizeColumnsToContents();
}

void Form::engageAnalyTable()
{
    int row = -1, col = -1;
    util->getAnalyTableSize(row, col);
    if(row < 0 || col < 0) {return ;}
    ui->viewTable->setColumnCount(col);
    ui->viewTable->setRowCount(row);
    for(int i = 0; i < row; i++) {
        insertTableHeader(util->getAnalyTableRowHeader(i), i);
    }
    for(int i = 0; i < col; i++) {
        insertTableHeader(util->getAnalyTableColHeader(i), i, false);
    }
    for(int i = 0; i < row; i++) {
        for(int j = 0; j < col; j++) {
            insertTableItem(row, col, util->getAnalyTableItem(row, col));
        }
    }
    ui->viewTable->resizeColumnsToContents();
}

void Form::engageAnalyStack()
{
    QStringList strList = util->getAnalyStack();
    ui->viewTable->setColumnCount(1);
    ui->viewTable->setRowCount(strList.size());
    insertTableHeader("栈内元素", 0);
    int cnt = strList.size();
    for(int i = cnt - 1; i >= 0; i--) {
        insertTableItem(0, i - cnt + 1, strList[i]);
    }
    ui->viewTable->resizeColumnsToContents();
}

void Form::setTableScale(int row, int coloum)
{
    ui->viewTable->setColumnCount(coloum);
    ui->viewTable->setRowCount(row);
}

void Form::resetTable()
{
    ui->viewTable->clear();
    ui->viewTable->setColumnCount(0);
    ui->viewTable->setRowCount(0);
}


