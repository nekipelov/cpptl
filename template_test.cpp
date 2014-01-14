/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: BSD
 */

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <stdio.h>
#include <string>
#include <boost/bind.hpp>

#include "value.h"
#include "template.h"
#include "templateengine.h"

using namespace cpptl;

BOOST_AUTO_TEST_SUITE( templater )

BOOST_AUTO_TEST_CASE( templater_only_html )
{
    TemplateEngine engine;

    {
        std::string templ = "<p>test</p>";
        BOOST_CHECK( engine.templ(templ).render() == templ );
    }

    {
        std::string templ = "<p>email@@example.com</p>";
        std::string expected = "<p>email@example.com</p>";
        BOOST_CHECK( engine.templ(templ).render() == expected );
    }
}

BOOST_AUTO_TEST_CASE( templater_variables )
{
    TemplateEngine engine;
    Value context{ Value::ObjectTag() };

    context["test"] = "foo";
    context["firstname"] = "Foo";
    context["lastname"] = "Bar";

    std::string templ1 = "<p>@test</p>";
    BOOST_CHECK( engine.templ(templ1).render(context) == "<p>foo</p>" );

    std::string templ2 = "<p>@firstname - @lastname</p>";
    BOOST_CHECK( engine.templ(templ2).render(context) == "<p>Foo - Bar</p>" );

    std::string templ3 = "<p>@name - @email</p>";
    BOOST_CHECK( engine.templ(templ3).render(
        {{"name", "Alex"}, {"email", "alex@nekipelov.net"}}) ==
        "<p>Alex - alex@nekipelov.net</p>");
}

BOOST_AUTO_TEST_CASE( templater_sub_variables )
{
    TemplateEngine engine;

    {
        const std::string templ = "<p>@{people.firstname} - @{people.lastname}</p>";
        const std::string expected = "<p>Foo - Bar</p>";

        Value values{ Value::ObjectTag() };
        Value people{ Value::ObjectTag() };

        people["firstname"] = "Foo";
        people["lastname"] = "Bar";

        values["people"] = people;


        BOOST_CHECK( engine.templ(templ).render(values) == expected );
    }
    {
        const std::string templ = "<p>@{people.firstname.text} - @{people.lastname.text}</p>";
        const std::string expected = "<p>Foo - Bar</p>";

        Value values{ Value::ObjectTag() };
        Value people{ Value::ObjectTag() };
        Value text1{ Value::ObjectTag() };
        Value text2{ Value::ObjectTag() };

        text1["text"] = "Foo";
        text2["text"] = "Bar";

        people["firstname"] = text1;
        people["lastname"] = text2;

        values["people"] = people;

        BOOST_CHECK( engine.templ(templ).render(values) == expected );
    }
}

BOOST_AUTO_TEST_CASE( templater_for_list )
{
    TemplateEngine engine;

    const std::string templ = R"(
            <ul>
                @for(item in list) {
                    <li>@item</li>
                }
            </ul>)";
    const std::string expected = R"(
            <ul>
                    <li>Adam</li>
                    <li>Bert</li>
                    <li>John</li>
                    <li>Martin</li>
            </ul>)";

    {
        Value values{ Value::ObjectTag() };
        Value items{ Value::ArrayTag() };

        items.append( "Adam" );
        items.append( "Bert" );
        items.append( "John" );
        items.append( "Martin" );

        values["list"] = items;

        BOOST_CHECK( engine.templ(templ).render(values) == expected );
    }
}

BOOST_AUTO_TEST_CASE( templater_helpers )
{
    TemplateEngine engine;
    Value values{Value::ObjectTag()};

    values["value"] = 123.34447;

    auto helloWorld = [](const Value & /*context*/, const Value &args) {
        BOOST_CHECK( args.type() == Value::Array );
        BOOST_CHECK( args.size() == 0 );
        return std::string("Hello, world!");
    };

    auto printDouble = [](const Value & /*context*/, const Value &args) {
        BOOST_CHECK( args.type() == Value::Array );
        BOOST_CHECK( args.size() == 1 );
        BOOST_CHECK( args[0].type() == Value::Double );

        char buf[256];
        snprintf( buf, sizeof(buf), "%.2f", args[0].toDouble());
        return std::string(buf);
    };

    auto printInteger = [](const Value & /*context*/, const Value &args) {
        BOOST_CHECK( args.type() == Value::Array );
        BOOST_CHECK( args.size() == 1 );
        BOOST_CHECK( args[0].type() == Value::Double );

        char buf[256];
        snprintf( buf, sizeof(buf), "%d", args[0].toInt());
        return std::string(buf);
    };

    auto multiply = [](const Value & /*context*/, const Value &args) {
        BOOST_CHECK( args.type() == Value::Array );
        BOOST_CHECK( args.size() == 2 );
        BOOST_CHECK( args[0].type() == Value::Integer );
        BOOST_CHECK( args[1].type() == Value::Integer );

        char buf[256];
        int value = args[0].toInt() * args[1].toInt();

        snprintf( buf, sizeof(buf), "%d", value);
        return std::string(buf);
    };

    auto printString = [](const Value & /*context*/, const Value &args) {
        BOOST_CHECK( args.type() == Value::Array );
        BOOST_CHECK( args.size() == 2 );
        BOOST_CHECK( args[0].type() == Value::String );
        BOOST_CHECK( args[1].type() == Value::String );

        std::string result = args[0].toString();
        result += args[1].toString();
        return result;
    };

    engine.registerHelper("printDouble", printDouble);
    engine.registerHelper("printInteger", printInteger);
    engine.registerHelper("helloWorld", helloWorld);
    engine.registerHelper("multiply", multiply);
    engine.registerHelper("printString", printString);
    {
        std::string templ = "<p>@printDouble(value)</p>";
        BOOST_CHECK( engine.templ(templ).render(values) == "<p>123.34</p>" );
    }
    {
        std::string templ = "<p>@printInteger(value)</p>";
        BOOST_CHECK( engine.templ(templ).render(values) == "<p>123</p>" );
    }
    {
        std::string templ = "<p>@helloWorld()</p>";
        BOOST_CHECK( engine.templ(templ).render(values) == "<p>Hello, world!</p>" );
    }
    {
        std::string templ = "<p>@{helloWorld()}</p>";
        BOOST_CHECK( engine.templ(templ).render(values) == "<p>Hello, world!</p>" );
    }
    {
        std::string templ = "<p>@multiply(10, 20)</p>";
        BOOST_CHECK( engine.templ(templ).render(values) == "<p>200</p>" );
    }
    {
        std::string templ = "<p>@printString(\"Hello, \", \"world!\")</p>";
        BOOST_CHECK( engine.templ(templ).render(values) == "<p>Hello, world!</p>" );
    }

    {
        std::string templ = R"(
                <p>@printString( printString("10*", "20="), multiply(10, 20))</p>
        )";

        std::string expected = R"(
                <p>10*20=200</p>
        )";

        BOOST_CHECK( engine.templ(templ).render(values) == expected );
    }
}

BOOST_AUTO_TEST_CASE( templater_conditions_if )
{
    TemplateEngine engine;

    const std::string src = R"(
            @if( value1 ) {
                <p>if block</p>
            }
            else if( value2 ) {
                <p>else if block</p>
            }
            else {
                <p>else block</p>
            }
        )";

    {
        const std::string expected = R"(
                <p>if block</p>
        )";

        Value values{Value::ObjectTag()};
        values["value1"] = true;
        values["value2"] = false;

        BOOST_CHECK( engine.templ(src).render(values) == expected );
    }

    {
        const std::string expected = R"(
                <p>else if block</p>
        )";

        Value values{Value::ObjectTag()};
        values["value1"] = false;
        values["value2"] = true;

        BOOST_CHECK( engine.templ(src).render(values) == expected );
    }

    {
        const std::string expected = R"(
                <p>else block</p>
        )";

        Value values{Value::ObjectTag()};
        values["value1"] = false;
        values["value2"] = false;

        BOOST_CHECK( engine.templ(src).render(values) == expected );
    }
}

BOOST_AUTO_TEST_CASE( templater_conditions_unless )
{
    TemplateEngine engine;
    const std::string src = R"(
            @unless( value ) {
                <p>unless block</p>
            }
            else {
                <p>else block</p>
            }
        )";

    {
        const std::string expected = R"(
                <p>unless block</p>
        )";

        Value values{Value::ObjectTag()};
        values["value"] = false;

        BOOST_CHECK( engine.templ(src).render(values) == expected );
    }
    {
        const std::string expected = R"(
                <p>else block</p>
        )";

        Value values{Value::ObjectTag()};
        values["value"] = true;


        BOOST_CHECK( engine.templ(src).render(values) == expected );

    }
}

BOOST_AUTO_TEST_CASE( templater_members )
{
    TemplateEngine engine;

    Value values{Value::ObjectTag()};

    auto returnObject = [](const Value & /*context*/, const Value &args) {
        BOOST_CHECK( args.type() == Value::Array );
        BOOST_CHECK( args.size() == 0 );

        Value obj{Value::ObjectTag()};
        obj["name"] = "Foo";

        return obj;
    };

    engine.registerHelper("returnObject", returnObject);
    {
        std::string templ = "<p>@{returnObject().name}</p>";
        BOOST_CHECK( engine.templ(templ).render(values) == "<p>Foo</p>" );
    }

    {
        Value array{Value::ArrayTag()};
        values["array"] = array;

        std::string templ = R"(
                @if( array.empty? ) {
                    <p>Array is empty</p>
                }
                else {
                    <p>Array not empty</p>
                }
            )";
        std::string expected = R"(
                    <p>Array is empty</p>
            )";

        BOOST_CHECK( engine.templ(templ).render(values) == expected );
    }

    {
        Value array{Value::ArrayTag()};
        values["array"] = array;

        array.append( "item1" );
        array.append( "item2" );
        array.append( "item3" );

        std::string templ = R"(
                @if( array.empty? ) {
                    <p>Array is empty</p>
                }
                else {
                    <p>Array size: @{array.size}</p>
                }
            )";

        std::string expected = R"(
                    <p>Array size: 3</p>
            )";

        BOOST_CHECK( engine.templ(templ).render(values) == expected );
    }

    {
        Value array{Value::ArrayTag()};

        array[0] = 0;
        array[1] = 1;
        array[2] = 2;

    }
}

BOOST_AUTO_TEST_CASE( templater_objects_args )
{
    TemplateEngine engine;

    Value values{Value::ObjectTag()};

    auto returnObject = [](const Value &, const Value &args) {
        BOOST_CHECK( args.type() == Value::Array );
        BOOST_CHECK( args.size() == 1 );
        BOOST_CHECK( args[0].type() == Value::Object );

        Value obj = args[0];

        BOOST_CHECK( obj.size() == 3 );

        BOOST_CHECK( obj["string"].type() == Value::String );
        BOOST_CHECK( obj["string"].toString() == "hello" );

        BOOST_CHECK( obj["empty"].type() == Value::Object );
        BOOST_CHECK( obj["empty"].isEmpty() == true );

        BOOST_CHECK( obj["integer"].type() == Value::Integer );
        BOOST_CHECK( obj["integer"].toInt() == 10 );

        return Value("Foo");
    };

    engine.registerHelper("returnObject", returnObject);
    {
        std::string templ = "<p>@returnObject( {string: \"hello\", empty: {}, integer: 10} )</p>";
        BOOST_CHECK( engine.templ(templ).render(values) == "<p>Foo</p>" );
    }
}

BOOST_AUTO_TEST_CASE( templater_expressions )
{
    TemplateEngine engine;

    Value values{Value::ObjectTag()};

    values["string"] = "Hello, ";

    {
        std::string templ = "<p>@{string + \"world!\"}</p>";
        BOOST_CHECK( engine.templ(templ).render(values) == "<p>Hello, world!</p>" );
    }
}

BOOST_AUTO_TEST_CASE( templater_inline_if )
{
    TemplateEngine engine;

    Value values{Value::ObjectTag()};

    values["expr1"] = "true";
    values["expr2"] = "false";

    {
        values["condition"] = true;
        std::string templ = "<p>@{condition ? expr1 : expr2 }</p>";
        BOOST_CHECK( engine.templ(templ).render(values) == "<p>true</p>" );
    }
    {
        values["condition"] = false;
        std::string templ = "<p>@{condition ? expr1 : expr2 }</p>";
        BOOST_CHECK( engine.templ(templ).render(values) == "<p>false</p>" );
    }
}

BOOST_AUTO_TEST_CASE( templater_escape )
{
    TemplateEngine engine;

    {
        Value values{Value::ObjectTag()};
        values["string"] = "<b>Hello</b>";

        std::string templ = "<p>@string</p>";
        BOOST_CHECK( engine.templ(templ).render(values) == "<p>&lt;b&gt;Hello&lt;/b&gt;</p>" );
    }
}


BOOST_AUTO_TEST_SUITE_END()
