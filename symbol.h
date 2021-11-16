#ifndef SYMBOL_H
#define SYMBOL_H

#include <QObject>
#include <QHash>
#include <QChar>
#include <QString>
#include <QHashFunctions>

class Symbol
{
public:
    Symbol();

    enum class Type {BLANK = 0, TERMINAL = 1, NONTERMINAL = 2};

    virtual Type getType() const;
    virtual bool isBlank() const;

    void setName(const QString & name);
    QString getName() const;

    void refInc();
    void refDec();
    int getRefCnt() const;
    bool canDestroy() const;

    bool operator == (const Symbol & lhs) const ;

protected:
    QString name;
    int refCnt = 0;
};

#endif // SYMBOL_H
