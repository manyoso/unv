#ifndef output_h
#define output_h

#include <QtCore>

class SourceBuffer;

class Output {
public:
    Output(SourceBuffer*);

    /*!
     * \brief writes the LLVM IR to the specified output in Options
     */
    void write(const QString&);

private:
    SourceBuffer* m_source;
};

#endif // output_h
