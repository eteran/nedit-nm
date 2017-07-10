
#ifndef EXPRESSION_H_
#define EXPRESSION_H_

#include "Token.h"
#include <memory>
#include <vector>
#include <boost/variant.hpp>

class Expression;

struct BinaryExpression {
	std::unique_ptr<Expression> lhs;
	std::unique_ptr<Expression> rhs;
	Token::Type                 op;
};

struct UnaryExpression {
	std::unique_ptr<Expression> operand;
	Token::Type                 op;
	bool                        prefix;
};

struct AtomExpression {
	std::string atom;
};

struct CallExpression {
	std::unique_ptr<Expression>              function;
	std::vector<std::unique_ptr<Expression>> parameters;
};

struct ArrayIndexExpression {
    std::unique_ptr<Expression>              array;
    std::vector<std::unique_ptr<Expression>> index;
};

struct InvalidExpression {};

// instead of using a class heirarchy, we use this variant because we want to
// be able to "upgrade" an expression when building it up
class Expression {
public:
    Expression()                                 = default;
    Expression(const Expression &other)          = delete;
	Expression &operator=(const Expression &rhs) = delete;
    Expression(Expression &&other)               = default;
    Expression &operator=(Expression &&rhs)      = default;
    ~Expression()                                = default;

public:
    template <class T>
    Expression &operator=(T expr) {
        value_ = std::move(expr);
		return *this;
	}

    template <class T>
    T *get() {
        return boost::get<T>(&value_);
    }

private:
    boost::variant<InvalidExpression, BinaryExpression, UnaryExpression, AtomExpression, CallExpression, ArrayIndexExpression> value_;
};

#endif
