
#ifndef PARSER_H_
#define PARSER_H_

#include "Expression.h"
#include "Statement.h"
#include "Tokenizer.h"
#include <memory>
#include <string>

class Input;
class Token;

class Parser {
public:
	explicit Parser(const std::string &filename);
	~Parser() = default;

public:
    std::unique_ptr<BlockStatement>          parseBlockStatement();
    std::unique_ptr<BreakStatement>          parseBreakStatement();
    std::unique_ptr<CondStatement>           parseIfStatement();
    std::unique_ptr<ContinueStatement>       parseContinueStatement();
    std::unique_ptr<DeleteStatement>         parseDeleteStatement();
    std::unique_ptr<Expression>              parseExpression();
    std::unique_ptr<ExpressionStatement>     parseEmptyStatement();
    std::unique_ptr<ExpressionStatement>     parseExpressionStatement();
    std::unique_ptr<FunctionStatement>       parseFunctionStatement();
    std::unique_ptr<LoopStatement>           parseWhileStatement();
    std::unique_ptr<ReturnStatement>         parseReturnStatement();
    std::unique_ptr<Statement>               parseForStatement();
    std::unique_ptr<Statement>               parseStatement();
    std::vector<std::unique_ptr<Expression>> parseExpressionList();

private:
    void parseExpression0(std::unique_ptr<Expression> &exp);
    void parseExpression1(std::unique_ptr<Expression> &exp);
    void parseExpression2(std::unique_ptr<Expression> &exp);
    void parseExpression3(std::unique_ptr<Expression> &exp);
    void parseExpression4(std::unique_ptr<Expression> &exp);
    void parseExpression5(std::unique_ptr<Expression> &exp);
    void parseExpression6(std::unique_ptr<Expression> &exp);
    void parseExpression7(std::unique_ptr<Expression> &exp);
    void parseExpression8(std::unique_ptr<Expression> &exp);
    void parseExpression9(std::unique_ptr<Expression> &exp);
    void parseExpression10(std::unique_ptr<Expression> &exp);
    void parseExpression11(std::unique_ptr<Expression> &exp);
    void parseAtom(std::unique_ptr<Expression> &exp);
    void parseArrayIndex(std::unique_ptr<Expression> &exp);
	void parseCall(std::unique_ptr<Expression> &exp);

private:
	std::string readIdentifier();
    Token  peekToken() const;
    Token  readToken();

private:
    template <class Ex>
    void consumeRequired(Token::Type type) {
        auto token = readToken();
        if (token.type != type) {
            throw Ex(token);
        }
    }

private:
	Tokenizer tokenizer_;
	size_t    index_;
	bool      in_function_;
};

#endif
