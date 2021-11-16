#ifndef PRODUCTION_H
#define PRODUCTION_H

#include <QVector>

#include "terminal.h"
#include "nonterminal.h"

class Production
{
    friend class GrammarAnalyzer;
public:
    Production();

    void setLhs(NonTerminal* lhs);
    NonTerminal* getLhs() const;
    void insertRhs(Symbol* rhs);
    void newProduction();
    void insertProduction(QVector<Symbol*> & prod);
    bool isLeftRecursion();
    bool isSharedPrefix();

private:
    NonTerminal* lhs = nullptr;
    QVector<QVector<Symbol*>> rhs;
};

#endif // PRODUCTION_H
