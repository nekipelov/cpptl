/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: BSD
 */

#include <map>
#include <fstream>
#include <boost/bind.hpp>

#include "templateengine.h"

namespace cpptl {

class TemplateEngineImpl {
public:

    std::map<std::string, TemplateEngine::Helper> helpers;
    std::map<std::string, Template> cache;
};

TemplateEngine::TemplateEngine()
{
    pimpl.reset(new TemplateEngineImpl);
}

TemplateEngine::~TemplateEngine()
{
}

Template TemplateEngine::templ(const std::string &text)
{
    return Template(*this, text);
}

Template TemplateEngine::templFile(const std::string &fileName)
{
    std::map<std::string, Template>::iterator it = pimpl->cache.find(fileName);

    if( it != pimpl->cache.end() )
        return it->second;

    std::ifstream ifs(fileName.c_str());

    if( ifs )
    {
        ifs.seekg(0, std::ios::end);
        std::ifstream::pos_type fileSize = ifs.tellg();
        ifs.seekg(0, std::ios::beg);

        std::string content;
        content.resize(fileSize);

        ifs.read(&content[0], fileSize);

        Template templ(*this, content);

        pimpl->cache.insert( std::make_pair(fileName, templ) );

        return templ;
    }
    else
    {
        std::cerr << "Can`t open file" << fileName << std::endl;
    }

    return Template(*this, std::string());
}

bool TemplateEngine::hasHelper(const std::string &name)
{
    return pimpl->helpers.find(name) != pimpl->helpers.end();
}

void TemplateEngine::registerHelper(const std::string &name, const Helper &handler)
{
    pimpl->helpers[name] = handler;
}

Value TemplateEngine::callHelper(const std::string &name, const Value &context, const Value &args) const
{
    std::map<std::string, TemplateEngine::Helper>::iterator it = pimpl->helpers.find(name);

    if( it != pimpl->helpers.end() )
    {
        return it->second(context, args);
    }
    else
    {
        std::cerr << "helper \"" << name << "\" not found";
        return std::string();
    }
}

} // namespace cpptl
