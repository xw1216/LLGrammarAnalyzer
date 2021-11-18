#ifndef LEXANALYZER_H
#define LEXANALYZER_H

#include <QDir>
#include <QFile>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QIODevice>
#include <QFileDialog>
#include <QTextStream>

#include "preprocess.h"
#include "terminal.h"

/**
 * @brief 词法识别器类
 * @details 该类整合了预处理子程序，可以将经过预处理的源码
 * 处理识别出源码中对应词法单元
 * 提供全体识别与调用子程序逐次识别的两种接口
 * 可处理的词法单元可见对应表格
 */
class LexAnalyzer
{
public:
    LexAnalyzer();
    ~LexAnalyzer();
    const QString &getSrc() const;
    void setSrc(const QString &newSrc);

public:
    /**
     * @brief The SymbolItem class 用于记录 Token 值与类型
     */
    class SymbolItem {
    public:
        enum class Type{    KEYWORD=1, ID = 21, INTEGER = 22,
                                    FLOAT = 23, STRING = 24, OPERATOR=25 };
        SymbolItem() {value = "";}
        SymbolItem(QString value, Type itemType) {
            this->value = value;
            this->itemType = itemType;
        }
        QString getValue() const { return value; }
        Type getType() const { return itemType; }
    private:
        QString value;
        Type itemType;
    };

public:
    friend class PreProcess;
    /**
     * @brief initUtil 初始化内部变量
     */
    void initUtil();

    /**
     * @brief startPreProcess 调用预处理子程序
     * @return 子程序执行是否成功
     */
//    bool startPreProcess();

    /**
     * @brief startLexAnalyze 开始全体词法分析
     * @param symbolIter 带出Token链表的起始迭代器
     * @return  词法分析是否成功
     */
    bool startLexAnalyze(QStringList::Iterator & symbolIter);

    /**
     * @brief lexAnalyByStep 开始单步词法分析
     * @param symbol 带出检测出的Token
     * @return 是否执行成功或已经到达文件末尾
     */
    int lexAnalyByStep(QString & symbol, QString & content);

    // 获取相关列表的迭代器与表项数

    QStringList::Iterator getSymbolBegin();
    QStringList::Iterator getSymbolEnd();
    int getSymbolNum();
    QList<SymbolItem>::Iterator getIdBegin();
    int getIdNum();
    QList<SymbolItem>::Iterator getConstantBegin();
    int getConstantNum();

    /**
     * @brief getErrorMsg 获取处理错误信息
     * @return 错误信息
     */
    QString getErrorMsg() const;

private:
    QString src;                                        // 存放输入源码
    QString scanBufferA;                        // 左扫描半区
    QString scanBufferB;                        // 右扫描半区
//    PreProcess * preServer = nullptr;   // 预处理器指针
    QString symbolMsg;                      // 单步处理返回的Token项
    QString symbolContent;
    QString errorMsg;                           // 错误提示信息

    int lexBegin = 0;                               // 词法单元开始指针
    int lexForward = 0;                         // 词法单元向前扫描指针

    bool isReady = false;                       // 是否已经启动分析
    bool isBufferA = false;                     // 是否正在左半区
    bool isFileEnd = false;                     // 文件是否结束
    bool isBackEngaged = false;         // 是否为半区加载后的扫描后退

    QMap<QString, int> keywordMap;  // 用于快速查找关键词的Map
    QList<SymbolItem> identifierList;   // 标识符列表
    QList<SymbolItem> constantList;     // 常量表
    QStringList symbolAnalyList;            // 词法分析Token表

private:
    const int BufferLength = 128;           // 扫描缓冲区长度
    const int keywordNum = 20;            // 关键词数量

    QStringList keywordList = {
        "void", "int", "long", "float", "double",
        "bool", "string", "if", "elif", "else", "return",
        "while", "for", "break", "continue", "switch",
        "case", "default", "true", "false"
    };

    QStringList operatorList = {
        "=", "+", "-", "*", "/", "%", "==", ">", ">=", "<", "<=",
        "!=", "&", "|", "!", ";", ",", "(", ")", "{", "}", "[", "]",":"
    };

private:
    /**
     * @brief initReserveMap 初始化内部查找变量
     */
    void initReserveMap();

    /**
     * @brief mainAnalyzer 单步主词法分析函数
     * @return 该次处理是否成功
     */
    bool mainAnalyzer();

    void resetResult();

    /**
     * @brief engageBuffer 装载扫描半区
     * @param isLeftBuffer 是否需要装载左半区
     */
    void engageBuffer(bool isLeftBuffer);
    /**
     * @brief replaceBufferChar 替换缓冲区内特定数据
     * @param buffer 指定缓冲区引用
     * @param length 替换长度
     * @param src 源码
     */
    void replaceBufferChar(QString & buffer, int length, QString & src);

    /**
     * @brief getNextChar 获取需要扫描的下一个字符
     * @return Unicode字符
     */
    QChar getNextChar();

    /**
     * @brief scanBackspace 扫描退格
     */
    void scanBackspace();


    /**
     * @brief findKeyword  查找目标是否为关键字
     * @param target 查找目标
     * @return 查找到的关键词表索引，若不存在则返回-1
     */
    int findKeyword(QString & target);
    /**
     * @brief findOperator 查找目标是否为操作符
     * @param target 查找目标
     * @return 查找到的操作符表索引，若不存在则返回-1
     */
    int findOperator(QString target);

    /**
     * @brief pushId 将标识符压入表中
     * @param id 指定标识符
     */
    void pushId(QString & id);
    /**
     * @brief pushConstant 将常数压入表中
     * @param constant 指定常数
     * @param type 常数类型
     */
    void pushConstant(QString & constant, SymbolItem::Type type);

    /**
     * @brief symbolRecogHandler 标识符识别函数
     * @param ch 正在扫描的字符
     * @param str 暂存字符串
     */
    void symbolRecogHandler(QChar & ch, QString & str);
    /**
     * @brief numberRecogHandler 数字识别函数
     * @param ch 正在扫描的字符
     * @param str 暂存字符串
     */
    void numberRecogHandler(QChar & ch, QString & str);
    /**
     * @brief stringRecogHandler 字符串识别函数
     * @param ch 正在扫描的字符
     * @param str 暂存字符串
     */
    void stringRecogHandler(QChar & ch, QString & str);
    /**
     * @brief operatorRecogHandler 操作符识别函数
     * @param ch 正在扫描的字符
     * @param str 暂存字符串
     * @return 是否为操作符
     */
    bool operatorRecogHandler(QChar & ch, QString & str);

    /**
     * @brief getMnemonicName 获得指定类型的助记符名称
     * @param symbolStr 标识符名
     * @param type 类型
     * @return  助记符名称
     */
    QString getMnemonicName(QString symbolStr, SymbolItem::Type type);
    /**
     * @brief getOperatorName 获取操作符名称
     * @param symbol 指定操作符
     * @return  操作符名称
     */
    QString getOperatorName(QString symbol);
    /**
     * @brief sendSymbolMsg 发送单步分析结果
     * @param symbol 分析结果
     */
    void sendSymbolMsg(QString & symbol, QString & content);
    /**
     * @brief generateSymbolFlag 生成单步分析Token
     * @param symbolStr 词法单元内容
     * @param type 类型
     */
    void generateSymbolFlag(QString symbolStr, SymbolItem::Type type);

    /**
     * @brief isLetter 是否为字母
     * @param character 给定字符
     * @return 是否为字母
     */
    bool isLetter(QChar & character);
    /**
     * @brief isNumber 是否为数字字符
     * @param character 给定字符
     * @return 是否为数字字符
     */
    bool isNumber(QChar & character);
    /**
     * @brief isIdChar 是否为可构成标识符字符
     * @param character 给定字符
     * @return 是否为可构成标识符字符
     */
    bool isIdChar(QChar & character);

};

#endif // LEXANALYZER_H
