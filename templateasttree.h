/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: BSD
 */

#ifndef CPPTL_TEMPLATEASTTREE_H
#define CPPTL_TEMPLATEASTTREE_H

#ifdef __cplusplus

#include "template.h"

extern "C" {
#endif

struct AstNode;
typedef struct AstNode Node;

Node *nodeAddIntegerExpression(int value);
Node *nodeAddStringExpression(const char *string);

Node *nodeAddHtmlText(const char *text);
Node *nodeAddVariable(const char *name);
Node *nodeAddVariableMember(Node *variable, Node *member);
//Node *nodeAddQuotedVariable(const char *name);
Node *nodeAddIfCondition(Node *expression, Node *statement);
Node *nodeAddElseIfCondition(Node *expression, Node *statement);
Node *nodeAddIfElseCondition(Node *expression, Node *ifStatement, Node *elseStatement);
Node *nodeAddIfElseIfCondition(Node *expression, Node *ifStatement, Node *elseIfNode);
Node *nodeAddIfElseIfElseCondition(Node *expression, Node *ifStatement, Node *elseIfNode, Node *elseStatement);
Node *nodeAddUnlessCondition(Node *expression, Node *statement);
Node *nodeAddUnlessElseCondition(Node *expression, Node *unlessStatement, Node *elseStatement);
Node *nodeAddForLoop(Node *variableName, Node *list, Node *statement);
Node *nodeAddHelper(const char *name, Node *arguments);
Node *nodeAddHelperMembers(Node *helper, Node *member);
Node *nodeAddQuotedHelper(const char *name, Node *arguments, Node *member);
Node *nodeAddObjectMember(const char *name, Node *value);
Node *nodeAddObject(Node *members);
Node *nodeAddSibling(Node *node, Node *sibling);

Node *nodeAddPlus(Node *lhs, Node *rhs);
Node *nodeAddMinus(Node *lhs, Node *rhs);
Node *nodeAddMutiply(Node *lhs, Node *rhs);
Node *nodeAddDivide(Node *lhs, Node *rhs);
Node *nodeAddEq(Node *lhs, Node *rhs);
Node *nodeAddNotEq(Node *lhs, Node *rhs);
Node *nodeAddGreatOrEq(Node *lhs, Node *rhs);
Node *nodeAddGreat(Node *lhs, Node *rhs);
Node *nodeAddLessOrEq(Node *lhs, Node *rhs);
Node *nodeAddLess(Node *lhs, Node *rhs);


void nodePrint(const char *text, const Node *node);
void nodePrint2(const Node *node, int level);

void freeNodes(Node *root);

#ifdef __cplusplus
}

class TemplateContext;

std::string traverserTreeNodes(const Node *node, const TemplateContext &context);
#endif

#endif // CPPTL_TEMPLATEASTTREE_H
