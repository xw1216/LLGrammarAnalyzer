#include "nonterminal.h"

NonTerminal::NonTerminal()
{

}

Symbol::Type NonTerminal::getType() const
{
    return Symbol::Type::NONTERMINAL;
}

bool NonTerminal::isBlank() const
{
    return false;
}

void NonTerminal::setIndexOfProd(int index)
{
    indexOfProd = index;
}

int NonTerminal::getIndexOfProd() const
{
    return indexOfProd;
}
