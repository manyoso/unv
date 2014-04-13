#ifndef visitor_h
#define visitor_h

#include <QtCore>

#include "ast.h"

struct Visitor {
    void walk(Node& node) { node.walk(*this); }
    virtual void begin(Node&) = 0;
    virtual void end(Node&) = 0;
};

#endif // visitor_h
