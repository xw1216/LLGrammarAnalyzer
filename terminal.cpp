#include "terminal.h"

const QStringList Terminal::terminalList = {
        "blank", "void", "int", "long", "float", "double","bool",
        "string", "if", "elif", "else", "return","while", "for",
        "break", "continue", "switch","case", "default",
        "true", "false", "id", "integer", "floatnum", "str",

        "assign", "plus", "sub", "mul", "div", "mod", "eq",
        "gre", "geq", "les", "leq", "neq", "and", "or", "not",
        "smc", "cma", "lpar", "rpar","lbrc", "rbrc", "lsbrc",
        "rsbrc","colon"
    };

Terminal::Terminal()
{

}

Symbol::Type Terminal::getType() const
{
    return Symbol::Type::TERMINAL;
}

bool Terminal::isBlank() const
{
    if(this->name.isEmpty()) {
        return true;
    } else {
        return false;
    }
}

bool Terminal::isRecognized(QString name)
{
    if(terminalList.contains(name)) {
        return true;
    } else {
        return false;
    }
}

int Terminal::termIndex(QString name)
{
    return terminalList.indexOf(name);
}

