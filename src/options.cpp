#include "options.h"

Options* Options::instance()
{
    static Options* _instance = 0;
    if (!_instance)
        _instance = new Options;
    return _instance;
}

Options::Options()
    : m_errorLimit(20)
    , m_readFromStdin(false)
{
}

Options::~Options()
{
}

void Options::parseCommandLine()
{
    QCommandLineParser parser;
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("files", "Files to compile.", "[files...]");

    QCommandLineOption include(QStringList() << "i" << "include", "Include directories.", "include", "");
    parser.addOption(include);

    QCommandLineOption errorLimit("error-limit", "Stop after N errors. [Default: 20]", "N", "20");
    parser.addOption(errorLimit);

    QCommandLineOption outputFile(QStringList() << "o" << "out",
                                  "Output to file or stdout if empty.", "file", "");
    parser.addOption(outputFile);

    QCommandLineOption outputType(QStringList() << "e" << "emit",
                                  "Specify the type of output. [Default: obj]\n   type=obj|llvm|ast", "type", "obj");
    parser.addOption(outputType);

    QCommandLineOption readFromStdin("stdin", "Read from stdin.");
    parser.addOption(readFromStdin);

    parser.process(*QCoreApplication::instance());

    m_files = parser.positionalArguments();
    m_includeDirs = parser.values(include);
    m_errorLimit = parser.value(errorLimit).toInt();
    m_outputFile = parser.value(outputFile);
    m_outputType = parser.value(outputType);
    if (m_outputType != "obj" && m_outputType != "llvm" && m_outputType != "ast")
        m_outputType = "obj";
    m_readFromStdin = parser.isSet(readFromStdin);

    if (m_files.isEmpty() && !m_readFromStdin)
        parser.showHelp();
}
