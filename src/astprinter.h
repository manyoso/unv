#ifndef astprinter_h
#define astprinter_h

#include <QtCore>
#include "visitor.h"

struct ASTPrinter : public Visitor {
    virtual void visit(Node&);
};

#endif // astprinter_h
