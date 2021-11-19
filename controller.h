#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "preprocess.h"
#include "lexanalyzer.h"
#include "grammaranalyzer.h"

class Controller
{
public:
    typedef  QVector<GrammarAnalyzer::OutMsg>::Iterator  MsgIter;
    Controller();
    ~Controller();

    // 执行过程函数
    bool startProProcess(QString & in);

    bool initGrammarAnaly();

    bool establishGrammar();

    void resetGrammar();

    void resetGrammarAnalyStatus();


    // 语法分析全部执行
    bool grammarAnalyAll();
    MsgIter getAnalyMsgBegin();
    MsgIter getAnalyMsgEnd();

    // 语法分析单步执行
    int grammarAnalyStep();
    GrammarAnalyzer::OutMsg getGrammarAnalyStepMsg();

     // 输出信息获取
    QStringList getAnalyStack();
    QStringList getGrammar();
    QString getSrcCode();
    QString getPreProcessErrMsg();
    QString getGrammarAnalyErrMsg();
    QString getErrMsg();


    // 获取展示LL(1) 分析表的内容
    void getAnalyTableSize(int & row, int & col);
    QString getAnalyTableColHeader(int col);
    QString getAnalyTableRowHeader(int row);
    QString getAnalyTableItem(int x, int y);

private:
    PreProcess* preProcess = nullptr;
    LexAnalyzer* lexAnaly = nullptr;
    GrammarAnalyzer* grammarAnaly = nullptr;

    QString srcCode;
    QString errMsg;

private:
     void initCompiler();
};

#endif // CONTROLLER_H
