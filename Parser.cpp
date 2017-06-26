
#include "Parser.h"
#include "Error.h"
#include "Expression.h"
#include "Input.h"
#include "Statement.h"
#include "Tokenizer.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <cassert>

/**
 * @brief Parser::Parser
 */
Parser::Parser(const std::string &filename) : tokenizer_(filename), index_(0), in_function_(false) {
}

/**
 * @brief Parser::peekToken
 * @return
 */
Token Parser::peekToken() const {
	if (index_ < tokenizer_.size()) {
		return tokenizer_[index_];
	}

	return Token();
}

/**
 * @brief Parser::readToken
 * @return
 */
Token Parser::readToken() {
	Token token = peekToken();

	if (token.type != Token::Invalid) {
		++index_;
	}

	return token;
}

/**
 * @brief Parser::ungetToken
 */
void Parser::ungetToken() {
	if (index_ > 0) {
		--index_;
	}
}

/**
 * @brief Parser::tokenIndex
 * @return
 */
size_t Parser::tokenIndex() const{
    return index_;
}

/**
 * @brief Parser::setTokenIndex
 * @param index
 */
void Parser::setTokenIndex(size_t index) {
    index_ = index;
}

/**
 * @brief Parser::parseForStatement
 * @return
 */
std::unique_ptr<Statement> Parser::parseForStatement() {

    // TODO(eteran): support commas in init/incr portions

    Token openParen = readToken();
    if (openParen.type != Token::LeftParen) {
        throw MissingOpenParen(openParen);
	}

    auto init = parseExpression();

    Token semi1 = peekToken();
    if(semi1.type == Token::Semicolon) {
        // standard C-style FOR loop


        // consume the semicolon now that we are sure
        semi1 = readToken();

        auto cond = parseExpression();

        Token semi2 = readToken();
        if (semi2.type != Token::Semicolon) {
            throw MissingSemicolon(semi2);
        }

        auto incr = parseExpression();

        Token closeParen = readToken();
        if (closeParen.type != Token::RightParen) {
            throw MissingClosingParen(closeParen);
        }

        // consume any newlines
        while(peekToken().type == Token::Newline) {
            readToken();
        }

        if (!init->isValid()) {
            init = nullptr;
        }

        if (!cond->isValid()) {
            cond = nullptr;
        }

        if (!incr->isValid()) {
            incr = nullptr;
        }

        auto body = parseStatement();

        auto loop   = std::make_unique<LoopStatement>();
        loop->body_ = std::move(body);
        loop->init_ = std::move(init);
        loop->incr_ = std::move(incr);
        loop->cond_ = std::move(cond);

        return loop;
    } else {

        if(init->type() == Expression::Binary) {
            BinaryExpression *expr = init->as_binary();
            if(expr->op == Token::In) {
                auto container = std::move(expr->rhs);
                auto iterator  = std::move(expr->lhs);

                Token closeParen = readToken();
                if (closeParen.type != Token::RightParen) {
                    throw MissingClosingParen(closeParen);
                }

                // consume any newlines
                while(peekToken().type == Token::Newline) {
                    readToken();
                }
                auto body = parseStatement();

                auto loop   = std::make_unique<ForEachStatement>();
                loop->iterator  = std::move(iterator);
                loop->container = std::move(container);
                loop->body_     = std::move(body);

                return loop;
            }
        }
    }

    throw MissingSemicolon(peekToken());
}

/**
 * @brief Parser::parseIfStatement
 * @return
 */
std::unique_ptr<CondStatement> Parser::parseIfStatement() {

    Token openParen = readToken();
    if (openParen.type != Token::LeftParen) {
        throw MissingOpenParen(openParen);
	}

	auto condition = parseExpression();


    Token closeParen = readToken();
    if (closeParen.type != Token::RightParen) {
        throw MissingClosingParen(closeParen);
	}

    // consume any newlines
    while(peekToken().type == Token::Newline) {
        readToken();
    }

	auto body = parseStatement();

    auto cond   = std::make_unique<CondStatement>();
    cond->body_ = std::move(body);
    cond->cond_ = std::move(condition);

    // consume any newlines
    while(peekToken().type == Token::Newline) {
        readToken();
    }

	if (peekToken().type == Token::Else) {
		readToken();
		cond->else_ = parseStatement();
	}

	return cond;
}

std::unique_ptr<DeleteStatement> Parser::parseDeleteStatement() {

    auto expr = parseExpression();

    if(expr->type() != Expression::ArrayIndex) {
        throw InvalidDelete(peekToken());
    }

    auto indexExpression = expr->as_index();

    auto stmt  = std::make_unique<DeleteStatement>();
    stmt->expression = std::move(indexExpression->array);
    stmt->index      = std::move(indexExpression->index);

    return stmt;
}

/**
 * @brief Parser::parseReturnStatement
 * @return
 */
std::unique_ptr<ReturnStatement> Parser::parseReturnStatement() {

    auto expr = parseExpression();
    auto ret  = std::make_unique<ReturnStatement>();
    ret->expression = std::move(expr);

    return ret;
}

/**
 * @brief Parser::parseExpressionStatement
 * @return
 */
std::unique_ptr<ExpressionStatement> Parser::parseExpressionStatement() {

	if (auto expression = parseExpression()) {
        auto statement = std::make_unique<ExpressionStatement>();
        statement->expression = std::move(expression);

        Token nl = readToken();
        if (nl.type != Token::Newline) {
            throw MissingNewline(nl);
		}

		return statement;
	}

	return nullptr;
}

/**
 * @brief Parser::parseExpression
 * @return
 */
std::unique_ptr<Expression> Parser::parseExpression() {

    auto expr = std::make_unique<Expression>();

	parseExpression0(expr);

	return expr;
}

/**
 * @brief Parser::parseWhileStatement
 * @return
 */
std::unique_ptr<LoopStatement> Parser::parseWhileStatement() {

    Token openParen = readToken();
    if (openParen.type != Token::LeftParen) {
        throw MissingOpenParen(openParen);
	}

    auto condition = parseExpression();

    Token closeParen = readToken();
    if (closeParen.type != Token::RightParen) {
        throw MissingClosingParen(closeParen);
	}

    // consume any newlines
    while(peekToken().type == Token::Newline) {
        readToken();
    }

	auto body = parseStatement();

    auto loop   = std::make_unique<LoopStatement>();
    loop->body_ = std::move(body);
	loop->init_ = nullptr;
	loop->incr_ = nullptr;
    loop->cond_ = std::move(condition);

	return loop;
}

/**
 * @brief Parser::parseBlockStatement
 * @return
 */
std::unique_ptr<BlockStatement> Parser::parseBlockStatement() {

    auto block = std::make_unique<BlockStatement>();

    Token openBrace = readToken();
    if (openBrace.type != Token::LeftBrace) {
        throw MissingOpenBrace(openBrace);
	}

	while (peekToken().type != Token::RightBrace) {
        block->statements.push_back(parseStatement());
	}

    Token closeBrace = readToken();
    if (closeBrace.type != Token::RightBrace) {
        throw MissingClosingBrace(closeBrace);
	}

	return block;
}

/**
 * @brief Parser::parseFunction
 * @return
 */
std::unique_ptr<CallStatement> Parser::parseCallStatement() {

    if(in_function_) {
        throw FunctionDefinedWithinFunction(peekToken());
    }

    in_function_ = true;

    Token name = readToken();
    if(name.type != Token::Identifier) {
        throw MissingIdentifier(name);
    }

    // consume any newlines
    while(peekToken().type == Token::Newline) {
        readToken();
    }

    std::unique_ptr<BlockStatement> body = parseBlockStatement();
    auto function = std::make_unique<CallStatement>();

    function->name       = name.value;
    function->statements = std::move(body->statements);

    in_function_ = false;

    return function;
}

/**
 * @brief Parser::parseStatement
 * @return
 */
std::unique_ptr<Statement> Parser::parseStatement() {

	Token token = peekToken();

	switch (token.type) {
	default:
		assert(0);
    case Token::Delete:
        token = readToken();
        return parseDeleteStatement();
    case Token::Return:
        token = readToken();
        return parseReturnStatement();
	case Token::LeftBrace:
		return parseBlockStatement();
	case Token::While:
		token = readToken();
		return parseWhileStatement();
	case Token::For:
		token = readToken();
		return parseForStatement();
	case Token::If:
		token = readToken();
		return parseIfStatement();
	case Token::Identifier:
		return parseExpressionStatement();
    case Token::Increment:
    case Token::Decrement:
        return parseExpressionStatement();
    case Token::Newline:
        // empty expression
        token = readToken();
        return std::make_unique<ExpressionStatement>();
    case Token::Else:
    case Token::In:
        throw UnexpectedKeyword(token);
	case Token::RightParen:
	case Token::LeftParen:
        throw UnexpectedParen(token);
	case Token::RightBrace:
        throw UnexpectedBrace(token);
    case Token::LeftBracket:
        throw UnexpectedBracket(token);
	case Token::String:
        throw UnexpectedStringConstant(token);
    case Token::Break:{
        Token brk  = readToken();
        Token nl = readToken();
        if (nl.type != Token::Newline) {
            throw MissingNewline(nl);
		}
        return std::make_unique<BreakStatement>();
    }
    case Token::Continue: {
        Token cont = readToken();
        Token nl = readToken();
        if (nl.type != Token::Newline) {
            throw MissingNewline(nl);
        }
        return std::make_unique<ContinueStatement>();
    }
	case Token::Define:
        token = readToken();
        return parseCallStatement();
	case Token::Invalid:
		// no more tokens
		return nullptr;
	}
}

/**
 * @brief Parser::parseExpression0
 * @param exp
 */
void Parser::parseExpression0(std::unique_ptr<Expression> &exp) {

	Token identifier = peekToken();

	// =, +=, -=, *=, /=, %=, &=, |=
	if (identifier.type == Token::Identifier) {

        size_t currentIndex = tokenIndex();

        // TODO(eteran): support array index assignment
        auto lhs = std::make_unique<Expression>();
        parseExpression1(lhs);

		Token op = peekToken();

		if (op.type == Token::Assign || op.type == Token::AddAssign || op.type == Token::SubAssign || op.type == Token::MulAssign || op.type == Token::DivAssign || op.type == Token::ModAssign) {
			op = readToken();

			BinaryExpression bin;

            bin.lhs = std::move(lhs);
			bin.op  = op.type;
            bin.rhs = std::make_unique<Expression>();

			parseExpression0(bin.rhs);
            *exp = std::move(bin);
		} else {
            setTokenIndex(currentIndex);
		}
	}

	parseExpression1(exp);
}

/**
 * @brief Parser::parseExpression1
 * @param exp
 */
void Parser::parseExpression1(std::unique_ptr<Expression> &exp) {

	// (concatenation)
	// NOTE(eteran): this "operator when there is no operator" is a very poor
	// design choice IMO and requires a bit of hackyness to do correctly :-(
	// would have been much better if they did something like: "hello " . "world"
	// (using something like a dot operator to mean concat, avoid + makes sense
	// since integers and strings get implicitly converted between each other)

	parseExpression2(exp);

    Token op = peekToken();
    while(op.type == Token::LeftParen || op.type == Token::Identifier || op.type == Token::Integer || op.type == Token::String) {
        // NOTE(eteran): NOT a readToken() like the rest, since there is no actual operator in the code!
        // op = readToken();

        BinaryExpression bin;

        bin.lhs = std::make_unique<Expression>();
        bin.op  = Token::Concatenate;
        bin.rhs = std::make_unique<Expression>();

        // upgrade the expression we have already to being an LHS value
        *bin.lhs = std::move(*exp);

        // parse the RHS expression
        parseExpression1(bin.rhs);

        *exp = std::move(bin);
        op   = peekToken();
    }
}

/**
 * @brief Parser::parseExpression2
 * @param exp
 */
void Parser::parseExpression2(std::unique_ptr<Expression> &exp) {
	// ||

	parseExpression3(exp);

	Token op = peekToken();

	while (op.type == Token::LogicalOr) {

		op = readToken();

		BinaryExpression bin;

        bin.lhs = std::make_unique<Expression>();
		bin.op  = op.type;
        bin.rhs = std::make_unique<Expression>();

		// upgrade the expression we have already to being an LHS value
        *bin.lhs = std::move(*exp);

		// parse the RHS expression
		parseExpression2(bin.rhs);

        *exp = std::move(bin);
		op   = peekToken();
	}
}

/**
 * @brief Parser::parseExpression3
 * @param exp
 */
void Parser::parseExpression3(std::unique_ptr<Expression> &exp) {
	// &&

	parseExpression4(exp);

	Token op = peekToken();

	while (op.type == Token::LogicalAnd) {

		op = readToken();

		BinaryExpression bin;

        bin.lhs = std::make_unique<Expression>();
		bin.op  = op.type;
        bin.rhs = std::make_unique<Expression>();

		// upgrade the expression we have already to being an LHS value
        *bin.lhs = std::move(*exp);

		// parse the RHS expression
		parseExpression3(bin.rhs);

        *exp = std::move(bin);
        op   = peekToken();
	}
}

/**
 * @brief Parser::parseExpression4
 * @param exp
 */
void Parser::parseExpression4(std::unique_ptr<Expression> &exp) {
	// |

	parseExpression5(exp);

	Token op = peekToken();

	while (op.type == Token::BinaryOr) {

		op = readToken();

		BinaryExpression bin;

        bin.lhs = std::make_unique<Expression>();
		bin.op  = op.type;
        bin.rhs = std::make_unique<Expression>();

		// upgrade the expression we have already to being an LHS value
        *bin.lhs = std::move(*exp);

		// parse the RHS expression
		parseExpression4(bin.rhs);

        *exp = std::move(bin);
        op   = peekToken();
	}
}

/**
 * @brief Parser::parseExpression5
 * @param exp
 */
void Parser::parseExpression5(std::unique_ptr<Expression> &exp) {
	// &

	parseExpression6(exp);

	Token op = peekToken();

	while (op.type == Token::BinaryAnd) {

		op = readToken();

		BinaryExpression bin;

        bin.lhs = std::make_unique<Expression>();
		bin.op  = op.type;
        bin.rhs = std::make_unique<Expression>();

		// upgrade the expression we have already to being an LHS value
        *bin.lhs = std::move(*exp);

		// parse the RHS expression
		parseExpression5(bin.rhs);

        *exp = std::move(bin);
        op   = peekToken();
	}
}

/**
 * @brief Parser::parseExpression6
 * @param exp
 */
void Parser::parseExpression6(std::unique_ptr<Expression> &exp) {
    // >=, >, <, <=, ==, !=, in

    // NOTE(eteran): according to NEDIT sources "in" shares priority with these

    parseExpression7(exp);

	Token op = peekToken();

    while (op.type == Token::In || op.type == Token::GreaterThan || op.type == Token::GreaterThanOrEqual || op.type == Token::LessThan || op.type == Token::LessThanOrEqual || op.type == Token::Equal || op.type == Token::NotEqual) {

		op = readToken();

		BinaryExpression bin;

        bin.lhs = std::make_unique<Expression>();
		bin.op  = op.type;
        bin.rhs = std::make_unique<Expression>();

		// upgrade the expression we have already to being an LHS value
        *bin.lhs = std::move(*exp);

		// parse the RHS expression
		parseExpression6(bin.rhs);

        *exp = std::move(bin);
        op   = peekToken();
	}
}

/**
 * @brief Parser::parseExpression7
 * @param exp
 */
void Parser::parseExpression7(std::unique_ptr<Expression> &exp) {
	// +, -

	parseExpression8(exp);

	Token op = peekToken();

	while (op.type == Token::Add || op.type == Token::Sub) {
		op = readToken();

		BinaryExpression bin;

        bin.lhs = std::make_unique<Expression>();
		bin.op  = op.type;
        bin.rhs = std::make_unique<Expression>();

		// upgrade the expression we have already to being an LHS value
        *bin.lhs = std::move(*exp);

		// parse the RHS expression
		parseExpression7(bin.rhs);

        *exp = std::move(bin);
        op   = peekToken();
	}
}

/**
 * @brief Parser::parseExpression8
 * @param exp
 */
void Parser::parseExpression8(std::unique_ptr<Expression> &exp) {
	// *, /, %

	parseExpression9(exp);

	Token op = peekToken();
	while (op.type == Token::Mul || op.type == Token::Div || op.type == Token::Mod) {
		op = readToken();

		BinaryExpression bin;

        bin.lhs = std::make_unique<Expression>();
		bin.op  = op.type;
        bin.rhs = std::make_unique<Expression>();

		// upgrade the expression we have already to being an LHS value
        *bin.lhs = std::move(*exp);

		// parse the RHS expression
		parseExpression8(bin.rhs);

        *exp = std::move(bin);
        op   = peekToken();
	}
}

/**
 * @brief Parser::parseExpression9
 * @param exp
 */
void Parser::parseExpression9(std::unique_ptr<Expression> &exp) {

    // -, !, ++, -- (unary)
    Token op = peekToken();
    while (op.type == Token::Increment || op.type == Token::Decrement || op.type == Token::Sub || op.type == Token::Not) {
        op = readToken();

        UnaryExpression expr;

        expr.op        = op.type;
        expr.operand   = std::make_unique<Expression>();
        expr.prefix    = true;

        // parse the operand expression
        parseExpression9(expr.operand);

        *exp = std::move(expr);
        op   = peekToken();
    }

    parseExpression10(exp);

    // ++, -- (postfix)
    op = peekToken();
    while (op.type == Token::Increment || op.type == Token::Decrement) {
        op = readToken();

        UnaryExpression expr;

        expr.op      = op.type;
        expr.operand = std::make_unique<Expression>();
        expr.prefix  = false;

        // upgrade the expression we have already to being an LHS value
        *expr.operand = std::move(*exp);

        *exp = std::move(expr);
        op   = peekToken();
    }
}

/**
 * @brief Parser::parseExpression10
 * @param exp
 */
void Parser::parseExpression10(std::unique_ptr<Expression> &exp) {
	// ^ (power)

	parseExpression11(exp);

    // NOTE(eteran): we don't loop, as that would make things left-right
    //               associative, but this operator is right-left associative
	Token op = peekToken();
    if (op.type == Token::Exponent) {
		op = readToken();

		BinaryExpression bin;

        bin.lhs = std::make_unique<Expression>();
		bin.op  = op.type;
        bin.rhs = std::make_unique<Expression>();

		// upgrade the expression we have already to being an LHS value
        *bin.lhs = std::move(*exp);

		// parse the RHS expression
		parseExpression10(bin.rhs);

        *exp = std::move(bin);
        op   = peekToken();
	}
}

/**
 * @brief Parser::parseExpression11
 * @param exp
 */
void Parser::parseExpression11(std::unique_ptr<Expression> &exp) {
	// ()

	Token token = peekToken();

    if(token.type == Token::LeftParen) {
        token = readToken();

        // get sub-expression
        parseExpression0(exp);

        Token closeParen = readToken();
        if (closeParen.type != Token::RightParen) {
            throw MissingClosingParen(closeParen);
        }
    } else {
        parseArrayIndex(exp);
    }
}

/**
 * @brief Parser::parseArrayIndex
 * @param exp
 */
void Parser::parseArrayIndex(std::unique_ptr<Expression> &exp) {

    parseAtom(exp);

    Token leftBracket = peekToken();
    while(leftBracket.type == Token::LeftBracket) {

        // consume the left bracket
        readToken();

        std::unique_ptr<Expression> index = parseExpression();

        Token closeBracket = readToken();
        if(closeBracket.type != Token::RightBracket) {
            throw MissingClosingBracket(closeBracket);
        }

        ArrayIndexExpression arrayIndex;
        arrayIndex.array = std::make_unique<Expression>();
        arrayIndex.index = std::move(index);

        *(arrayIndex.array) = std::move(*exp);

        *exp = std::move(arrayIndex);

        leftBracket = peekToken();
    }

    parseCall(exp);
}

/**
 * @brief Parser::parseAtom
 * @param exp
 */
void Parser::parseAtom(std::unique_ptr<Expression> &exp) {
    // var, $var, 123, "hello"

	Token token = peekToken();

    if (token.type == Token::Identifier || token.type == Token::Integer || token.type == Token::String) {
        Token name  = readToken();

        AtomExpression atom;
        atom.atom = name.value;
        *exp = atom;
	}
}



/**
 * @brief Parser::parseCall
 * @param exp
 */
void Parser::parseCall(std::unique_ptr<Expression> &exp) {
    Token leftBracket = peekToken();
    if(leftBracket.type == Token::LeftParen) {

        // consume the left parens
        readToken();

        if(peekToken().type == Token::RightParen) {
            // empty parameter list
            // consume the closing parameter
            readToken();

            CallExpression call;
            call.function   = std::move(exp);

            exp = std::make_unique<Expression>();
            *exp = std::move(call);

        } else {
            std::vector<std::unique_ptr<Expression>> arguments = parseExpressionList();

            Token closeParen = readToken();
            if(closeParen.type != Token::RightParen) {
                throw MissingClosingParen(closeParen);
            }

            CallExpression call;
            call.function   = std::move(exp);
            call.parameters = std::move(arguments);

            exp = std::make_unique<Expression>();
            *exp = std::move(call);
        }
    }
}

/**
 * @brief Parser::parseExpressionList
 * @return
 */
std::vector<std::unique_ptr<Expression>> Parser::parseExpressionList() {
    std::vector<std::unique_ptr<Expression>> expressions;

    while(true) {
        auto expr = parseExpression();
        expressions.push_back(std::move(expr));

        if(peekToken().type != Token::Comma) {
            break;
        }

        // consume the comma
        readToken();
    }

    return expressions;
}
