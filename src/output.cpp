#include "output.h"
#include "astprinter.h"
#include "options.h"
#include "sourcebuffer.h"

static void error(const QString& err)
{
    QTextStream out(stderr);
    out << err << '\n';
    out.flush();
    exit(EXIT_FAILURE);
}

Output::Output(SourceBuffer* source)
    : m_source(source)
{
}

void Output::write(const QString& llvmIR)
{
    QString file = Options::instance()->outputFile();
    QString type = Options::instance()->outputType();
    if (type == "ast") {
        QTextStream out(stdout);
        QFile f(file);
        if (!file.isEmpty()) {
            if (f.open(QIODevice::WriteOnly))
                out.setDevice(&f);
        }
        f.close();
        ASTPrinter printer(m_source, &out);
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
                error(QString("can not write to file $0").arg(file));
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
        if (!config.waitForFinished())
            error("could not find llvm-config tool");
        QString bindir = config.readAllStandardOutput();
        bindir.remove('\n');

        QProcess llc;
        llc.setProgram(bindir + QDir::separator() + "llc");
        llc.setArguments(QStringList() << "-filetype=obj");
        llc.start();
        if (!llc.waitForStarted())
            error("could not start llc tool");

        llc.write(llvmIR.toLatin1());

        llc.waitForBytesWritten();
        llc.closeWriteChannel();

        if (!llc.waitForFinished())
            error("llc tool crashed");

        QByteArray err = llc.readAllStandardError();
        if (!err.isEmpty())
            error("llc tool exited with error");

        QByteArray output = llc.readAllStandardOutput();

        QFile f(file);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(output);
            f.flush();
            f.close();
        } else {
            error(QString("can not write to file $0").arg(file));
        }
    }
}
