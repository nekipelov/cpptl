/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: BSD
 */

#include <list>

#include "template.h"
#include "templateengine.h"
#include "templateasttree.h"
#include "templatecontext.h"

extern "C" {
    Node *getAstTree(const char *templ);
}

namespace cpptl {

class TemplateImpl {
public:
    TemplateImpl(TemplateEngine &engine, const std::string &templ);
    ~TemplateImpl();

    std::string render(const TemplateContext &context) const;
    TemplateEngine &engine;
    const std::string templ;
    mutable Node *node;
};

Template::Template(TemplateEngine &engine, const std::string &templ)
{
    pimpl.reset( new TemplateImpl(engine, templ) );
}

Template::~Template()
{
}

std::string Template::render(const Value &context) const
{
    TemplateContext ctx = {pimpl->templ, context, *this};
    return pimpl->render(ctx);
}

std::string Template::render(const std::map<std::string, Value> &context) const
{
    Value values(Value::Object);
    std::map<std::string, Value>::const_iterator it = context.begin();
    std::map<std::string, Value>::const_iterator end = context.end();

    for(; it != end; ++it)
        values[it->first] = it->second;

    TemplateContext ctx = {pimpl->templ, values, *this};
    return pimpl->render(ctx);
}

const TemplateEngine &Template::engine() const
{
    return pimpl->engine;
}

TemplateImpl::TemplateImpl(TemplateEngine &engine, const std::string &templ)
    : engine(engine), templ(templ), node(NULL)
{
}

TemplateImpl::~TemplateImpl()
{
    if( node )
        freeNodes(node);
}

std::string TemplateImpl::render(const TemplateContext &context) const
{
    if( !node )
        node = getAstTree(templ.c_str());

    if( node )
        return traverserTreeNodes(node, context);
     else
        return std::string("template syntax error");
}

} // namespace cpptl
