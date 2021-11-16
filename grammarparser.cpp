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
    QDir dir = QDir(QDir::currentPath()).filePath("grammar.txt");
    if(!dir.exists()) {return false; }
     QFile file(dir.canonicalPath());
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
}

void GrammarParser::resolveGrammarMnimonic()
{
    grammarMnimonic = grammarStr.split(QChar('\n'), Qt::SkipEmptyParts);
}

void GrammarParser::startParser()
{
    for(int i = 0; i < grammarMnimonic.size(); i++) {
        resolveProduction(grammarMnimonic[i]);
    }
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
        if(phrases[i].size() <= 0) {continue;}
        if(phrases[i].at(0) == QChar('@')) {
            isRHS  = true;
        } else if (phrases[i].at(0) == QChar('|')) {
            prod->newProduction();
        } else if(phrases[i].at(0) == QChar('$')) {
            terminalHandler(prod, phrases[i]);
        } else {
            if(isRHS) { nonTerminalHandler(prod, phrases[i], isRHS); }
            else {
                indexProd = findSameProduction(phrases[i]);
                if(indexProd == -1) {
                    indexProd = grammar.size();
                    nonTerminalHandler(prod, phrases[i], isRHS, indexProd);
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
    if(terminalName == "$blank") { return; }
    Terminal* terminal = findSameTerminal(terminalName);
    if(!terminal) {
        terminal = new Terminal();
        if(terminal == nullptr) { return; }
        if(!Terminal::isRecognized(terminalName)) {
            delete terminal;
            terminal = nullptr;
            return;
        }
        terms.push_back(terminal);
    }
    terminal->refInc();
    terminal->setName(terminalName.remove(QChar('$')));
    prod->insertRhs(terminal);

}

void GrammarParser::nonTerminalHandler(Production* prod, QString &nonTerminalName, bool isRHS, int indexOfProd)
{
    if(prod == nullptr || nonTerminalName.isEmpty()) {return; }
    NonTerminal* nonTermimal = findSameNonTerminal(nonTerminalName);
    if(!nonTermimal) {
        nonTermimal = new NonTerminal();
        if(nonTermimal == nullptr) { return; }
        nonTerms.push_back(nonTermimal);
    }
    nonTermimal->refInc();
    nonTermimal->setName(nonTerminalName);
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
