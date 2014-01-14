/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: BSD
 */

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <string>
#include <list>

#include "value.h"
#include "templateasttree.h"
#include "templateengine.h"
#include "templatecontext.h"

using namespace cpptl;

static Value nodeEval(const Node *node, const TemplateContext &context);
static std::string nodeTraverse(const Node *node, const TemplateContext &context);
static std::string escapeHtml(const std::string &s);

extern "C" void dump(const std::string &tabs, const Node *node, int level);

struct IfConditionNode {
    IfConditionNode(const AstNode *expression, const AstNode *ifStatement,
                    const AstNode *elseIfStatement, const AstNode *elseStatement)
        : expression(expression), ifStatement(ifStatement),
          elseIfStatement(elseIfStatement), elseStatement(elseStatement)
    {}

    ~IfConditionNode();

    const AstNode *expression;
    const AstNode *ifStatement;
    const AstNode *elseIfStatement;
    const AstNode *elseStatement;
};

struct ElseIfConditionNode {
    ElseIfConditionNode(const AstNode *expression, const AstNode *statement)
        : expression(expression), statement(statement)
    {}

    ~ElseIfConditionNode();

    const AstNode *expression;
    const AstNode *statement;
};

struct HelperNode {
    HelperNode(const std::string &name, const AstNode *arguments, const AstNode *member)
        : name(name), arguments(arguments), member(member)
    {}

    ~HelperNode();

    const std::string name;
    const AstNode *arguments;
    const AstNode *member;
};

struct UnlessConditionNode {
    UnlessConditionNode(const AstNode *expression, const AstNode *unlessStatement,
                        const AstNode *elseStatement)
        : expression(expression), unlessStatement(unlessStatement), elseStatement(elseStatement)
    {}

    ~UnlessConditionNode();

    const AstNode *expression;
    const AstNode *unlessStatement;
    const AstNode *elseStatement;
};

struct ForLoopNode {
    ForLoopNode(const AstNode *variable, const AstNode *list, const AstNode *statement)
        : variable(variable), list(list), statement(statement)
    {}

    ~ForLoopNode();

    const AstNode *variable;
    const AstNode *list;
    const AstNode *statement;
};

struct VariableNode {
    VariableNode(const std::string &name, AstNode *member)
        : name(name), member(member)
    {}

    ~VariableNode();

    const std::string name;
    AstNode *member;
};

struct ObjectNode {
    ObjectNode(AstNode *members)
        : members(members)
    {}

    ~ObjectNode();

    AstNode *members;
};

struct ObjectMemberNode {
    ObjectMemberNode(const std::string &name, const AstNode *value)
        : name(name), value(value)
    {}

    ~ObjectMemberNode();

    const std::string name;
    const AstNode *value;
};

struct BinaryExpressionOp {
    enum Operation {
        Plus,
        Minus,
        Multiply,
        Divide,
        Eq,
        NotEq,
        GreatOrEq,
        Great,
        LessOrEq,
        Less,
    };

    BinaryExpressionOp(Operation operation, const AstNode *lhs, const AstNode *rhs)
        : operation(operation), lhs(lhs), rhs(rhs)
    {}

    ~BinaryExpressionOp();

    Operation operation;
    const AstNode *lhs;
    const AstNode *rhs;
};

struct AstNode
{
    enum NodeType {
        Invalid = 0,
        IntegerValue = 1,
        StringValue = 2,
        HtmlText = 3,
        Variable = 4,
        IfCondition = 5,
        ElseIfCondition = 6,
        UnlessCondition = 7,
        ForLoop = 8,
        Helper = 9,
        Object = 10,
        ObjectMember = 11,
        BinaryExpression = 12
    };

    AstNode(NodeType type) : type(type), next(0)
    {
        value.text = 0;
    }

    ~AstNode()
    {
        delete next;

        switch(type) {
        case HtmlText:
        case StringValue:
            delete value.text;
            break;
        case Variable:
            delete value.variable;
            break;
        case IfCondition:
            delete value.ifCondition;
            break;
        case ElseIfCondition:
            delete value.elseIfCondition;
            break;
        case UnlessCondition:
            delete value.unlessCondition;
            break;
        case ForLoop:
            delete value.forLoop;
            break;
        case Helper:
            delete value.helper;
            break;
        case Object:
            delete value.object;
            break;
        case ObjectMember:
            delete value.objectMember;
            break;
        case BinaryExpression:
            delete value.binaryExpr;
            break;
        case IntegerValue:
        case Invalid:
        default:
            break;
        }
    }

    union {
        int integer;
        std::string *text;
        IfConditionNode *ifCondition;
        ElseIfConditionNode *elseIfCondition;
        UnlessConditionNode *unlessCondition;
        ForLoopNode *forLoop;
        HelperNode  *helper;
        VariableNode *variable;
        ObjectNode *object;
        ObjectMemberNode *objectMember;
        BinaryExpressionOp *binaryExpr;
    } value;

    NodeType type;
    AstNode *next;
};

IfConditionNode::~IfConditionNode()
{
    delete expression;
    delete ifStatement;
    delete elseIfStatement;
    delete elseStatement;
}

ElseIfConditionNode::~ElseIfConditionNode()
{
    delete expression;
    delete statement;
}

HelperNode::~HelperNode()
{
    delete arguments;
    delete member;
}

UnlessConditionNode::~UnlessConditionNode()
{
    delete expression;
    delete unlessStatement;
    delete elseStatement;
}

ForLoopNode::~ForLoopNode()
{
    delete variable;
    delete list;
    delete statement;
}

VariableNode::~VariableNode()
{
    delete member;
}

ObjectNode::~ObjectNode()
{
    delete members;
}

ObjectMemberNode::~ObjectMemberNode()
{
    delete value;
}

BinaryExpressionOp::~BinaryExpressionOp()
{
    delete lhs;
    delete rhs;
}


extern "C" {

Node *nodeAddIntegerExpression(int value)
{
    Node *node = new AstNode(AstNode::IntegerValue);
    node->value.integer = value;
    return node;
}

Node *nodeAddStringExpression(const char *string)
{
    Node *node = new AstNode(AstNode::StringValue);
    node->value.text = new std::string(string);
    return node;
}

Node *nodeAddHtmlText(const char *text)
{
    Node *node = new AstNode(AstNode::HtmlText);
    node->value.text = new std::string(text);
    return node;
}

Node *nodeAddVariable(const char *name)
{
    VariableNode *var = new VariableNode(name, NULL);
    Node *node = new AstNode(AstNode::Variable);

    node->value.variable = var;
    return node;
}

Node *nodeAddVariableMember(Node *variable, Node *member)
{
    assert( variable->type == AstNode::Variable );

    Node *lastVar = variable;
    while( lastVar->value.variable->member )
        lastVar = lastVar->value.variable->member;
    lastVar->value.variable->member = member;

    return variable;
}

//Node *nodeAddQuotedVariable(const char *name)
//{
//    assert( strlen(name) > 2 );
//    const char *ptr = name + 1;
//    while( isspace(*ptr) )
//        ++ptr;

//    VariableNode *var = new VariableNode{ptr, NULL};
//    Node *node = new AstNode{AstNode::Variable};

//    node->value.variable = var;
//    return node;
//}

Node *nodeAddIfCondition(Node *expression, Node *statement)
{
    IfConditionNode *ifConditionNode = new IfConditionNode(expression, statement, NULL, NULL);
    Node *node = new AstNode(AstNode::IfCondition);
    node->value.ifCondition = ifConditionNode;
    return node;
}

Node *nodeAddElseIfCondition(Node *expression, Node *statement)
{
    ElseIfConditionNode *elseIfConditionNode = new ElseIfConditionNode(expression, statement);
    Node *node = new AstNode(AstNode::ElseIfCondition);
    node->value.elseIfCondition = elseIfConditionNode;
    return node;
}

Node *nodeAddIfElseCondition(Node *expression, Node *ifStatement, Node *elseStatement)
{
    IfConditionNode *ifConditionNode = new IfConditionNode(expression, ifStatement, NULL, elseStatement);
    Node *node = new AstNode(AstNode::IfCondition);
    node->value.ifCondition = ifConditionNode;
    return node;
}

Node *nodeAddIfElseIfCondition(Node *expression, Node *ifStatement, Node *elseIfNode)
{
    IfConditionNode *ifConditionNode = new IfConditionNode(expression, ifStatement, elseIfNode, NULL);
    Node *node = new AstNode(AstNode::IfCondition);
    node->value.ifCondition = ifConditionNode;
    return node;
}

Node *nodeAddIfElseIfElseCondition(Node *expression, Node *ifStatement, Node *elseIfNode, Node *elseStatement)
{
    IfConditionNode *ifConditionNode = new IfConditionNode(expression, ifStatement, elseIfNode, elseStatement);
    Node *node = new AstNode(AstNode::IfCondition);
    node->value.ifCondition = ifConditionNode;
    return node;
}


Node *nodeAddUnlessCondition(Node *expression, Node *statement)
{
    UnlessConditionNode *unlessConditionNode = new UnlessConditionNode(expression, statement, NULL);
    Node *node = new AstNode(AstNode::UnlessCondition);
    node->value.unlessCondition = unlessConditionNode;
    return node;
}

Node *nodeAddUnlessElseCondition(Node *expression, Node *unlessStatement, Node *elseStatement)
{
    UnlessConditionNode *unlessConditionNode = new UnlessConditionNode(expression, unlessStatement, elseStatement);
    Node *node = new AstNode(AstNode::UnlessCondition);
    node->value.unlessCondition = unlessConditionNode;
    return node;
}

Node *nodeAddForLoop(Node *variableName, Node *list, Node *statement)
{
    assert( variableName->type == AstNode::StringValue );

    ForLoopNode *forLoopNode = new ForLoopNode(variableName, list, statement);
    Node *node = new AstNode(AstNode::ForLoop);
    node->value.forLoop = forLoopNode;
    return node;
}

Node *nodeAddHelper(const char *name, Node *arguments)
{
    HelperNode *helper = new HelperNode(name, arguments, NULL);
    Node *node = new AstNode(AstNode::Helper);
    node->value.helper = helper;
    return node;
}

Node *nodeAddHelperMembers(Node *helper, Node *member)
{
    helper->value.helper->member = member;
    return helper;
}

Node *nodeAddQuotedHelper(const char *name, Node *arguments, Node *member)
{
    const char *ptr = name;

    HelperNode *helper = new HelperNode(ptr, arguments, member);
    Node *node = new AstNode(AstNode::Helper);
    node->value.helper = helper;
    return node;
}

Node *nodeAddObjectMember(const char *name, Node *value)
{
    ObjectMemberNode *member = new ObjectMemberNode(name, value);
    Node *node = new AstNode(AstNode::ObjectMember);
    node->value.objectMember = member;
    return node;
}

Node *nodeAddObject(Node *members)
{
    ObjectNode *object = new ObjectNode(members);
    Node *node = new AstNode(AstNode::Object);
    node->value.object = object;
    return node;
}

Node *nodeAddSibling(Node *node, Node *sibling)
{
    Node *examine = node;

    while(examine->next )
        examine = examine->next;
    examine->next = sibling;

    return node;
}

Node *nodeAddPlus(Node *lhs, Node *rhs)
{
    BinaryExpressionOp *expr = new BinaryExpressionOp(BinaryExpressionOp::Plus, lhs, rhs);
    Node *node = new AstNode(AstNode::BinaryExpression);
    node->value.binaryExpr = expr;
    return node;
}

Node *nodeAddMinus(Node *lhs, Node *rhs)
{
    BinaryExpressionOp *expr = new BinaryExpressionOp(BinaryExpressionOp::Minus, lhs, rhs);
    Node *node = new AstNode(AstNode::BinaryExpression);
    node->value.binaryExpr = expr;
    return node;
}

Node *nodeAddMutiply(Node *lhs, Node *rhs)
{
    BinaryExpressionOp *expr = new BinaryExpressionOp(BinaryExpressionOp::Multiply, lhs, rhs);
    Node *node = new AstNode(AstNode::BinaryExpression);
    node->value.binaryExpr = expr;
    return node;
}

Node *nodeAddDivide(Node *lhs, Node *rhs)
{
    BinaryExpressionOp *expr = new BinaryExpressionOp(BinaryExpressionOp::Divide, lhs, rhs);
    Node *node = new AstNode(AstNode::BinaryExpression);
    node->value.binaryExpr = expr;
    return node;
}

Node *nodeAddEq(Node *lhs, Node *rhs)
{
    BinaryExpressionOp *expr = new BinaryExpressionOp(BinaryExpressionOp::Eq, lhs, rhs);
    Node *node = new AstNode(AstNode::BinaryExpression);
    node->value.binaryExpr = expr;
    return node;
}

Node *nodeAddNotEq(Node *lhs, Node *rhs)
{
    BinaryExpressionOp *expr = new BinaryExpressionOp(BinaryExpressionOp::NotEq, lhs, rhs);
    Node *node = new AstNode(AstNode::BinaryExpression);
    node->value.binaryExpr = expr;
    return node;
}

Node *nodeAddGreatOrEq(Node *lhs, Node *rhs)
{
    BinaryExpressionOp *expr = new BinaryExpressionOp(BinaryExpressionOp::GreatOrEq, lhs, rhs);
    Node *node = new AstNode(AstNode::BinaryExpression);
    node->value.binaryExpr = expr;
    return node;
}

Node *nodeAddGreat(Node *lhs, Node *rhs)
{
    BinaryExpressionOp *expr = new BinaryExpressionOp(BinaryExpressionOp::Great, lhs, rhs);
    Node *node = new AstNode(AstNode::BinaryExpression);
    node->value.binaryExpr = expr;
    return node;
}

Node *nodeAddLessOrEq(Node *lhs, Node *rhs)
{
    BinaryExpressionOp *expr = new BinaryExpressionOp(BinaryExpressionOp::LessOrEq, lhs, rhs);
    Node *node = new AstNode(AstNode::BinaryExpression);
    node->value.binaryExpr = expr;
    return node;
}

Node *nodeAddLess(Node *lhs, Node *rhs)
{
    BinaryExpressionOp *expr = new BinaryExpressionOp(BinaryExpressionOp::Less, lhs, rhs);
    Node *node = new AstNode(AstNode::BinaryExpression);
    node->value.binaryExpr = expr;
    return node;
}

void dump(const std::string &tabs, const Node *node, int level)
{
    assert( node != NULL );

    switch(node->type)
    {
    case AstNode::IntegerValue:
        std::cerr << tabs << "integer " << node->value.integer << std::endl;
        break;
    case AstNode::StringValue:
        std::cerr << tabs << "string \"" << *node->value.text << '\"' << std::endl;
        break;
    case AstNode::HtmlText:
        std::cerr << tabs << "html text \"" << *node->value.text << '\"' << std::endl;
        break;
    case AstNode::Variable: {
        std::cerr << tabs << "variable " << node->value.variable->name << std::endl;

        Node *member = node->value.variable->member;
        while( member ) {
            std::cerr << tabs << "    member: " << member->value.variable->name << std::endl;
            member = member->next;
        }

        break;
    }
    case AstNode::IfCondition:
        std::cerr << tabs << "if condition: " << std::endl;
        nodePrint2(node->value.ifCondition->expression, level + 2);
        std::cerr << tabs << "    statement" << std::endl;
        nodePrint2(node->value.ifCondition->ifStatement, level + 2);

        if( node->value.ifCondition->elseIfStatement ) {
            std::cerr << tabs << "    else if statement" << std::endl;
            nodePrint2(node->value.ifCondition->elseIfStatement, level + 2);
        }
        if( node->value.ifCondition->elseStatement ) {
            std::cerr << tabs << "    else statement" << std::endl;
            nodePrint2(node->value.ifCondition->elseStatement, level + 2);
        }
        std::cerr << tabs << "endif" << std::endl;
        break;
    case AstNode::ElseIfCondition:
        std::cerr << tabs << "else-if condition:" << std::endl;
        nodePrint2(node->value.elseIfCondition->expression, level + 2);
        nodePrint2(node->value.elseIfCondition->statement, level + 2);
        std::cerr <<  "\n";
        break;
    case AstNode::UnlessCondition:
        std::cerr << tabs << "unless condition:" << std::endl;
        nodePrint2(node->value.unlessCondition->expression, level + 2);
        std::cerr << tabs << "    statement:" << std::endl;
        nodePrint2(node->value.unlessCondition->unlessStatement , level + 2);

        if( node->value.unlessCondition->elseStatement ) {
            std::cerr << tabs << "    else statement:" << std::endl;
            nodePrint2(node->value.unlessCondition->elseStatement, level + 2);
        }

        std::cerr << "\n";
        break;
    case AstNode::ForLoop:
        std::cerr << tabs << "for loop:" << std::endl;
        nodePrint2(node->value.forLoop->variable, level + 1);
        nodePrint2(node->value.forLoop->list, level + 1);
        nodePrint2(node->value.forLoop->statement , level + 2);
        std::cerr << "\n";
        break;
    case AstNode::Helper:
        std::cerr << tabs << "helper: " << node->value.helper->name << std::endl;

        if( node->value.helper->arguments ) {
            std::cerr << tabs << "arguments: " << std::endl;
            nodePrint2(node->value.helper->arguments, level + 2);
        }

        if( node->value.helper->member ) {
            std::cerr << tabs << "members: " << std::endl;

            const Node *member = node->value.helper->member;
            while( member ) {
                std::cerr << tabs << "    member: " << member->value.helper->name << std::endl;
                member = member->next;
            }
        }

        std::cerr << "\n";
        break;
    case AstNode::ObjectMember:
        std::cerr << tabs << "object member: " << node->value.objectMember->name << std::endl;
        nodePrint2( node->value.objectMember->value, level + 2 );
        std::cerr << "\n";
        break;
    case AstNode::Object:
        if( node->value.object->members ) {
            std::cerr << tabs << "object: " << std::endl;
            nodePrint2( node->value.object->members, level + 1 );
        }
        else {
            std::cerr << tabs << "object: empty" << std::endl;
        }
        std::cerr << "\n";
        break;
    case AstNode::BinaryExpression: {
        const AstNode *lhs = node->value.binaryExpr->lhs;
        const AstNode *rhs = node->value.binaryExpr->lhs;

        switch( node->value.binaryExpr->operation )
        {
        case BinaryExpressionOp::Plus:
            std::cerr << tabs << "expression plus" << std::endl;
            break;
        case BinaryExpressionOp::Minus:
            std::cerr << tabs << "expression minus" << std::endl;
            break;
        case BinaryExpressionOp::Multiply:
            std::cerr << tabs << "expression multiply" << std::endl;
            break;
        case BinaryExpressionOp::Divide:
            std::cerr << tabs << "expression divide" << std::endl;
            break;
        case BinaryExpressionOp::Eq:
            std::cerr << tabs << "expression eq" << std::endl;
            break;
        case BinaryExpressionOp::NotEq:
            std::cerr << tabs << "expression not-eq" << std::endl;
            break;
        case BinaryExpressionOp::GreatOrEq:
            std::cerr << tabs << "expression great-or-eq" << std::endl;
            break;
        case BinaryExpressionOp::Great:
            std::cerr << tabs << "expression great" << std::endl;
            break;
        case BinaryExpressionOp::LessOrEq:
            std::cerr << tabs << "expression less-or-eq" << std::endl;
            break;
        case BinaryExpressionOp::Less:
            std::cerr << tabs << "expression less" << std::endl;
            break;
        default:
            abort();
        }

        std::cerr << tabs << "   lhs:" << std::endl;
        nodePrint2( lhs, level + 2 );
        std::cerr << tabs << "   rhs:" << std::endl;
        nodePrint2( rhs, level + 2 );

        std::cerr << "\n";
        break;
    }
    default:
        abort();
    }
}

void nodePrint2(const Node *node, int level)
{
    std::string tabs;
    for(int i = 0; i < level; ++i)
        tabs += "    ";

    dump(tabs, node, level);

    while(node->next) {
        dump(tabs, node->next, level);
        node = node->next;
    }
}

void nodePrint(const char *text, const Node *node)
{
    std::cerr << "printNode: " << text << std::endl;
    nodePrint2(node, 0);
}

void freeNodes(Node *root)
{
    delete root;
}

}

static std::string evalForArray(const std::string &varName,
                               const Node *statement,
                               const Value &array,
                               const TemplateContext &context)
{
    std::string result;
    Value newContext = Value(Value::ObjectTag());

    newContext["parentContext"] = context.context;

    Value::ValueIterator it(array);

    while(it.hasNext())
    {
        newContext[varName] = it.next();
        TemplateContext ctx = {context.templ, newContext, context.caller};
        result += nodeTraverse(statement, ctx);
    }

    return result;
}

Value findVariable(const Value &context, const std::string &name)
{
    assert(name.empty() == false );

    if( context.hasMember(name) )
    {
        return context.member(name);
    }
    else if( context.hasMember("parentContext") )
    {
        return findVariable(context.member("parentContext"), name);
    }
    else if( context.type() == Value::Array || context.type() == Value::Object )
    {
        switch(name[0]) {
        case 'l':
            if( name == "length" )
                return context.size();
            break;
        case 's':
            if( name == "size" )
                return context.size();
            break;
        case 'e':
            if( name == "empty?" )
                return context.size() == 0;
            break;
        case 'i':
            if( name == "isEmpty?" )
                return context.size() == 0;
            break;
        }
    }

    std::cerr << "Invalid variable: " << name << std::endl;
    return Value();
}

// TODO Value обойдется дорого, надо что-нибудь придумать!
static Value nodeEval(const Node *node, const TemplateContext &context)
{
    switch(node->type)
    {
    case AstNode::IntegerValue:
        return Value(node->value.integer);
        break;
    case AstNode::StringValue:
        return Value(*node->value.text);
        break;
    case AstNode::HtmlText:
        return Value(*node->value.text);
        break;
    case AstNode::Variable: {
        Value value = findVariable(context.context, *node->value.text);
        const Node *member = node->value.variable->member;

        while( member && value.isNull() == false ) {
            value = findVariable(value, *member->value.text);
            member = member->value.variable->member;
        }

        if( value.type() == Value::String )
            value = escapeHtml(value.toString());

        return value;
        break;
    }
    case AstNode::IfCondition: {
        Value value = nodeEval(node->value.ifCondition->expression, context);

        if( value.toBool() )
        {
            return nodeTraverse( node->value.ifCondition->ifStatement, context );
        }
        else if( node->value.ifCondition->elseIfStatement )
        {
            const Node *elseIfNode = node->value.ifCondition->elseIfStatement;

            while( elseIfNode )
            {
                assert( elseIfNode->type == AstNode::ElseIfCondition );
                Value value = nodeEval(elseIfNode->value.elseIfCondition->expression, context);

                if( value.toBool() )
                    return nodeTraverse( elseIfNode->value.elseIfCondition->statement, context );
                else
                    elseIfNode = elseIfNode->next;
            }
        }

        if( node->value.ifCondition->elseStatement )
            return nodeTraverse( node->value.ifCondition->elseStatement, context );
        else
            return Value();

        break;
    }
    case AstNode::UnlessCondition: {
        Value value = nodeEval(node->value.unlessCondition->expression, context);

        if( value.toBool() == false )
            return nodeTraverse(node->value.unlessCondition->unlessStatement, context);
        else if( node->value.unlessCondition->elseStatement )
            return nodeTraverse(node->value.unlessCondition->elseStatement, context);
        else
            return Value();
        break;
    }
    case AstNode::ForLoop: {
        assert( node->value.forLoop->variable->type == AstNode::StringValue );

        Value list = nodeEval(node->value.forLoop->list, context);
        const Node *statement = node->value.forLoop->statement;
        const std::string varName = *node->value.forLoop->variable->value.text;

        if( list.type() == Value::Array || list.type() == Value::Object )
            return evalForArray(varName, statement, list, context);
        else
            return Value();

        break;
    }
    case AstNode::Helper: {
        const std::string &name = node->value.helper->name;
        const Node *argsNode = node->value.helper->arguments;
        Value args = Value(Value::ArrayTag());

        {
            const Node *examine = argsNode;
            while(examine) {
                args.append( nodeEval(examine, context) );
                examine = examine->next;
            }
        }


        const TemplateEngine &engine = context.caller.engine();
        Value result = engine.callHelper(name, context.context, args );
        const Node *member = node->value.helper->member;

        while( member )
        {
            result = findVariable(result, *member->value.text);
            member = member->value.variable->member;
        }

        return result;

        break;
    }
    case AstNode::Object: {
        Value obj(Value::Object);
        const Node *member = node->value.object->members;

        while( member ) {
            obj[member->value.objectMember->name.c_str()] = nodeEval(member->value.objectMember->value, context);
            member = member->next;
        }

        return obj;
        break;
    }
    case AstNode::BinaryExpression: {
        const Value &lhs = nodeEval(node->value.binaryExpr->lhs, context);
        const Value &rhs = nodeEval(node->value.binaryExpr->rhs, context);

        switch( node->value.binaryExpr->operation )
        {
        case BinaryExpressionOp::Plus:
            return lhs + rhs;
            break;
        case BinaryExpressionOp::Minus:
            return lhs - rhs;
            break;
        case BinaryExpressionOp::Multiply:
            return lhs * rhs;
            break;
        case BinaryExpressionOp::Divide:
            return lhs / rhs;
            break;
        case BinaryExpressionOp::Eq:
            return lhs == rhs;
            break;
        case BinaryExpressionOp::NotEq:
            return lhs != rhs;
            break;
        case BinaryExpressionOp::GreatOrEq:
            return lhs >= rhs;
            break;
        case BinaryExpressionOp::Great:
            return lhs > rhs;
            break;
        case BinaryExpressionOp::LessOrEq:
            return lhs <= rhs;
            break;
        case BinaryExpressionOp::Less:
            return lhs < rhs;
            break;
        default:
            std::cerr << "invalid expression type: " << node->value.binaryExpr->operation
                      << std::endl;

            abort();
        }
        break;
    }
    default:
        break;
    }

    std::cerr << "invalid nodeType: " << node->type << std::endl;
    abort();
    return Value();
}

static std::string nodeTraverse(const Node *node, const TemplateContext &context)
{
    std::string result = nodeEval(node, context).toString();

    while( node->next )
    {
        result += nodeEval(node->next, context).toString();
        node = node->next;
    }

    return result;
}

std::string traverserTreeNodes(const Node *node, const TemplateContext &context)
{
    if( node )
        return nodeTraverse(node, context);
    else
        return std::string();
}

static std::string escapeHtml(const std::string &s)
{
    std::string result = s;

    boost::replace_all( result, "&", "&amp;" );
    boost::replace_all( result, ">", "&gt;" );
    boost::replace_all( result, "<", "&lt;" );
    boost::replace_all( result, "\"", "&quot;" );

    return result;
}
