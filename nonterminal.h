#ifndef NONTERMINAL_H
#define NONTERMINAL_H

#include <QSet>
#include "symbol.h"

class GrammarAnalyzer;

class NonTerminal : public Symbol
{
    friend class GrammarAnalyzer;
public:
    NonTerminal();

    Type getType() const override;
    bool isBlank() const override;

    void setIndexOfProd(int index);
    int getIndexOfProd() const;

private:
    int indexOfProd = -1;
    QSet<Symbol*> first;
    QSet<Symbol*> follow;
    QSet<Symbol*> synch;
};

#endif // NONTERMINAL_H
