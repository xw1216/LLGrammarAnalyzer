#include "symbol.h"

Symbol::Symbol()
{

}

Symbol::Type Symbol::getType() const
{
    return Symbol::Type::BLANK;
}

bool Symbol::isBlank() const
{
    return true;
}

void Symbol::setName(const QString &name)
{
    this->name = name;
}

QString Symbol::getName() const
{
    return this->name;
}

void Symbol::refInc()
{
    refCnt++;
}

void Symbol::refDec()
{
    refCnt--;
}

int Symbol::getRefCnt() const
{
    return refCnt;
}

bool Symbol::canDestroy() const
{
    if(refCnt == 0) {
        return true;
    } else {
        return false;
    }
}

bool Symbol::operator == (const Symbol & lhs) const {
    if(this->getType() != lhs.getType()) {
        return false;
    } else if(this->getName() != lhs.getName()) {
        return false;
    } else {
        return true;
    }
}
