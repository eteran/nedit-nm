
#include "Parser.h"
#include "Error.h"
#include "Expression.h"
#include "Input.h"
#include "Statement.h"
#include "Tokenizer.h"
#include <memory>

/**
 * @brief Parser::Parser
 */
Parser::Parser(const std::string &filename) : tokenizer_(filename) {
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
 * @brief Parser::parseForStatement
 * @return
 */
std::unique_ptr<Statement> Parser::parseForStatement() {

    consumeRequired<SyntaxError>(Token::For);
    consumeRequired<MissingOpenParen>(Token::LeftParen);

    std::vector<std::unique_ptr<Expression>> init_exprs = parseExpressionList();

    if (peekToken().type == Token::Semicolon) {
		// standard C-style FOR loop

		// consume the semicolon now that we are sure
        consumeRequired<MissingSemicolon>(Token::Semicolon);

		auto cond = parseExpression();

        consumeRequired<MissingSemicolon>(Token::Semicolon);

        std::vector<std::unique_ptr<Expression>> incr_exprs = parseExpressionList();

        consumeRequired<MissingClosingParen>(Token::RightParen);

		// consume any newlines
		while (peekToken().type == Token::Newline) {
			readToken();
		}

		auto body = parseStatement();

		auto loop   = std::make_unique<LoopStatement>();
        loop->body  = std::move(body);
        loop->init  = std::move(init_exprs);
        loop->incr  = std::move(incr_exprs);
        loop->cond  = std::move(cond);

		return loop;
    }


    // if we didn't get a semicolon, then we better have a "if(x in y)" expression
    if(init_exprs.size() == 1) {
        auto &init = init_exprs[0];
        if (BinaryExpression *expr = dynamic_cast<BinaryExpression *>(init.get())) {
            if (expr->op == Token::In) {
                auto container = std::move(expr->rhs);
                auto iterator  = std::move(expr->lhs);

                consumeRequired<MissingClosingParen>(Token::RightParen);

                // consume any newlines
                while (peekToken().type == Token::Newline) {
                    readToken();
                }
                auto body = parseStatement();

                auto loop       = std::make_unique<ForEachStatement>();
                loop->iterator  = std::move(iterator);
                loop->container = std::move(container);
                loop->body      = std::move(body);

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

    consumeRequired<SyntaxError>(Token::If);
    consumeRequired<MissingOpenParen>(Token::LeftParen);

    auto condition = parseExpression();

    consumeRequired<MissingClosingParen>(Token::RightParen);

	// consume any newlines
	while (peekToken().type == Token::Newline) {
		readToken();
	}

	auto body = parseStatement();

	auto cond   = std::make_unique<CondStatement>();
    cond->body  = std::move(body);
    cond->cond  = std::move(condition);

	// consume any newlines
	while (peekToken().type == Token::Newline) {
		readToken();
	}

	if (peekToken().type == Token::Else) {
		readToken();

		// consume any newlines
		while (peekToken().type == Token::Newline) {
			readToken();
		}

		cond->else_ = parseStatement();
	}

	return cond;
}

/**
 * @brief parseBreakStatement
 * @return
 */
std::unique_ptr<BreakStatement> Parser::parseBreakStatement() {

    consumeRequired<SyntaxError>(Token::Break);
    consumeRequired<MissingNewline>(Token::Newline);

	return std::make_unique<BreakStatement>();
}

/**
 * @brief parseContinueStatement
 * @return
 */
std::unique_ptr<ContinueStatement> Parser::parseContinueStatement() {

    consumeRequired<SyntaxError>(Token::Continue);
    consumeRequired<MissingNewline>(Token::Newline);

	return std::make_unique<ContinueStatement>();
}

/**
 * @brief Parser::parseDeleteStatement
 * @return
 */
std::unique_ptr<DeleteStatement> Parser::parseDeleteStatement() {

    consumeRequired<SyntaxError>(Token::Delete);

	auto expr = parseExpression();

    if(auto indexExpression = dynamic_cast<ArrayIndexExpression *>(expr.get())) {
        auto stmt        = std::make_unique<DeleteStatement>();
        stmt->expression = std::move(indexExpression->array);
        stmt->index      = std::move(indexExpression->index);

        return stmt;
    }


    throw InvalidDelete(peekToken());

}

/**
 * @brief Parser::parseReturnStatement
 * @return
 */
std::unique_ptr<ReturnStatement> Parser::parseReturnStatement() {

    consumeRequired<SyntaxError>(Token::Return);

	auto expr       = parseExpression();
	auto ret        = std::make_unique<ReturnStatement>();
	ret->expression = std::move(expr);

	return ret;
}

/**
 * @brief Parser::parseExpressionStatement
 * @return
 */
std::unique_ptr<ExpressionStatement> Parser::parseExpressionStatement() {

	if (auto expression = parseExpression()) {
		auto statement        = std::make_unique<ExpressionStatement>();
		statement->expression = std::move(expression);

        consumeRequired<MissingNewline>(Token::Newline);

		// consume any newlines
		while (peekToken().type == Token::Newline) {
			readToken();
		}

		return statement;
	}

	return nullptr;
}

/**
 * @brief Parser::parseEmptyStatement
 * @return
 */
std::unique_ptr<ExpressionStatement> Parser::parseEmptyStatement() {
	// empty expression

    consumeRequired<MissingNewline>(Token::Newline);

    // consume any newlines
	while (peekToken().type == Token::Newline) {
		readToken();
	}

	return std::make_unique<ExpressionStatement>();
}

/**
 * @brief Parser::parseWhileStatement
 * @return
 */
std::unique_ptr<LoopStatement> Parser::parseWhileStatement() {

    consumeRequired<SyntaxError>(Token::While);
    consumeRequired<MissingOpenParen>(Token::LeftParen);

	auto condition = parseExpression();

    consumeRequired<MissingClosingParen>(Token::RightParen);

	// consume any newlines
	while (peekToken().type == Token::Newline) {
		readToken();
	}

	auto body = parseStatement();

    auto loop  = std::make_unique<LoopStatement>();
    loop->body = std::move(body);
    loop->cond = std::move(condition);

	return loop;
}

/**
 * @brief Parser::parseBlockStatement
 * @return
 */
std::unique_ptr<BlockStatement> Parser::parseBlockStatement() {

    consumeRequired<MissingOpenBrace>(Token::LeftBrace);

    auto block = std::make_unique<BlockStatement>();

    while (peekToken().type != Token::RightBrace) {
        block->statements.push_back(parseStatement());
    }

    consumeRequired<MissingClosingBrace>(Token::RightBrace);

	return block;
}

/**
 * @brief Parser::parseFunction
 * @return
 */
std::unique_ptr<FunctionStatement> Parser::parseFunctionStatement() {

    consumeRequired<SyntaxError>(Token::Define);

	if (in_function_) {
		throw FunctionDefinedWithinFunction(peekToken());
	}

	in_function_ = true;

	Token name = readToken();
	if (name.type != Token::Identifier) {
		throw MissingIdentifier(name);
	}

	// consume any newlines
	while (peekToken().type == Token::Newline) {
		readToken();
	}

	std::unique_ptr<BlockStatement> body     = parseBlockStatement();
	auto                            function = std::make_unique<FunctionStatement>();

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
		throw UnexpectedKeyword(token);
	case Token::Delete:
		return parseDeleteStatement();
	case Token::Return:
		return parseReturnStatement();
	case Token::LeftBrace:
		return parseBlockStatement();
	case Token::While:
		return parseWhileStatement();
	case Token::For:
		return parseForStatement();
	case Token::If:
		return parseIfStatement();
	case Token::Identifier:
	case Token::Increment:
	case Token::Decrement:
		return parseExpressionStatement();
	case Token::Newline:
		return parseEmptyStatement();
	case Token::RightParen:
	case Token::LeftParen:
		throw UnexpectedParen(token);
	case Token::RightBrace:
		throw UnexpectedBrace(token);
	case Token::LeftBracket:
		throw UnexpectedBracket(token);
	case Token::String:
		throw UnexpectedStringConstant(token);
	case Token::Break:
		return parseBreakStatement();
	case Token::Continue:
		return parseContinueStatement();
	case Token::Define:
		return parseFunctionStatement();
	case Token::Invalid:
		// no more tokens
		return nullptr;
	}
}

/**
 * @brief Parser::parseExpression
 * @return
 */
std::unique_ptr<Expression> Parser::parseExpression() {

    std::unique_ptr<Expression> expr;
    parseExpression0(expr);
    return expr;
}

/**
 * @brief Parser::parseExpression0
 * @param exp
 */
void Parser::parseExpression0(std::unique_ptr<Expression> &exp) {

	// =, +=, -=. *=, /=, %=

	parseExpression1(exp);

	Token op = peekToken();

	if (op.type == Token::Assign || op.type == Token::AddAssign || op.type == Token::SubAssign || op.type == Token::MulAssign || op.type == Token::DivAssign || op.type == Token::ModAssign) {

		op = readToken();

        auto bin = std::make_unique<BinaryExpression>();

        bin->lhs = std::move(exp);
        bin->op  = op.type;

		// parse the RHS expression
        parseExpression0(bin->rhs);

        exp = std::move(bin);
        op  = peekToken();
	}
}

/**
 * @brief Parser::parseExpression1
 * @param exp
 */
void Parser::parseExpression1(std::unique_ptr<Expression> &exp) {

	// (concatenation)
	// NOTE(eteran): this "operator when there is no operator" is a very poor
    // design choice IMO, and requires a bit of hackyness to do correctly :-(
    // it would have been much better if they did something like: "hello " . "world"
	// (using something like a dot operator to mean concat, avoid + makes sense
	// since integers and strings get implicitly converted between each other)

	parseExpression2(exp);

	Token op = peekToken();
	while (op.type == Token::LeftParen || op.type == Token::Identifier || op.type == Token::Integer || op.type == Token::String) {
		// NOTE(eteran): NOT a readToken() like the rest, since there is no actual operator in the code!
		// op = readToken();

        auto bin = std::make_unique<BinaryExpression>();

        bin->lhs = std::move(exp);
        bin->op  = Token::Concatenate;

		// parse the RHS expression
        parseExpression1(bin->rhs);

        exp = std::move(bin);
        op  = peekToken();
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

        auto bin = std::make_unique<BinaryExpression>();

        bin->lhs = std::move(exp);
        bin->op  = op.type;

		// parse the RHS expression
        parseExpression2(bin->rhs);

        exp = std::move(bin);
        op  = peekToken();
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

        auto bin = std::make_unique<BinaryExpression>();

        bin->lhs = std::move(exp);
        bin->op  = op.type;

		// parse the RHS expression
        parseExpression3(bin->rhs);

        exp = std::move(bin);
        op  = peekToken();
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

        auto bin = std::make_unique<BinaryExpression>();

        bin->lhs = std::move(exp);
        bin->op  = op.type;

        // parse the RHS expression
        parseExpression4(bin->rhs);

        exp = std::move(bin);
        op  = peekToken();
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

        auto bin = std::make_unique<BinaryExpression>();

        bin->lhs = std::move(exp);
        bin->op  = op.type;

		// parse the RHS expression
        parseExpression5(bin->rhs);

        exp = std::move(bin);
        op  = peekToken();
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

        auto bin = std::make_unique<BinaryExpression>();

        bin->lhs = std::move(exp);
        bin->op  = op.type;

		// parse the RHS expression
        parseExpression6(bin->rhs);

        exp = std::move(bin);
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

        auto bin = std::make_unique<BinaryExpression>();

        bin->lhs = std::move(exp);
        bin->op  = op.type;

		// parse the RHS expression
        parseExpression7(bin->rhs);

        exp = std::move(bin);
        op  = peekToken();
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

        auto bin = std::make_unique<BinaryExpression>();

        bin->lhs = std::move(exp);
        bin->op  = op.type;

		// parse the RHS expression
        parseExpression8(bin->rhs);

        exp = std::move(bin);
        op  = peekToken();
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

        auto unary = std::make_unique<UnaryExpression>();

        unary->op      = op.type;
        unary->prefix  = true;

		// parse the operand expression
        parseExpression9(unary->operand);

        exp = std::move(unary);
        op  = peekToken();
	}

	parseExpression10(exp);

	// ++, -- (postfix)
	op = peekToken();
	while (op.type == Token::Increment || op.type == Token::Decrement) {
		op = readToken();

        auto unary = std::make_unique<UnaryExpression>();

        unary->op      = op.type;
        unary->operand = std::move(exp);
        unary->prefix  = false;

        exp = std::move(unary);
        op  = peekToken();
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

        auto bin = std::make_unique<BinaryExpression>();

        bin->lhs = std::move(exp);
        bin->op  = op.type;

		// parse the RHS expression
        parseExpression10(bin->rhs);

        exp = std::move(bin);
        op  = peekToken();
	}
}

/**
 * @brief Parser::parseExpression11
 * @param exp
 */
void Parser::parseExpression11(std::unique_ptr<Expression> &exp) {
	// ()

	Token token = peekToken();

	if (token.type == Token::LeftParen) {
		token = readToken();

		// get sub-expression
		parseExpression0(exp);

        consumeRequired<MissingClosingParen>(Token::RightParen);
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
	while (leftBracket.type == Token::LeftBracket) {

		// consume the left bracket
		readToken();

        std::vector<std::unique_ptr<Expression>> index = parseExpressionList();

        consumeRequired<MissingClosingBracket>(Token::RightBracket);

        auto arrayIndex = std::make_unique<ArrayIndexExpression>();

        arrayIndex->array = std::move(exp);
        arrayIndex->index = std::move(index);

        exp = std::move(arrayIndex);

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
		Token name = readToken();

        auto atom = std::make_unique<AtomExpression>();
        atom->value = name.value;
        atom->type  = name.type;
        exp         = std::move(atom);
    }
}

/**
 * @brief Parser::parseCall
 * @param exp
 */
void Parser::parseCall(std::unique_ptr<Expression> &exp) {
	Token leftBracket = peekToken();
	if (leftBracket.type == Token::LeftParen) {

        if(auto a = dynamic_cast<AtomExpression *>(exp.get())) {
            if(a->type == Token::Type::Identifier) {

                // consume the left parens
                readToken();

                if (peekToken().type == Token::RightParen) {
                    // empty parameter list
                    // consume the closing parameter
                    consumeRequired<MissingClosingParen>(Token::RightParen);

                    auto call = std::make_unique<CallExpression>();
                    call->function = std::move(exp);

                    exp = std::move(call);

                } else {
                    std::vector<std::unique_ptr<Expression>> arguments = parseExpressionList();

                    consumeRequired<MissingClosingParen>(Token::RightParen);

                    auto call = std::make_unique<CallExpression>();
                    call->function   = std::move(exp);
                    call->parameters = std::move(arguments);

                    exp = std::move(call);
                }
            }
        }
	}
}

/**
 * @brief Parser::parseExpressionList
 * @return
 */
std::vector<std::unique_ptr<Expression>> Parser::parseExpressionList() {
	std::vector<std::unique_ptr<Expression>> expressions;

	while (true) {
		auto expr = parseExpression();

        if(expr) {
            expressions.push_back(std::move(expr));
        } else {
            if(peekToken().type == Token::Comma) {
                throw UnexpectedComma(peekToken());
            }
        }

		if (peekToken().type != Token::Comma) {
			break;
		}

		// consume the comma
		readToken();
	}

	return expressions;
}
