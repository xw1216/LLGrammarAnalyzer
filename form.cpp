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

void Form::engageGrammar()
{

}

void Form::engageAnalyTable()
{

}

void Form::engageAnalyStack()
{

}

void Form::setTableScale(int coloum, int row)
{
    ui->viewTable->setColumnCount(coloum);
    ui->viewTable->setRowCount(row);
}
