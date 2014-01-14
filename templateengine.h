/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: BSD
 */

#ifndef CPPTL_TEMPLATEENGINE_H
#define CPPTL_TEMPLATEENGINE_H

#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

#include "template.h"
#include "value.h"

namespace cpptl {

class TemplateEngineImpl;

class TemplateEngine {
public:
    typedef boost::function<Value(const Value &, const Value &)> Helper;

    TemplateEngine();
    ~TemplateEngine();

    Template templ(const std::string &text);
    Template templFile(const std::string &fileName);

    bool hasHelper(const std::string &name);

    void registerHelper(const std::string &name, const Helper &helper);
    Value callHelper(const std::string &name, const Value &context, const Value &args) const;

private:
    boost::scoped_ptr<TemplateEngineImpl> pimpl;
};

} // namespace cpptl

#endif // CPPTL_TEMPLATEENGINE_H
