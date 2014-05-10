#ifndef filesources_h
#define filesources_h

#include <QtCore>

class SourceBuffer;

class FileSources {
public:
    static FileSources* instance();

    SourceBuffer* sourceBuffer(const QString& fileName);

private:
    SourceBuffer* sourceBuffer(const QFileInfo&);

    FileSources();
    ~FileSources();

    QHash<QString, QSharedPointer<SourceBuffer> > m_sourceBuffers;
};

#endif // filesources_h
