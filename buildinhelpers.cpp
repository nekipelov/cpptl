/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: BSD
 */

#include <boost/algorithm/string/replace.hpp>
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

Value rawHtml(const Value &, const Value &html)
{
    if( html.size() == 1 )
    {
        if( html[0].type() == Value::String)
        {
            std::string s = html[0].toString();

            boost::replace_all(s, "&amp;", "&");
            boost::replace_all(s, "&gt;", ">");
            boost::replace_all(s, "&lt;", "<");
            boost::replace_all(s, "&quot;", "\"");

            return Value(s, Value::UnsafeStringTag());
        }
        else
        {
            return html[0];
        }
    }
    else
    {
        return Value("");
    }
}

} // namespace cpptl
