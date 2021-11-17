#ifndef TERMINAL_H
#define TERMINAL_H

#include <QStringList>

#include "symbol.h"

class Terminal : public Symbol
{
public:
    Terminal();

    Type getType() const  override;
    bool isBlank() const override;
    static bool isRecognized(QString name);
    static int termIndex(QString name);

public:
    static const QStringList terminalList;
};

#endif // TERMINAL_H
