
#ifndef EXPRESSION_H_
#define EXPRESSION_H_

#include "Token.h"
#include <memory>
#include <vector>

struct Expression {
    virtual ~Expression() = default;
};

struct BinaryExpression : public Expression {
	std::unique_ptr<Expression> lhs;
	std::unique_ptr<Expression> rhs;
	Token::Type                 op;
};

struct UnaryExpression : public Expression {
	std::unique_ptr<Expression> operand;
	Token::Type                 op;
	bool                        prefix;
};

struct AtomExpression : public Expression {
    std::string value;
    Token::Type type;
};

struct CallExpression : public Expression {
	std::unique_ptr<Expression>              function;
	std::vector<std::unique_ptr<Expression>> parameters;
};

struct ArrayIndexExpression : public Expression {
	std::unique_ptr<Expression>              array;
    std::vector<std::unique_ptr<Expression>> index;
};

#endif
