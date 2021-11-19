#ifndef FORM_H
#define FORM_H

#include "controller.h"

#include <QWidget>
#include <QTableWidget>
#include <QTableWidgetItem>

namespace Ui {
class Form;
}

class Form : public QWidget
{
    Q_OBJECT
public:
    enum class ShowMode { Grammar, Table, Stack };
public:
    explicit Form(QWidget *parent = nullptr);
    ~Form();

    void setController(Controller* control);
    void setContMode(ShowMode mode);

    void init();

    static void setFontPlat(QTableWidgetItem *item, int size, bool isBold);
    static void setHeaderFontPlat(QTableWidgetItem *item);

private:
    Ui::Form *ui;
    Controller* util;
    ShowMode openMode;

    void engageGrammar();
    void engageAnalyTable();
    void engageAnalyStack();

    void setTableScale(int row, int col);
    void resetTable();

    void insertTableHeader(QString title, int index, bool isHoriz = true);
    void insertTableItem(int row, int col, QString cont);

};

#endif // FORM_H
