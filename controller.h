#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "preprocess.h"
#include "lexanalyzer.h"
#include "grammaranalyzer.h"

class Controller
{
public:
    Controller();
    bool startProProcess(QString & in, QString & out);

    int grammarAnalyStep();

    // 获取单步语法分析展示内容
    QString getAnalyStackText();
    QString getLexInputType();
    QString getLexInputText();
    QString getActionText();
    QString getErrMsg();


    // 获取展示LL(1) 分析表的内容
    void getAnalyTableSize(int & row, int & col);
    QString getAnalyTableColHeader(int col);
    QString getAnalyTableRowHeader(int row);
    QString getAnalyTableItem(int & x, int & y);

private:
    PreProcess* preProcess = nullptr;
    LexAnalyzer* lexAnaly = nullptr;
    GrammarAnalyzer* grammarAnaly = nullptr;

    QString errMsg();

    void initComponents();
    void engageSrcCode(QString & src);


};

#endif // CONTROLLER_H
