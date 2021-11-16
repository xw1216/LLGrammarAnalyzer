#ifndef GRAMMARPARSER_H
#define GRAMMARPARSER_H

#include <QDir>
#include <QFile>
#include <QString>
#include <QVector>
#include <QStringList>
#include <QTextStream>

#include "production.h"

class GrammarParser
{
public:
    GrammarParser();

    /**
     * @brief initParser Parser 初始化总控函数
     */
    bool initParser();

    /**
     * @brief startParser   Parser 语法解析总控
     */
    void startParser();

    void clear();

    void getGrammar(QVector<Production*> & grammar);

private:
    const QString grammarFilename = "grammar.txt";          // 固定的 grammar 文件地址
    QString grammarStr;                                                     // 从文件读入的语法描述串内容
    QStringList grammarMnimonic;                                    // 解析后的每个文法产生式的助记符形式
    QVector<Production*> grammar;                                  // 经过完全解析的语法产生式
    QVector<NonTerminal*> nonTerms;
    QVector<Terminal*> terms;

private:
    /*               Parser 初始化               */

    /**
     * @brief readGrammarFile   从固定文件中读入语法描述
     * @return
     */
    bool readGrammarFile();

    /**
     * @brief removeStrCR   若行尾为 CRLF ， 去除 CR
     */
    void removeStrCR();

    /**
     * @brief resolveGrammarMnimonic    解析文件内容生成文法产生式的助记符形式
     */
    void resolveGrammarMnimonic();

    /*                 Parser 语法解析                     */

    /**
     * @brief resolveProduction     解析单个语法产生式
     * @param single    单个语法产生式助记符
     */
    void resolveProduction(QString & single);

    /**
     * @brief terminalHandler   终结符处理函数
     * @param prod  产生式引用
     * @param terminalName 终结符名称
     */
    void terminalHandler(Production* prod, QString terminalName);

    /**
     * @brief nonTerminalHandler 非终结符处理函数
     * @param prod  产生式引用
     * @param nonTerminalName   非终结符名称
     * @param isRHS 是否为右侧符号
     */
    void nonTerminalHandler(Production* prod, QString & nonTerminalName, bool isRHS, int indexOfProd = -1);

    int findSameProduction(QString & nonTerminalName);

    NonTerminal* findSameNonTerminal(QString & nonTerminalName);

    Terminal* findSameTerminal(QString & terminalName);

};

#endif // GRAMMARPARSER_H
