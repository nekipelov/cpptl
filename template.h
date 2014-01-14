/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: BSD
 */

#ifndef CPPTL_TEMPLATE_H
#define CPPTL_TEMPLATE_H

#include <map>
#include <boost/shared_ptr.hpp>

#include "value.h"

namespace cpptl {

class TemplateEngine;
class TemplateImpl;

class Template {
public:
    Template(TemplateEngine &engine, const std::string &templ);
    ~Template();

    std::string render(const Value &context = Value()) const;
    std::string render(const std::map<std::string, Value> &context) const;
    const TemplateEngine &engine() const;

private:
    boost::shared_ptr<TemplateImpl> pimpl;
};

} // namespace cpptl

#endif // CPPTL_TEMPLATE_H
