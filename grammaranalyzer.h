#ifndef GRAMMARANALYZER_H
#define GRAMMARANALYZER_H

#include <QList>
#include <QMultiMap>

#include "production.h"
#include "lexanalyzer.h"
#include "grammarparser.h"

class GrammarAnalyzer
{
    friend class GrammarParser;
    friend class NonTerminal;
public:
    class AnalyTableItem {
    public:
        enum class Type { BLANK, PROD, SYNCH };
        Type type;
        Production* prod;

        bool isSynch() const {
            if(type == Type::SYNCH) {return true; }
            return false;
        }

        bool isBlank() const {
            if(type == Type::BLANK) {return true; }
            return false;
        }
    };
    class OutMsg {
    public:
        QStringList stack;
        QString inputType;
        QString inputCont;
        QString action;
    };
    enum class ActionType {PROD, REDUC, JUMP, SYNCH, ACC};
public:
    GrammarAnalyzer();
    ~GrammarAnalyzer();

    void setLexAnalyzer(LexAnalyzer* lexPtr);
    void initGrammarAnalyzer();
    void parseGrammarFile();

    void establishGrammar();
    void establishAnalyTable();
    void resetGrammar();
    void resetAnalyTable();

    void initAnalyStack();
    bool grammarAnalyse();
    int grammarAnalyStep();
    void resetAnalyStatus();

    OutMsg getOutputMsg();
    void resetGrammarAnalyzer();

    QVector<OutMsg> outMsgList;                                     // 全部对外输出信息

private:
    LexAnalyzer* lex;                                                             // 词法分析器
    GrammarParser* parser;                                                  // 语法解析器
    Terminal* endTerm = nullptr;                                           // 语句结束符 仅在移除左递归后使用
    Terminal* blankTerm = nullptr;                                        // 子串 epsilon 符 仅在移除左递归后使用

    QVector<Symbol*> analyStack;                                      // 分析栈
    QVector<QVector<AnalyTableItem>> analyTable;         // 分析表
    bool isNeedNextInput = false;                                        // 获取词法单步结果的标识符
    QString lexName, lexContent;                                        // 当前的

    QVector<Production*> grammar;                                  // 经过完全解析的语法产生式
    QVector<NonTerminal*> nonTermimals;                       // 所有出现过的非终结符（无重复）
    QVector<Terminal*> termimals;                                    // 所有出现过的终结符 （无重复）
    NonTerminal* startNonTerm;                                         // 文法开始非终结符

    OutMsg outMsg;                                                          // 单步对外输出信息
    QString errMsg;                                                             // 错误信息

private:
    /*               消除左递归               */
    void removeLeftRecursion();
    void replaceDirectLeftRecursion(Production* target);
    void replaceIndirectLeftRecursion(Production* target, Production* replace);
    void simplifyUnusedProduction();
    void markUsedSymbol(QVector<bool> & usedProd);


    /*          引用与内存回收       */
    void refArrayInc(QVector<Symbol*> & array);
    void refArrayDec(QVector<Symbol*> & array);
    void establishLhsProdRelation();
    void freeUnusedSymbol(QVector<bool> & usedProd);


    /*              统计出现符号              */
    void recordSymbols();
    void recordNonTerm(NonTerminal* nonTerm);
    void recordTerm(Terminal* term);


    /*                提取公因子               */
    void factoringProduction();
    void replaceFirstNonTerm(Production* target, QVector<QVector<int>>& intersect);
    QVector<QVector<Symbol*> > genPreFactorProd(QVector<Symbol*> initProd);
    void mergeSameProd(Production* target);
    QVector<QVector<int>> calcuProdIntersect(Production* target, bool isGroup = true);
    int maxIntersectLength(Production* target, QVector<int> & intersect);
    void createFactorProd(int factorCnt, Production * target, QVector<int> & intersect);


    /*             FIRST FOLLOW SYNCH 集合计算          */
    QSet<Symbol*> calcuFirst(Symbol* sym);
    QSet<Symbol*> calcuFirst(QVector<Symbol*> & symbolList);
    QSet<Symbol*> calcuFollow(NonTerminal* nonTerm);
    QSet<Symbol*> calcuSynch(NonTerminal* & nonTerm);
    void clearSetCalcuResult();


    /*                  LL(1) 分析表构建                         */
    void reScaleAnalyTable();
    void fillTableProd(Production* prod);
    bool setAnalyTableItem(NonTerminal* lhs, Terminal* term, AnalyTableItem::Type type, Production* prod = nullptr);
    void getTableItemPos(NonTerminal* lhs, Terminal* term, int & x, int & y);



    /*                  分析过程与分析栈                        */

    int analyseHandler();
    Terminal *findTerminal(QString & termName);
    int getLexInput();
    QString printProduction(Production* prod);
    QStringList printAnalyStack();


    /*                  分析/出错信息                                */
    void sendErrorMsg(QString errMsg);
    void sendAnalyOutput(QString action);

};

#endif // GRAMMARANALYZER_H
