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
    GrammarAnalyzer();
    ~GrammarAnalyzer();
    void setLexAnalyzer(LexAnalyzer* lexPtr);
    void initGrammarAnalyzer();
    void parseGrammarFile();

    void establishGrammar();

private:
    LexAnalyzer* lex;                                                              // 词法分析器
    GrammarParser* parser;                                                  // 语法解析器
    Terminal* endTerm = nullptr;
    Terminal* blankTerm = nullptr;

    QVector<Symbol*> analyStack;                                      // 分析栈

    QVector<Production*> grammar;                                  // 经过完全解析的语法产生式
    QVector<NonTerminal*> nonTermimals;                      // 所有出现过的非终结符（无重复）
    QVector<Terminal*> termimals;                                    // 所有出现过的终结符 （无重复）
    NonTerminal* startNonTerm;

    QString errMsg;

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

    /*             FIRST FOLLOW SYNCH 集合计算          */
    QSet<Symbol*> calcuFirst(Symbol* sym);
    QSet<Symbol*> calcuFirst(QVector<Symbol*> & symbolList);
    QSet<Symbol*> calcuFollow(NonTerminal* nonTerm);
    QSet<Symbol*> calcuSynch(NonTerminal* & nonTerm);

    void sendErrorMsg(QString errMsg);

};

#endif // GRAMMARANALYZER_H
