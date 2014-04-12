#ifndef options_h
#define options_h

#include <QtCore>

class Options {
public:
    static Options* instance();

    void parseCommandLine();

    int errorLimit() const { return m_errorLimit; }
    void setErrorLimit(int limit) { m_errorLimit = limit; }

private:
    Options();
    ~Options();

    int m_errorLimit;
};

#endif // options_h
