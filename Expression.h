
#ifndef EXPRESSION_H_
#define EXPRESSION_H_

#include "Token.h"
#include <memory>
#include <vector>

class Expression;

struct BinaryExpression {
    std::unique_ptr<Expression> lhs;
    std::unique_ptr<Expression> rhs;
    Token::Type                 op;
};

struct UnaryExpression {
    std::unique_ptr<Expression> operand;
    Token::Type                 op;
    bool prefix;
};

struct AtomExpression {
	std::string atom;
};

struct CallExpression {
    std::unique_ptr<Expression> function;
    std::vector<std::unique_ptr<Expression>> parameters;
};

struct ArrayIndexExpression {
    std::unique_ptr<Expression> array;
    std::unique_ptr<Expression> index;
};

struct InvalidExpression {
};

// instead of using a class heirarchy, we use this variant because we want to
// be able to "upgrade" an expression when building it up
class Expression {
public:
    enum Type : uint8_t { Invalid, Binary, Unary, Atom, Call, ArrayIndex };

public:
	Expression() : type_(Invalid) {
	}

    Expression(const Expression &other) = delete;
    Expression &operator=(const Expression &rhs)  = delete;
	
	Expression(Expression &&other) : type_(other.type_) {
		switch(type_) {
		case Invalid:
			break;
		case Binary:
			new (&binary_) BinaryExpression(std::move(other.binary_));
			break;
		case Unary:
			new (&unary_) UnaryExpression(std::move(other.unary_));
			break;
		case Atom:
			new (&atom_) AtomExpression(std::move(other.atom_));
			break;
        case Call:
            new (&call_) CallExpression(std::move(other.call_));
            break;
        case ArrayIndex:
            new (&arrayIndex_) ArrayIndexExpression(std::move(other.arrayIndex_));
            break;
		}
		
		other.destroy();
	}
	
	Expression &operator=(Expression &&rhs) {
		destroy();
		
		switch(rhs.type_) {
		case Invalid:
			break;
		case Binary:
			new (&binary_) BinaryExpression(std::move(rhs.binary_));
			break;
		case Unary:
			new (&unary_) UnaryExpression(std::move(rhs.unary_));
			break;
		case Atom:
			new (&atom_) AtomExpression(std::move(rhs.atom_));
			break;
        case Call:
            new (&call_) CallExpression(std::move(rhs.call_));
            break;
        case ArrayIndex:
            new (&arrayIndex_) ArrayIndexExpression(std::move(rhs.arrayIndex_));
            break;
		}
		
		type_ = rhs.type_;		
		rhs.destroy();
		return *this;
	}
	
	~Expression() {
		destroy();
	}
	
public:
	explicit operator bool() const {
		return type_ != Invalid;
	}
	
	bool isValid() const {
		return type_ != Invalid;
	}
	
public:
	Expression &operator=(BinaryExpression expr) {
		destroy();
		new (&binary_) BinaryExpression(std::move(expr));
		type_ = Binary;
		return *this;
	}
	
	Expression &operator=(UnaryExpression expr) {
		destroy();
		new (&unary_) UnaryExpression(std::move(expr));
		type_ = Unary;
		return *this;
	}
	
	Expression &operator=(AtomExpression expr) {
		destroy();
		new (&atom_) AtomExpression(std::move(expr));
		type_ = Atom;
		return *this;
	}

    Expression &operator=(CallExpression expr) {
        destroy();
        new (&call_) CallExpression(std::move(expr));
        type_ = Call;
        return *this;
    }

    Expression &operator=(ArrayIndexExpression expr) {
        destroy();
        new (&arrayIndex_) ArrayIndexExpression(std::move(expr));
        type_ = ArrayIndex;
        return *this;
    }
	
	Expression &operator=(InvalidExpression expr) {
		(void)expr;
		destroy();
		return *this;
	}
	
private:
	void destroy() {
	
		switch(type_) {
		case Invalid:
			break;
		case Binary:
			binary_.~BinaryExpression();
			type_ = Invalid;
			break;
		case Unary:
			unary_.~UnaryExpression();
			type_ = Invalid;
			break;
		case Atom:
			atom_.~AtomExpression();
			type_ = Invalid;
			break;
        case Call:
            call_.~CallExpression();
            type_ = Invalid;
            break;
        case ArrayIndex:
            arrayIndex_.~ArrayIndexExpression();
            type_ = Invalid;
            break;
        }
	}

public:
    Type type() const {
        return type_;
    }

    BinaryExpression *as_binary() {
        return &binary_;
    }

    ArrayIndexExpression *as_index() {
        return &arrayIndex_;
    }

private:
	union {
        InvalidExpression    invalid_;
        BinaryExpression     binary_;
        UnaryExpression      unary_;
        AtomExpression       atom_;
        CallExpression       call_;
        ArrayIndexExpression arrayIndex_;
    };

    Type type_;
};

#endif
