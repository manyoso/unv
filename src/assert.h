#ifndef assert_h
#define assert_h

#include <QtCore>

static inline bool assert(bool b)
{
    if (!b) {
        QTextStream out(stderr);
#ifdef Q_OS_UNIX
        out << "\033[91mASSERT\033[39m: Oops, this is really embarrassing...\n";
#else
        out << "ASSERT: Oops, this is really embarrassing...\n";
#endif
        out.flush();
        exit(EXIT_FAILURE);
    }
    return !b;
}

#endif // assert_h
