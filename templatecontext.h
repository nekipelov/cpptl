/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: BSD
 */

#ifndef CPPTL_TEMPLATECONTEXT_H
#define CPPTL_TEMPLATECONTEXT_H

#include <string>

namespace cpptl {
    class Value;
    class Template;
} // namespace cpptl

struct TemplateContext {
    const std::string &templ;
    const cpptl::Value &context;
    const cpptl::Template &caller;
};

#endif // CPPTL_TEMPLATECONTEXT_H
