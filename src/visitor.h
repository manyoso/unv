#ifndef visitor_h
#define visitor_h

#include <QtCore>

#include "ast.h"

struct Visitor {
    void walk(Node& node) { node.walk(*this); }
    virtual void visit(Node&) = 0;
};

#endif // visitor_h
