#ifndef GRAMMARANALYZER_H
#define GRAMMARANALYZER_H

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

    bool grammarAnalyStep();
    void analyStepOutput(QString & stack, QString & input, QString & action);
    void resetAnalyStatus();

private:
    LexAnalyzer* lex;                                                             // 词法分析器
    GrammarParser* parser;                                                  // 语法解析器
    Terminal* endTerm = nullptr;                                           // 语句结束符
    Terminal* blankTerm = nullptr;                                        // 子串 epsilon 符

    QVector<Symbol*> analyStack;                                      // 分析栈
    QVector<QVector<AnalyTableItem>> analyTable;         // 分析表

    QVector<Production*> grammar;                                  // 经过完全解析的语法产生式
    QVector<NonTerminal*> nonTermimals;                       // 所有出现过的非终结符（无重复）
    QVector<Terminal*> termimals;                                    // 所有出现过的终结符 （无重复）
    NonTerminal* startNonTerm;                                         // 文法开始非终结符

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


    /*          统计出现符号                  */
    void recordSymbols();
    void recordNonTerm(NonTerminal* nonTerm);
    void recordTerm(Terminal* term);


    /*                提取公因子               */
    void factoringProduction();
    void replaceFirstNonTerm(Production* target, int index, Production* replace);
    void mergeSameProd(Production* target);
    QVector<QVector<int>> calcuFirstIntersect(Production* target);
    void createFactorProd(int factorCnt, QVector<QVector<Symbol*>> & rhs);


    /*             FIRST FOLLOW SYNCH 集合计算          */
    QSet<Symbol*> calcuFirst(Symbol* sym);
    QSet<Symbol*> calcuFirst(QVector<Symbol*> & symbolList);
    QSet<Symbol*> calcuFollow(NonTerminal* nonTerm);
    QSet<Symbol*> calcuSynch(NonTerminal* & nonTerm);
    void clearSetCalcuResult();


    /*                  LL(1) 分析表构建                         */

    void getTableItemPos(NonTerminal* lhs, Terminal* term, int & x, int & y);
    void setAnalyTableItemProd(int x, int y, AnalyTableItem::Type type, Production* prod = nullptr);


    /*                  分析过程与分析栈                        */
    void initAnalyStack();
    QString printProduction(Production* prod);
    QString printStepAction(ActionType* type, Symbol* stackTop, Symbol* input, Production* prod);
    QString printAnalyStack();





    void sendErrorMsg(QString errMsg);

};

#endif // GRAMMARANALYZER_H
