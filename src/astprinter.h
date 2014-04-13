#ifndef astprinter_h
#define astprinter_h

#include <QtCore>
#include "visitor.h"

class ASTPrinter : public Visitor {
public:
    ASTPrinter();
    ~ASTPrinter();
    virtual void begin(Node&);
    virtual void end(Node&);

private:
    QString indent() const;
    int m_scope;
};

#endif // astprinter_h
