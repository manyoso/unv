#ifndef options_h
#define options_h

#include <QtCore>

class Options {
public:
    static Options* instance();

    void parseCommandLine();

    QStringList files() const { return m_files; }
    QStringList includeDirs() const { return m_includeDirs; }
    int errorLimit() const { return m_errorLimit; }
    QString outputFile() const { return m_outputFile; }
    QString outputType() const { return m_outputType; }
    bool readFromStdin() const { return m_readFromStdin; }

private:
    Options();
    ~Options();

    QStringList m_files;
    QStringList m_includeDirs;
    int m_errorLimit;
    QString m_outputFile;
    QString m_outputType;
    bool m_readFromStdin;
};

#endif // options_h
