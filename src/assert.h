#ifndef assert_h
#define assert_h

#include <QtCore>
#include <signal.h>

static inline void assert(bool b)
{
    if (!b) {
        QTextStream out(stderr);
#ifdef Q_OS_UNIX
        out << "\033[91mASSERT\033[39m: Oops, this is really embarrassing...\n";
#else
        out << "ASSERT: Oops, this is really embarrassing...\n";
#endif
        out.flush();
        raise(SIGSEGV);
    }
}

#endif // assert_h
