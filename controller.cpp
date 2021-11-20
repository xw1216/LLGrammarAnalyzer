#include "controller.h"

Controller::Controller()
{
    initCompiler();
}

Controller::~Controller()
{
    if(this->preProcess) { delete  this->preProcess; this->preProcess = nullptr; }
    if(this->lexAnaly) { delete  this->lexAnaly; this->lexAnaly = nullptr; }
    if(this->grammarAnaly) { delete  this->grammarAnaly; this->grammarAnaly = nullptr;}
}

void Controller::initCompiler()
{
    this->preProcess = (PreProcess*) new PreProcess();
    this->lexAnaly = (LexAnalyzer*) new LexAnalyzer();
    this->grammarAnaly = (GrammarAnalyzer*) new GrammarAnalyzer();
    if(!preProcess || !lexAnaly || !grammarAnaly) {
        throw "控制器初始化无法完成";
    }
}

bool Controller::startProProcess(QString &in)
{
    srcCode = in;
    if(!preProcess->start(srcCode)) {
         throw preProcess->getErrMsg();
        return false;
    }
    lexAnaly->setSrc(srcCode);
    lexAnaly->initUtil();
    return true;
}

bool Controller::initGrammarAnaly()
{
    grammarAnaly->setLexAnalyzer(lexAnaly);
    grammarAnaly->resetGrammarAnalyzer();
    if(!(grammarAnaly->initGrammarAnalyzer())) {
        throw grammarAnaly->getErrMsg();
        return false;
    }
    if(!(grammarAnaly->parseGrammarFile())) {
        throw grammarAnaly->getErrMsg();
        return false;
    }
    return true;
}

bool Controller::establishGrammar()
{
    grammarAnaly->establishGrammar();
    grammarAnaly->establishAnalyTable();
    grammarAnaly->initAnalyStack();
    if(!(grammarAnaly->getErrMsg().isEmpty())) {
        throw grammarAnaly->getErrMsg();
        return false;
    }
    return true;
}

void Controller::resetGrammar()
{
    lexAnaly->setSrc(srcCode);
    lexAnaly->initUtil();
    grammarAnaly->resetGrammarAnalyzer();
}

void Controller::resetGrammarAnalyStatus()
{
    lexAnaly->setSrc(srcCode);
    lexAnaly->initUtil();
    grammarAnaly->resetAnalyStatus();
    grammarAnaly->initAnalyStack();
}

bool Controller::grammarAnalyAll()
{
    if(grammarAnaly->grammarAnalyse()) {
        return true;
    } else {
        if(!(lexAnaly->getErrorMsg().isEmpty())) {
            throw lexAnaly->getErrorMsg();
            return false;
        } else {
            throw grammarAnaly->getErrMsg();
            return false;
        }
    }
}

Controller::MsgIter Controller::getAnalyMsgBegin()
{
    return grammarAnaly->outMsgList.begin();
}

Controller::MsgIter Controller::getAnalyMsgEnd()
{
    return grammarAnaly->outMsgList.end();
}

int Controller::grammarAnalyStep()
{
    int execStatus = grammarAnaly->grammarAnalyStep();
    if(execStatus < 0) {
        if(!(lexAnaly->getErrorMsg().isEmpty())) {
            throw lexAnaly->getErrorMsg();
        } else {
            throw grammarAnaly->getErrMsg();
        }
        return -1;
    } else {
        return execStatus;
    }
}

GrammarAnalyzer::OutMsg Controller::getGrammarAnalyStepMsg()
{
    return grammarAnaly->getOutputMsg();
}

QStringList Controller::getAnalyStack()
{
    return grammarAnaly->printAnalyStack();
}

QStringList Controller::getGrammar()
{
    return grammarAnaly->printGrammar();
}

QString Controller::getSrcCode()
{
    return srcCode;
}

QString Controller::getPreProcessErrMsg()
{
    return preProcess->getErrMsg();
}

QString Controller::getGrammarAnalyErrMsg()
{
    return grammarAnaly->getErrMsg();
}

QString Controller::getErrMsg()
{
    return errMsg;
}

void Controller::getAnalyTableSize(int &row, int &col)
{
    grammarAnaly->getAnalyTableSize(row, col);
}

QString Controller::getAnalyTableColHeader(int col)
{
    return grammarAnaly->getAnalyTableColHeader(col);
}

QString Controller::getAnalyTableRowHeader(int row)
{
    return grammarAnaly->getAnalyTableRowHeader(row);
}

QString Controller::getAnalyTableItem(int x, int y)
{
    return grammarAnaly->getAnalyTableItem(x, y);
}
