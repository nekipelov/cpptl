/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: BSD
 */

#ifndef CPPTL_BUILDINHELPERS_H
#define CPPTL_BUILDINHELPERS_H

#include "templateengine.h"

namespace cpptl {

Value include(TemplateEngine &engine, const Value &context, const Value &fileName);

} // namespace cpptl

#endif // CPPTL_BUILDINHELPERS_H
