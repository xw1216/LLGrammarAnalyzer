#include "lexanalyzer.h"

LexAnalyzer::LexAnalyzer()
{
//    preServer = new PreProcess();
}

LexAnalyzer::~LexAnalyzer()
{
//    if(preServer != nullptr) {
//        delete preServer;
//    }
}

const QString &LexAnalyzer::getSrc() const
{
    return src;
}

void LexAnalyzer::setSrc(const QString &newSrc)
{
    src = newSrc;
}

void LexAnalyzer::initUtil()
{
   resetResult();
   lexBegin = lexForward = 0;
   scanBufferA.resize(BufferLength + 1, 1);
   scanBufferB.resize(BufferLength + 1, 1);
   scanBufferA[BufferLength] = 0;
   scanBufferB[BufferLength] = 0;
   initReserveMap();
}

void LexAnalyzer::initReserveMap()
{
    if(keywordMap.isEmpty()) {
        for(int i = 0; i < keywordNum; i++) {
            keywordMap.insert(keywordList.at(i), i);
        }
    }
}

bool LexAnalyzer::mainAnalyzer()
{
    QString str;
    QChar ch =  getNextChar().toLatin1();
    // Stop resolving any word when EOF has been received
    if(ch == 0) { return false; }
    // jump over any whitespace
    while(ch == ' ') { ch = getNextChar().toLatin1(); }
    if(isLetter(ch)) {
        symbolRecogHandler(ch, str);
        return true;
    }
    if(isNumber(ch)) {
        numberRecogHandler(ch, str);
        return true;
    }
    if(ch == '"') {
        stringRecogHandler(ch, str);
        return true;
    }
    if(operatorRecogHandler(ch, str)) {
        return true;
    } else {
        throw QString("无法识别的字符");
    }
}

void LexAnalyzer::resetResult()
{
    identifierList.clear();
    constantList.clear();
    symbolAnalyList.clear();
    isReady = true;
    isBufferA = false;
    isFileEnd = false;
    isBackEngaged = false;
}

void LexAnalyzer::engageBuffer(bool isLeftBuffer)
{
    if(isFileEnd) {return;}
    if(isBackEngaged) {isBackEngaged = false; return; }
    bool lessSrc = false;
    int engageLength = 0;
    if(src.length() < BufferLength) {
        lessSrc = true;
        isFileEnd = true;
        engageLength = src.length();
    } else { engageLength = BufferLength; }

    if(isLeftBuffer) {
        if(lessSrc) {
            replaceBufferChar(scanBufferA, src.length(), src);
            scanBufferA[src.length()] = 0;
        } else { replaceBufferChar(scanBufferA, BufferLength, src); }
    } else {
        if(lessSrc) {
            replaceBufferChar(scanBufferB, src.length(), src);
            scanBufferB[src.length()] = 0;
        } else { replaceBufferChar(scanBufferB, BufferLength, src); }
    }
    src.remove(0, engageLength);
    return;
}

void LexAnalyzer::replaceBufferChar(QString &buffer, int length, QString &src)
{
    if(length > BufferLength) { return; }
    for(int i = 0; i < length; i++) {
        buffer[i] = src[i];
    }
}

QChar LexAnalyzer::getNextChar()
{
    if(isBufferA && scanBufferA[lexForward] == 0) {return 0;}
    if(!isBufferA && scanBufferB[lexForward] == 0) {return 0;}
    if(lexForward + 1 >= BufferLength || isReady) {
        isReady = false;
        lexForward = 0;
        engageBuffer(!isBufferA);
        isBufferA = !isBufferA;
        return (isBufferA) ? scanBufferA[lexForward] : scanBufferB[lexForward];
    }
    else {
        return (isBufferA) ? scanBufferA[++lexForward] : scanBufferB[++lexForward];
    }
}

void LexAnalyzer::scanBackspace()
{
    if(lexForward == 0) {
        isBackEngaged = true;
        isBufferA = !isBufferA;
        lexForward = BufferLength - 1;
    } else {
        lexForward--;
    }
}

int LexAnalyzer::findKeyword(QString &target)
{
    auto mapIter = keywordMap.find(target);
    if(mapIter == keywordMap.end()) {
        return -1;
    } else {
        return mapIter.value();
    }
}

int LexAnalyzer::findOperator(QString target)
{
    return operatorList.indexOf(target);
}

void LexAnalyzer::pushId(QString &id)
{
    SymbolItem item = SymbolItem(id, SymbolItem::Type::ID);
    identifierList.push_back(item);
}

void LexAnalyzer::pushConstant(QString &constant, SymbolItem::Type type)
{
    SymbolItem item = SymbolItem(constant, type);
    constantList.push_back(item);
}

void LexAnalyzer::symbolRecogHandler(QChar &ch, QString & str)
{
    while(isIdChar(ch)) {
        str.push_back(ch);
        ch = getNextChar();
    }
    scanBackspace();
    int index = findKeyword(str);
    if(index != -1) {
        generateSymbolFlag(keywordList[index], SymbolItem::Type::KEYWORD);
    } else {
        generateSymbolFlag(str, SymbolItem::Type::ID);
        pushId(str);
    }
    str.clear();
}

void LexAnalyzer::numberRecogHandler(QChar &ch, QString &str)
{
    while (isNumber(ch) || ch =='.') {
        str.push_back(ch);
        ch = getNextChar();
    }
    scanBackspace();
    if(str.contains('.')) {
        generateSymbolFlag(str, SymbolItem::Type::FLOAT);
        pushConstant(str, SymbolItem::Type::FLOAT);
    } else {
        generateSymbolFlag(str, SymbolItem::Type::INTEGER);
        pushConstant(str, SymbolItem::Type::INTEGER);
    }
    str.clear();
}

void LexAnalyzer::stringRecogHandler(QChar &ch, QString &str)
{
    ch = getNextChar();
    while(ch != '"') {
        str.push_back(ch);
        ch = getNextChar();
    }
    generateSymbolFlag(str, SymbolItem::Type::STRING);
    pushConstant(str, SymbolItem::Type::STRING);
    str.clear();
}

bool LexAnalyzer::operatorRecogHandler(QChar &ch, QString &str)
{
    if(findOperator(QString(ch)) == -1) {
        scanBackspace();
        return false;
    }
    else {
        str.push_back(ch);
        while(true) {
            ch = getNextChar();
            str.push_back(ch);
            if(findOperator(str) == -1) {
                str.remove(str.length() - 1, 1);
                break;
            }
        }
        generateSymbolFlag(str, SymbolItem::Type::OPERATOR);
        scanBackspace();
        str.clear();
        return true;
    }
}

QString LexAnalyzer::getMnemonicName(QString symbolStr, SymbolItem::Type type)
{
    switch (type) {
    case SymbolItem::Type::ID:
        return "$id"; break;
    case SymbolItem::Type::INTEGER:
        return "$integer"; break;
    case SymbolItem::Type::FLOAT:
        return "$floatnum"; break;
    case SymbolItem::Type::KEYWORD:
        return "$" + symbolStr; break;
    case SymbolItem::Type::STRING:
        return "$string"; break;
    case SymbolItem::Type::OPERATOR:
        QString str = getOperatorName(symbolStr);
        if(str.isEmpty()) {return ""; }
        else return "$" + getOperatorName(symbolStr);
        break;
    }
    return "";
}

QString LexAnalyzer::getOperatorName(QString symbol)
{
    int index = operatorList.indexOf(symbol);
    switch (index) {
    case 0: return "assign"; break;
    case 1: return "plus"; break;
    case 2: return "sub"; break;
    case 3: return "mul"; break;
    case 4: return "div"; break;
    case 5: return "mod"; break;
    case 6: return "eq"; break;
    case 7: return "gre"; break;
    case 8: return "geq"; break;
    case 9: return "les"; break;
    case 10: return "leq"; break;
    case 11: return "neq"; break;
    case 12: return "and"; break;
    case 13: return "or"; break;
    case 14: return "not"; break;
    case 15: return "smc"; break;
    case 16: return "cma"; break;
    case 17: return "lpar"; break;
    case 18: return "rpar"; break;
    case 19: return "lbrc"; break;
    case 20: return "rbrc"; break;
    case 21: return "lsbrc"; break;
    case 22: return "rsbrc"; break;
    case 23: return "colon"; break;
    default:
        return "";
        break;
    }
}

void LexAnalyzer::sendSymbolMsg(QString &symbol, QString & content)
{
    this->symbolMsg = symbol;
    this->symbolContent = content;
}

void LexAnalyzer::generateSymbolFlag(QString symbolStr, SymbolItem::Type type)
{
    QString termName = getMnemonicName(symbolStr, type);
    QString termContent;
    QString propName;
    propName = "<" + termName + ", ";
    switch (type) {
    case SymbolItem::Type::ID:
        propName = propName + QString::number(identifierList.size()) + ">"; break;
        termContent = identifierList.last().getValue();
    case SymbolItem::Type::INTEGER:
    case SymbolItem::Type::FLOAT:
    case SymbolItem::Type::STRING:
        propName = propName + QString::number(constantList.size()) + ">"; break;
        termName = constantList.last().getValue();
    case SymbolItem::Type::KEYWORD:
        propName = propName + "->"; break;
    case SymbolItem::Type::OPERATOR:
        propName = propName + "->"; break;
        break;
    }
    sendSymbolMsg(termName, termContent);
    symbolAnalyList.append(propName);
}

bool LexAnalyzer::isLetter(QChar &character)
{
    char ascii = character.toLatin1();
    if((ascii >='a' && ascii <= 'z') || (ascii >= 'A' && ascii <= 'Z') || ascii == '_') {
        return true;
    }
    return false;
}

bool LexAnalyzer::isNumber(QChar &character)
{
    char ascii = character.toLatin1();
    if(ascii >= '0' && ascii <= '9') {return true;}
    return false;
}

bool LexAnalyzer::isIdChar(QChar &character)
{
    if(isLetter(character) || isNumber(character)) { return true; }
    return false;
}

//bool LexAnalyzer::startPreProcess()
//{
//    if(!preServer->start(src)) {
//        errorMsg = preServer->getErrMsg();
//        return false;
//    }
//    return true;
//}

bool LexAnalyzer::startLexAnalyze(QStringList::Iterator & symbolIter)
{
    try {
        bool keep = true;
        while(keep) {
            keep = mainAnalyzer();
        }
    }  catch (QString e) {
        errorMsg = e;
        return false;
    }
    symbolIter = getSymbolBegin();
    return true;
}

int LexAnalyzer::lexAnalyByStep(QString &symbol, QString & content)
{
    bool isEnd = false;
    try {
        isEnd = !mainAnalyzer();
    }  catch (QString e) {
        errorMsg = e;
        symbolMsg.clear();
        symbolContent.clear();
        return -1;
    }
    symbol = symbolMsg;
    content = symbolContent;
    return isEnd ? 0 : 1;
}

QStringList::Iterator LexAnalyzer::getSymbolBegin()
{
    return symbolAnalyList.begin();
}

QStringList::Iterator LexAnalyzer::getSymbolEnd() {
    return symbolAnalyList.end();
}

int LexAnalyzer::getSymbolNum()
{
    return symbolAnalyList.length();
}

QList<LexAnalyzer::SymbolItem>::Iterator LexAnalyzer::getIdBegin()
{
    return identifierList.begin();
}

int LexAnalyzer::getIdNum()
{
    return identifierList.length();
}

QList<LexAnalyzer::SymbolItem>::Iterator LexAnalyzer::getConstantBegin()
{
    return constantList.begin();
}

int LexAnalyzer::getConstantNum()
{
    return constantList.length();
}

QString LexAnalyzer::getErrorMsg() const
{
    return errorMsg;
}
