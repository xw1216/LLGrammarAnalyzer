#include "production.h"

Production::Production()
{

}

void Production::setLhs(NonTerminal* lhs)
{
    this->lhs = lhs;
}

NonTerminal *Production::getLhs() const
{
    return this->lhs;
}

void Production::insertRhs(Symbol* rhs)
{
    this->rhs[this->rhs.size() - 1].append(rhs);
}

void Production::newProduction()
{
    this->rhs.append(QVector<Symbol*>());
}

void Production::insertProduction(QVector<Symbol *> &prod)
{
    this->rhs.push_back(prod);
}

bool Production::isLeftRecursion()
{
    for(int i = 0; i < this->rhs.size(); i++) {
        if(this->rhs[i].size() <= 0) { continue; }
        if(this->lhs->getName() == this->rhs[i][0]->getName()) {
            return true;
        }
    }
    return false;
}

bool Production::isSharedPrefix()
{
    for(int i = 0; i < this->rhs.size(); i++) {
        for(int j = i + 1; j < this->rhs.size(); j++) {
            if(this->rhs[i][0]->getName() == this->rhs[j][0]->getName()) {
                return true;
            }
        }
    }
    return false;
}
