#include "filesources.h"

#include "sourcebuffer.h"
#include "options.h"

FileSources* FileSources::instance()
{
    static FileSources* _instance = 0;
    if (!_instance)
        _instance = new FileSources;
    return _instance;
}

SourceBuffer* FileSources::sourceBuffer(const QString& name)
{
    QFileInfo info(name);
    if (info.exists())
        return sourceBuffer(info);

    QStringList dirs;
    dirs << Options::instance()->includeDirs();

    foreach (QString dir, dirs) {
        QFileInfo info(dir + QDir::separator() + name);
        if (info.exists())
            return sourceBuffer(info);
    }

    return 0;
}

SourceBuffer* FileSources::sourceBuffer(const QFileInfo& info)
{
    if (m_sourceBuffers.contains(info.absoluteFilePath()))
        return m_sourceBuffers.value(info.absoluteFilePath()).data();

    QString fileContents;
    QFile file(info.absoluteFilePath());
    if (file.open(QFile::ReadOnly)) {
        QTextStream in(&file);
        fileContents = in.readAll();
        file.close();
    } else
        return 0;

    SourceBuffer* buffer = new SourceBuffer(fileContents, info.fileName());
    m_sourceBuffers.insert(info.absoluteFilePath(), QSharedPointer<SourceBuffer>(buffer));
    return buffer;
}

FileSources::FileSources()
{
}

FileSources::~FileSources()
{
}
