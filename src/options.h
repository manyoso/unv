#ifndef options_h
#define options_h

#include <QtCore>

class Options {
public:
    static Options* instance();

    void parseCommandLine();

    QStringList files() const { return m_files; }
    int errorLimit() const { return m_errorLimit; }
    QString outputFile() const { return m_outputFile; }
    QString outputType() const { return m_outputType; }

private:
    Options();
    ~Options();

    QStringList m_files;
    int m_errorLimit;
    QString m_outputFile;
    QString m_outputType;
};

#endif // options_h
