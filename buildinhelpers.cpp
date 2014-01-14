/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: BSD
 */

#include "buildinhelpers.h"

namespace cpptl {

Value include(TemplateEngine &engine, const Value &context, const Value &args)
{
    if( args.size() > 0 )
    {
        const std::string &fileName = args[0].toString();
        Template subTempl = engine.templFile(fileName);
        return subTempl.render( context );
    }
    else
    {
        assert( false );
        return Value();
    }
}

} // namespace cpptl
