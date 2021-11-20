#include "grammarparser.h"

GrammarParser::GrammarParser()
{

}

bool GrammarParser::initParser()
{
    if(!readGrammarFile()) {
        return false;
    }
    removeStrCR();
    resolveGrammarMnimonic();
    return true;
}

bool GrammarParser::readGrammarFile()
{
//    QDir dir = QDir(QDir::currentPath()).filePath("grammar.txt");
//    QDir dir = QDir("../grammar.txt");
//    if(!dir.exists()) {return false; }
     QFile file("./grammar.txt");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) { return false; }
    QTextStream stream(&file);
    stream.setAutoDetectUnicode(true);
    grammarStr = stream.readAll();
    file.close();
    return true;
}

void GrammarParser::removeStrCR()
{
    grammarStr.remove(QChar('\r'));
    grammarStr = grammarStr.trimmed();
}

void GrammarParser::resolveGrammarMnimonic()
{
    grammarMnimonic = grammarStr.split(QChar('\n'), Qt::SkipEmptyParts);
    for(int i = 0; i < grammarMnimonic.size(); i++) {
        grammarMnimonic[i] = grammarMnimonic[i].trimmed();
    }
}

void GrammarParser::startParser()
{
    for(int i = 0; i < grammarMnimonic.size(); i++) {
        QString str = grammarMnimonic.at(i);
        resolveProduction(str);
    }
}

void GrammarParser::reset()
{
    grammar.clear();
    grammarMnimonic.clear();
    grammar.clear();
    nonTerms.clear();
    terms.clear();
}


void GrammarParser::getGrammar(QVector<Production *> &grammar)
{
    this->grammar.swap(grammar);
    this->grammar.clear();
    this->nonTerms.clear();
    this->terms.clear();
}

void GrammarParser::resolveProduction(QString &single)
{
    Production* prod = new Production();
    if(prod == nullptr) {return;}
    QStringList phrases = single.split(" ", Qt::SkipEmptyParts);
    int indexProd = -1;
    bool isRHS = false;

    for(int i = 0; i < phrases.size(); i++) {
        QString str = phrases.at(i);
        if(str.size() <= 0) {continue;}
        if(str.at(0) == QChar('@')) {
            isRHS  = true;
            prod->newProduction();
        } else if (str.at(0) == QChar('|')) {
            prod->newProduction();
        } else if(str.at(0) == QChar('$')) {
            terminalHandler(prod, str);
        } else {
            if(isRHS) {  nonTerminalHandler(prod, str, isRHS); }
            else {
                indexProd = findSameProduction(str);
                if(indexProd == -1) {
                    indexProd = grammar.size();
                    nonTerminalHandler(prod, str, isRHS, indexProd);
                } else {
                    delete prod;
                    prod = grammar[indexProd];
                    prod->newProduction();
                }
            }
        }
    }
    if(!grammar.contains(prod)) {
        grammar.push_back(prod);
    }
}

void GrammarParser::terminalHandler(Production* prod, QString terminalName)
{
    if(prod == nullptr || terminalName.isEmpty()) {return;}
    terminalName.remove(0, 1);
    if(terminalName == "blank") { return; }
    Terminal* terminal = findSameTerminal(terminalName);
    if(!terminal) {
        terminal = new Terminal();
        if(terminal == nullptr) { return; }
        if(!Terminal::isRecognized(terminalName)) {
            delete terminal;
            terminal = nullptr;
            return;
        }
        terminal->setName(terminalName);
        terms.push_back(terminal);
    }
    terminal->refInc();
    prod->insertRhs(terminal);
}

void GrammarParser::nonTerminalHandler(Production* prod, QString &nonTerminalName, bool isRHS, int indexOfProd)
{
    if(prod == nullptr || nonTerminalName.isEmpty()) {return; }
    NonTerminal* nonTermimal = findSameNonTerminal(nonTerminalName);
    if(!nonTermimal) {
        nonTermimal = new NonTerminal();
        if(nonTermimal == nullptr) { return; }
        nonTermimal->setName(nonTerminalName);
        nonTerms.push_back(nonTermimal);
    }
    nonTermimal->refInc();
    if(isRHS) {
        prod->insertRhs(nonTermimal);
    } else {
        prod->setLhs(nonTermimal);
        prod->getLhs()->setIndexOfProd(indexOfProd);
    }
}

int GrammarParser::findSameProduction(QString &nonTerminalName)
{
    for(int i = 0; i < grammar.size(); i++) {
        if(grammar[i]->getLhs()->getName() == nonTerminalName) {
            return i;
        }
    }
    return -1;
}

NonTerminal *GrammarParser::findSameNonTerminal(QString &nonTerminalName)
{
    for(int i = 0; i < nonTerms.size(); i++) {
        if(nonTerms[i]->getName() == nonTerminalName) {
            return nonTerms[i];
        }
    }
    return nullptr;
}

Terminal *GrammarParser::findSameTerminal(QString &terminalName)
{
    for(int i = 0; i < terms.size(); i++) {
        if(terms[i]->getName() == terminalName) {
            return terms[i];
        }
    }
    return nullptr;
}
