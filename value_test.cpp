/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: BSD
 */

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>

#include "value.h"

using namespace cpptl;

BOOST_AUTO_TEST_SUITE(value)

BOOST_AUTO_TEST_CASE(value_nums)
{
    Value v1( static_cast<int64_t>(5) );
    Value v2( static_cast<uint64_t>(5) );
    Value v3( static_cast<double>(5.0) );
    Value v4( static_cast<int>(5) );
    Value v5( static_cast<unsigned int>(5) );

    BOOST_VERIFY(v1.toInt() == 5);
    BOOST_VERIFY(v2.toInt() == 5);
    BOOST_VERIFY(v3.toInt() == 5);
    BOOST_VERIFY(v4.toInt() == 5);
    BOOST_VERIFY(v5.toInt() == 5);

    BOOST_VERIFY(v1.toUInt() == 5);
    BOOST_VERIFY(v2.toUInt() == 5);
    BOOST_VERIFY(v3.toUInt() == 5);
    BOOST_VERIFY(v4.toUInt() == 5);
    BOOST_VERIFY(v5.toUInt() == 5);

    BOOST_VERIFY(v1.toInt64() == 5);
    BOOST_VERIFY(v2.toInt64() == 5);
    BOOST_VERIFY(v3.toInt64() == 5);
    BOOST_VERIFY(v4.toInt64() == 5);
    BOOST_VERIFY(v5.toInt64() == 5);

    BOOST_VERIFY(v1.toUInt64() == 5);
    BOOST_VERIFY(v2.toUInt64() == 5);
    BOOST_VERIFY(v3.toUInt64() == 5);
    BOOST_VERIFY(v4.toUInt64() == 5);
    BOOST_VERIFY(v5.toUInt64() == 5);

    BOOST_VERIFY(v1.toDouble() == 5.0);
    BOOST_VERIFY(v2.toDouble() == 5.0);
    BOOST_VERIFY(v3.toDouble() == 5.0);
    BOOST_VERIFY(v4.toDouble() == 5.0);
    BOOST_VERIFY(v5.toDouble() == 5.0);


    BOOST_VERIFY( (v1 + v2).toInt() == 10 );
    BOOST_VERIFY( (v2 + v3).toInt() == 10 );
    BOOST_VERIFY( (v3 + v4).toInt() == 10 );
    BOOST_VERIFY( (v1 + v3).toInt() == 10 );
    BOOST_VERIFY( (v1 + v4).toInt() == 10 );
    BOOST_VERIFY( (v2 + v4).toInt() == 10 );
    BOOST_VERIFY( (v5 + v4).toInt() == 10 );
    BOOST_VERIFY( (v1 + v2 + v3 + v4 + v5).toInt() == 25 );

    BOOST_VERIFY( (v1 - v2).toInt() == 0 );
    BOOST_VERIFY( (v2 - v3).toInt() == 0 );
    BOOST_VERIFY( (v3 - v4).toInt() == 0 );
    BOOST_VERIFY( (v1 - v3).toInt() == 0 );
    BOOST_VERIFY( (v1 - v4).toInt() == 0 );
    BOOST_VERIFY( (v2 - v4).toInt() == 0 );
    BOOST_VERIFY( (v2 - v5).toInt() == 0 );
    BOOST_VERIFY( (v1 + v2 - v3 - v4 - v5).toInt() == -5 );

    BOOST_VERIFY( (v1 / v2).toInt() == 1 );
    BOOST_VERIFY( (v2 / v3).toInt() == 1 );
    BOOST_VERIFY( (v3 / v4).toInt() == 1 );
    BOOST_VERIFY( (v1 / v3).toInt() == 1 );
    BOOST_VERIFY( (v1 / v4).toInt() == 1 );
    BOOST_VERIFY( (v2 / v4).toInt() == 1 );
    BOOST_VERIFY( (v2 / v5).toInt() == 1 );
    BOOST_VERIFY( (Value(25) / v4).toInt() == 5 );
    BOOST_VERIFY( (v1 / v2 / v3 / v4 / v5).toInt() == 0 );

    BOOST_VERIFY( (v1 * v2).toInt() == 25 );
    BOOST_VERIFY( (v2 * v3).toInt() == 25 );
    BOOST_VERIFY( (v3 * v4).toInt() == 25 );
    BOOST_VERIFY( (v1 * v3).toInt() == 25 );
    BOOST_VERIFY( (v1 * v4).toInt() == 25 );
    BOOST_VERIFY( (v2 * v4).toInt() == 25 );
    BOOST_VERIFY( (v1 * v5).toInt() == 25 );
    BOOST_VERIFY( (v2 * v5).toInt() == 25 );
    BOOST_VERIFY( (v1 * v2 * v3 * v4 * v5).toInt() == 5*5*5*5*5 );

    BOOST_VERIFY( (v1 + v2) == (v3 + v4) );
    BOOST_VERIFY( v1 == v5 );
    BOOST_VERIFY( v2 == v4 );

    BOOST_VERIFY( (v1 + 1) != v1 );
    BOOST_VERIFY( (v2 + 1) != v2 );
    BOOST_VERIFY( (v3 + 1) != v3 );
    BOOST_VERIFY( (v4 + 1) != v4 );
    BOOST_VERIFY( (v5 + 1) != v4 );
    BOOST_VERIFY( (v1 + v2) != v3 );

    BOOST_VERIFY( v1 > 1 && !(v1 > 10) );
    BOOST_VERIFY( v2 > 1 && !(v2 > 10) );
    BOOST_VERIFY( v3 > 1 && !(v3 > 10) );
    BOOST_VERIFY( v4 > 1 && !(v4 > 10) );

    BOOST_VERIFY( v1 < 10 && !(v1 < 1) );
    BOOST_VERIFY( v2 < 10 && !(v2 < 1) );
    BOOST_VERIFY( v3 < 10 && !(v3 < 1) );
    BOOST_VERIFY( v4 < 10 && !(v4 < 1) );

    BOOST_VERIFY( v1 >= 1 && v1 >= 5 && !(v1 > 50) );
    BOOST_VERIFY( v2 >= 1 && v2 >= 5 && !(v2 > 50) );
    BOOST_VERIFY( v3 >= 1 && v3 >= 5 && !(v3 > 50) );
    BOOST_VERIFY( v4 >= 1 && v4 >= 5 && !(v4 > 50) );

    BOOST_VERIFY( v1 <= 50 && v1 <= 5 && !(v1 <= 1) );
    BOOST_VERIFY( v2 <= 50 && v2 <= 5 && !(v2 <= 1) );
    BOOST_VERIFY( v3 <= 50 && v3 <= 5 && !(v3 <= 1) );
    BOOST_VERIFY( v4 <= 50 && v4 <= 5 && !(v4 <= 1) );
}

BOOST_AUTO_TEST_CASE(value_strings)
{
    Value v1("test");
    Value v2(" ");
    Value v3("string");

    BOOST_VERIFY(v1 == "test");
    BOOST_VERIFY(v2 == " ");
    BOOST_VERIFY(v3 == "string");

    BOOST_VERIFY(v1.toString() == "test");
    BOOST_VERIFY(v2.toString() == " ");
    BOOST_VERIFY(v3.toString() == "string");

    BOOST_VERIFY(v1 + v2 + v3 == "test string");

    Value s1 = v1;
    Value s2 = v2;
    Value s3 = v3;

    BOOST_VERIFY(v1 == s1);
    BOOST_VERIFY(v2 == s2);
    BOOST_VERIFY(v3 == s3);

    BOOST_VERIFY(v1 != s2);
    BOOST_VERIFY(v2 != s3);
    BOOST_VERIFY(v3 != s1);


    v1 = "10";
    v2 = "10.5";
    v3 = "true";
    Value v4 = "false";

    BOOST_VERIFY(v1.toInt() == 10);
    BOOST_VERIFY(v2.toDouble() == 10.5);
    BOOST_VERIFY(v3.toBool() == true);
    BOOST_VERIFY(v4.toBool() == false);
}

BOOST_AUTO_TEST_CASE(value_bool)
{
    Value v1(true);
    Value v2(false);

    BOOST_VERIFY(v1 == true);
    BOOST_VERIFY(v2 == false);

    BOOST_VERIFY(v1.toString() == "true");
    BOOST_VERIFY(v2.toString() == "false");
    BOOST_VERIFY(v1.toInt() == 1);

}


BOOST_AUTO_TEST_CASE(value_fromValue)
{
    Value v1 = Value::fromValue(true);
    Value v2 = Value::fromValue(false);
    Value v3 = Value::fromValue(std::string("string"));
    Value v4 = Value::fromValue(10);
    Value v5 = Value::fromValue(-10);
    Value v6 = Value::fromValue(10.10);
    Value v7 = Value::fromValue(-10.10);

    BOOST_VERIFY(v1.type() == Value::Bool);
    BOOST_VERIFY(v2.type() == Value::Bool);
    BOOST_VERIFY(v3.type() == Value::String);
    BOOST_VERIFY(v4.type() == Value::Int);
    BOOST_VERIFY(v5.type() == Value::Int);
    BOOST_VERIFY(v6.type() == Value::Double);
    BOOST_VERIFY(v7.type() == Value::Double);

    BOOST_VERIFY(v1 == true);
    BOOST_VERIFY(v2 == false);
    BOOST_VERIFY(v3 == "string");
    BOOST_VERIFY(v4 == 10);
    BOOST_VERIFY(v5 == -10);
    BOOST_VERIFY(v6 == 10.10);
    BOOST_VERIFY(v7 == -10.10);
}

BOOST_AUTO_TEST_CASE(value_null)
{
    Value nil;

    BOOST_VERIFY(nil.isNull());
    BOOST_VERIFY(nil.isEmpty());
    BOOST_VERIFY(nil.isArray() == false);
    BOOST_VERIFY(nil.isObject() == false);
    BOOST_VERIFY(nil.size() == 0);
    BOOST_VERIFY(nil.toString() == "");
    BOOST_VERIFY(nil.toInt() == 0);
    BOOST_VERIFY(nil.type() == Value::Null);
}

BOOST_AUTO_TEST_CASE(value_object)
{
    Value obj(Value::Object);

    BOOST_VERIFY(obj.isNull() == false);
    BOOST_VERIFY(obj.isEmpty());
    BOOST_VERIFY(obj.isObject());
    BOOST_VERIFY(obj.isArray() == false);
    BOOST_VERIFY(obj.size() == 0);
    BOOST_VERIFY(obj.type() == Value::Object);

    obj["property1"] = 10;
    obj["property2"] = "string";
    obj["property3"] = true;

    BOOST_VERIFY(obj.size() == 3);
    BOOST_VERIFY(obj["property1"] == 10);
    BOOST_VERIFY(obj["property2"] == "string");
    BOOST_VERIFY(obj["property3"] == true);

    BOOST_VERIFY(obj.hasMember("property1") == true);
    BOOST_VERIFY(obj.hasMember("property2") == true);
    BOOST_VERIFY(obj.hasMember("property3") == true);
    BOOST_VERIFY(obj.hasMember("invalid_property") == false);
}

BOOST_AUTO_TEST_CASE(value_array)
{
    Value obj(Value::Array);

    BOOST_VERIFY(obj.isNull() == false);
    BOOST_VERIFY(obj.isEmpty());
    BOOST_VERIFY(obj.isArray());
    BOOST_VERIFY(obj.isObject() == false);
    BOOST_VERIFY(obj.size() == 0);
    BOOST_VERIFY(obj.type() == Value::Array);

    obj.append(10);
    obj[1] = std::string("string");
    obj[2] = true;

    BOOST_VERIFY(obj.size() == 3);
    BOOST_VERIFY(obj[0] == 10);
    BOOST_VERIFY(obj[1] == "string");
    BOOST_VERIFY(obj[2] == true);
    BOOST_VERIFY(obj.at(0) == 10);
    BOOST_VERIFY(obj.at(1) == "string");
    BOOST_VERIFY(obj.at(2) == true);
}

BOOST_AUTO_TEST_CASE(value_user_type)
{
    Value obj;

    {
        std::list<Value> list;

        list.push_back(10);
        list.push_back("test");
        list.push_back(false);

        obj = Value::fromValue(list);
    }

    BOOST_VERIFY(obj.isNull() == false);
    BOOST_VERIFY(obj.isEmpty() == false);
    BOOST_VERIFY(obj.type() == Value::UserType);

    {
        std::list<Value> list = obj.toValue< std::list<Value> >();
        std::list<Value>::iterator it = list.begin();

        BOOST_VERIFY(list.size() == 3);

        if( list.size() == 3 )
        {
            Value v1 = *it++;
            Value v2 = *it++;
            Value v3 = *it++;

            BOOST_VERIFY(v1 == 10);
            BOOST_VERIFY(v2 == "test");
            BOOST_VERIFY(v3 == false);
        }
    }
}

BOOST_AUTO_TEST_CASE(value_ctor)
{
    Value obj1(Value::Bool);
    Value obj2(Value::Int);
    Value obj3(Value::Integer);
    Value obj4(Value::Double);
    Value obj5(Value::String);
    Value obj6(Value::Array);
    Value obj7(Value::Object);
    Value obj8(Value::UserType);

    BOOST_VERIFY(obj1.type() == Value::Bool);
    BOOST_VERIFY(obj2.type() == Value::Int);
    BOOST_VERIFY(obj3.type() == Value::Integer);
    BOOST_VERIFY(obj4.type() == Value::Double);
    BOOST_VERIFY(obj5.type() == Value::String);
    BOOST_VERIFY(obj6.type() == Value::Array);
    BOOST_VERIFY(obj7.type() == Value::Object);
    BOOST_VERIFY(obj8.type() == Value::UserType);
}


BOOST_AUTO_TEST_SUITE_END()
