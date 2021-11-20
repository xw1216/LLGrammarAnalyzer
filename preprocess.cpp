#include "preprocess.h"

QMap<QString, QString> PreProcess::symbolMap;

PreProcess::PreProcess() {}

bool PreProcess::start(QString &src)
{
    this->src = &src;
    lexBase = 0;
    lexForward = 0;
    stateBase = 0;
    try {
        mainRecognize();
    }  catch (QString & e) {
        errMsg = e;
        return false;
    }
    redressSymbol();
    trimSrc();
    return true;
}

const QString &PreProcess::getErrMsg() const
{
    return errMsg;
}

void PreProcess::mainRecognize()
{
    while(stateBase < src->length()) {
        switch (src->at(stateBase).toLatin1()) {
        case '#':
            macroHandle();
            scanBackspace();
            break;
        case '/':
            notationHandle();
            scanBackspace();
            break;
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            whiteHandle(stateBase);
            scanBackspace();
            break;
        case '"':
            scanJump();
            scanBackspace();
            break;
        default:
            if(src->at(stateBase).toLatin1() == 0) {
            throw QString("无法识别的字符。"); }
            break;
        }
        stateBase++;
    }
}

void PreProcess::recursiveFileProcess(QString & fileText)
{
    PreProcess* processServer = new PreProcess();
    try {
        processServer->recursiveFileRecognize(fileText);
    }  catch (QString & e) {
        delete processServer;
        throw e;
    }
    if(processServer != nullptr) {
        delete processServer;
        processServer = nullptr;
    }
}

void PreProcess::recursiveFileRecognize(QString &src)
{
    this->src = &src;
    lexBase = 0;
    lexForward = 0;
    stateBase = 0;
    mainRecognize();
}

void PreProcess::redressSymbol()
{
    for(auto mapIter = PreProcess::symbolMap.begin();
        mapIter != PreProcess::symbolMap.end(); mapIter++) {
        replaceSymbolInFile(mapIter.key(), mapIter.value());
    }
}

void PreProcess::trimSrc()
{
    int spaceCnt = 0;
    for(int i = 0; i < src->length(); i++) {
        if(src->at(i) == ' ') {spaceCnt++;}
        else {break;}
    }
    src->remove(0, spaceCnt);
    spaceCnt = 0;
    for(int i = src->length() - 1; i >= 0; i--) {
        if(src->at(i) == ' ') {spaceCnt++;}
        else {break;}
    }
    src->remove(src->length() - spaceCnt, spaceCnt);
}

QString PreProcess::getMacroName()
{
    QString macro;
    lexForward = lexBase;
    // Get macro name
    for(++lexForward; lexForward < src->length(); lexForward++) {
        char ascii  = src->at(lexForward).toLatin1();
        if(ascii == 0 || ascii == '\r' || ascii == '\n') {
            return QString();
        } else if(ascii == ' '){
            // set  pointer for processing extra whitespace
            lexBase = lexForward;
            break;
        } else {
            macro.push_back(src->at(lexForward));
        }
    }
    return macro;
}

QString PreProcess::getFilePath()
{
    QString filename;
    char ascii = src->at(lexForward).toLatin1();
    if(ascii != '"') {
       throw QString("文件路径错误。");
    } else {
        if(lexForward >= src->length() - 1) {throw QString("文件路径错误。");}
        lexForward++;
        for(;  lexForward < src->length() ; lexForward++) {
            ascii = (src->at(lexForward)).toLatin1();
            if(ascii == '\t' | ascii == '\n' | ascii == '\r') { throw QString("文件路径错误。"); }
            if(ascii == '"') {break;}
            filename.push_back(src->at(lexForward));
        }
        lexForward++;
    }
    return filename;
}

bool PreProcess::openFile(QString &filename, QString & text)
{
    if(filename.isEmpty()) { return false; }
    QDir dir(filename);
     QFile file(dir.absolutePath());
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {return false;}
    QTextStream stream(&file);
    stream.setAutoDetectUnicode(true);
    text = stream.readAll();
    return true;
}

void PreProcess::setIncludeFile()
{
    // open target file
    QString filename = getFilePath();
    QString fileText;
    if(!openFile(filename, fileText)) { throw QString("无法包含该文件。"); }
    // include files recursivly
    // TODO: exception may cause memory leak fix it
    recursiveFileProcess(fileText);
    // replace the macro with file text
    replaceTargetStr(stateBase, lexForward, fileText);
}

void PreProcess::getSymbolName(QString &str)
{
    char ascii;
    for(; lexForward < src->length(); lexForward++) {
        ascii = (src->at(lexForward)).toLatin1();
        if(ascii == ' ' || ascii == '\t' | ascii == '\n' | ascii == '\r') {break;}
        str.push_back(src->at(lexForward));
    }
    lexBase = lexForward;
}

void PreProcess::storeSymbolToMap(QString &symbol, QString &target)
{
    auto item = PreProcess::symbolMap.find(symbol);
    if(item !=PreProcess::symbolMap.end()) {
        PreProcess::symbolMap.insert(symbol, target);
    } else {
        PreProcess::symbolMap.remove(symbol);
        PreProcess::symbolMap.insert(symbol, target);
    }
}

void PreProcess::replaceSymbolInFile(QString symbol, QString target)
{
    int index = src->indexOf(symbol);
    int reScanPos = 0;
    while(index != -1) {
        // 防止截取到连续标识符中的Symbol
        int begin = index - 1, end = index + symbol.length();
        if((begin >= 0 && isIdChar(src->at(begin)) ||
            (end < src->length() && isIdChar(src->at(end))))) {
            reScanPos = index + 1;
        } else {
            src->replace(index, symbol.length(), target);
        }
        index = src->indexOf(symbol, reScanPos);
    }
}

void PreProcess::setDefineSymbol()
{
    QString symbol;
    QString target;
    getSymbolName(symbol);
    whiteHandle(lexBase);
    getSymbolName(target);
    storeSymbolToMap(symbol, target);
    replaceTargetStr(stateBase, lexForward, "");
}

void PreProcess::replaceTargetStr(int & base, int end, QString replace)
{
    src->replace(base, (end - base), replace);
    lexBase = lexForward = base = base + replace.length();
}

void PreProcess::macroHandle()
{
    lexBase = stateBase;
    QString macro = getMacroName();
    whiteHandle(lexBase);
    if(macro == QString("include")) {
        setIncludeFile();
    } else if (macro == QString("define")) {
        setDefineSymbol();
    } else {
        throw QString("无法识别的预处理指令。");
    }
}

void PreProcess::notationHandle()
{
    lexForward= lexBase = stateBase;
    if(src->at(lexForward + 1) == '/') {
        while(true) {
            lexForward++;
            if(lexForward >= src->length()) {break ;}
            if(src->at(lexForward) == '\n') { lexForward++; break; }
        }
        replaceTargetStr(stateBase, lexForward, "");
    } else if(src->at(lexForward + 1) == '*') {
        bool flag = false;
        lexForward++;
        while(true) {
            lexForward++;
            if(lexForward >= src->length()) {break ;}
            if(src->at(lexForward) == '*') { flag = true; }
            else if (src->at(lexForward) == '/' && flag) { lexForward++; break; }
            else {
                flag = false;
            }
        }
        replaceTargetStr(stateBase, lexForward, "");
    }
}

void PreProcess::whiteHandle(int & startPos)
{
    // note that lexBase and lexForward will jump over whitespace automatically
    QChar curChar;
    lexForward = startPos;
    bool beforeSpaceFlag = false;

    if(lexForward - 1 >= 0 && src->at(lexForward-1) == ' ') {
        beforeSpaceFlag = true;
    }
    while(true) {
        lexForward++;
        if(lexForward >= src->length()) {break ;}
        curChar = src->at(lexForward);
        if( !(curChar == ' ' || curChar == '\t' || curChar == '\n' || curChar == '\r')) {
            break;
        }
    }
    if(beforeSpaceFlag) {
        stateBase--;
        lexBase--;
    }
    replaceTargetStr(startPos, lexForward, " ");
}

void PreProcess::scanJump()
{
    lexForward = lexBase = stateBase;
    QChar curChar;
    while(true) {
        lexForward++;
        curChar = src->at(lexForward);
        if(curChar == '"') {
            lexForward++;
            break;
        }
    }
    stateBase = lexBase = lexForward;
}

void PreProcess::scanBackspace()
{
    if(stateBase >=0) {
        stateBase--;
    } else {
        stateBase = -1;
    }
}

bool PreProcess::isIdChar(QChar character)
{
    char ascii = character.toLatin1();
    if((ascii > 'a' && ascii < 'z') || (ascii > 'A' && ascii < 'Z') || (ascii > '0' && ascii < '9')) {
        return true;
    }
    return false;
}
