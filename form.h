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


private:
    Ui::Form *ui;
    Controller* util;
    ShowMode openMode;

    void engageGrammar();
    void engageAnalyTable();
    void engageAnalyStack();

};

#endif // FORM_H
