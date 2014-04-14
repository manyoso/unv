#include "output.h"
#include "astprinter.h"
#include "options.h"
#include "sourcebuffer.h"

Output::Output(SourceBuffer* source)
    : m_source(source)
{
}

void Output::write(const QString& llvmIR)
{
    QString file = Options::instance()->outputFile();
    QString type = Options::instance()->outputType();
    if (type == "ast") {
        ASTPrinter printer(m_source);
        printer.walk();
    } else if (type == "llvm") {
        if (file.isEmpty()) {
            QTextStream out(stdout);
            out << llvmIR;
            out.flush();
        } else {
            QFile f(file);
            if (f.open(QIODevice::WriteOnly)) {
                QTextStream out(&f);
                out << llvmIR;
                out.flush();
                f.close();
            } else {
                m_source->error(Token(),
                                QString("can not write to file $0").arg(file),
                                SourceBuffer::Fatal);
            }
        }
    } else if (type == "obj") {
        if (file.isEmpty()) {
            QFileInfo info(m_source->name());
            file = info.dir().path() + QDir::separator() + info.baseName() + ".o";
        }

        QProcess config;
        config.setProgram("llvm-config");
        config.setArguments(QStringList() << "--bindir");
        config.start(QIODevice::ReadOnly);
        if (!config.waitForFinished()) {
            m_source->error(Token(),
                            "could not find llvm-config tool",
                            SourceBuffer::Fatal);
        }
        QString bindir = config.readAllStandardOutput();
        bindir.remove('\n');

        QProcess llc;
        llc.setProgram(bindir + QDir::separator() + "llc");
        llc.setArguments(QStringList() << "-filetype=obj");
        llc.start();
        if (!llc.waitForStarted()) {
            m_source->error(Token(),
                            "could not start llc tool",
                            SourceBuffer::Fatal);
        }

        llc.write(llvmIR.toLatin1());

        llc.waitForBytesWritten();
        llc.closeWriteChannel();

        if (!llc.waitForFinished()) {
            m_source->error(Token(),
                            "llc tool crashed",
                            SourceBuffer::Fatal);
        }

        QByteArray error = llc.readAllStandardError();
        if (!error.isEmpty()) {
            m_source->error(Token(),
                            "llc tool exited with error",
                            SourceBuffer::Fatal);
        }

        QByteArray output = llc.readAllStandardOutput();

        QFile f(file);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(output);
            f.flush();
            f.close();
        } else {
            m_source->error(Token(),
                            QString("can not write to file $0").arg(file),
                            SourceBuffer::Fatal);
        }
    }
}
