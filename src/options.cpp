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
{
}

Options::~Options()
{
}
