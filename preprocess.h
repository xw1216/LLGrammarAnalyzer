#ifndef PREPROCESS_H
#define PREPROCESS_H

#include <exception>

#include <QDir>
#include <QMap>
#include <QString>
#include <QTextStream>

/**
 * @brief 预处理类
 * @details 该类将输入的代码字符串进行包含、宏定义以及空白符删减处理
 *  主要实现的功能有
 *  1. #include "相对路径"  该指令将指定的代码文件递归进行文本复制与展开
 *  2. #define SYMBOL target  该指令将指定的标识符替换为目标文本
 *  3. 该类将注释、连续空格、换行等文本内容进行删除
 */
class PreProcess
{
public:
        PreProcess();

        /**
         * @brief start 开始预处理
         * @param src 设置数据源
         * @return  预处理是否成功
         */
        bool start(QString & src);

        const QString &getErrMsg() const;

private:
        QString* src;   // 待处理数据源
        QString errMsg; // 错误信息
        static QMap<QString, QString> symbolMap; // 宏定义表

        int stateBase = 0;  // 预处理指定起始指针
        int lexBase = 0;    //  语法项开始指针
        int lexForward = 0; // 前向扫描指针

        /**
         * @brief mainRecognize 主分析函数
         */
        void mainRecognize();

        /**
         * @brief recursiveFileProcess 递归解析包含文件
         * @param fileText 带出递归处理得到的子文件内容
         */
        void recursiveFileProcess(QString & fileText);

        /**
         * @brief recursiveFileRecognize 递归包含文件识别
         * @param src 数据源
         */
        void recursiveFileRecognize(QString &src);

        /**
         * @brief redressSymbol 替换所有宏定义数据
         */
        void redressSymbol();
        /**
         * @brief trimSrc 删除源码前后空格
         */
        void trimSrc();

        /**
         * @brief macroHandle 指令处理函数
         */
        void macroHandle();
        /**
         * @brief notationHandle 注释处理函数
         */
        void notationHandle();
        /**
         * @brief whiteHandle 编辑字符处理函数
         * @param startPos 开始位置
         */
        void whiteHandle(int & startPos);

        /**
         * @brief getMacroName 获取宏指令类型
         * @return 宏指令类型名称
         */
        QString getMacroName();

        /**
         * @brief getFilePath 获取当前文件路径
         * @return 文件路径
         */
        QString getFilePath();
        /**
         * @brief openFile 打开文件
         * @param filename 文件路径名
         * @param text 带出的文件内容
         * @return 是否成功打开文件
         */
        bool openFile(QString & filename, QString & text);
        /**
         * @brief setIncludeFile 设置包含的文件内容
         */
        void setIncludeFile();

        /**
         * @brief getSymbolName 获取宏定义标识符
         * @param str 标识符
         */
        void getSymbolName(QString & str);
        /**
         * @brief storeSymbolToMap 将宏标识符存入表中
         * @param symbol 宏定义标识符
         * @param target 替换内容
         */
        void storeSymbolToMap(QString & symbol, QString & target);
        /**
         * @brief replaceSymbolInFile 替换文件中的宏定义标识符
         * @param symbol 宏定义标识符
         * @param target 替换内容
         */
        void replaceSymbolInFile(QString symbol, QString target);
        /**
         * @brief setDefineSymbol 设置宏定义标识符
         */
        void setDefineSymbol();

        /**
         * @brief replaceTargetStr 替换目标串中的特定字符
         * @param base 起始位置
         * @param end 结束位置
         * @param replace 带替换字串
         */
        void replaceTargetStr(int & base, int end, QString replace);

        /**
         * @brief scanJump 跳过扫描字符串注释等特定内容
         */
        void scanJump();
        /**
         * @brief scanBackspace 扫描回退一字符
         */
        void scanBackspace();
        /**
         * @brief isIdChar 是否为构成标识符的字符
         * @param character 给定字符
         * @return 是否为构成标识符的字符
         */
        bool isIdChar(QChar character);
};

#endif // PREPROCESS_H
