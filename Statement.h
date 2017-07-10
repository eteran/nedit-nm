
#ifndef STATEMENT_H_
#define STATEMENT_H_

#include <memory>
#include <vector>

class Expression;

class Statement {
public:
	virtual ~Statement() = default;
};

class DeleteStatement : public Statement {
public:
    std::unique_ptr<Expression>              expression;
    std::vector<std::unique_ptr<Expression>> index;
};

class FunctionStatement : public Statement {
public:
	std::string                             name;
	std::vector<std::unique_ptr<Statement>> statements;
};

class BlockStatement : public Statement {
public:
	std::vector<std::unique_ptr<Statement>> statements;
};

class CondStatement : public Statement {
public:
    std::unique_ptr<Expression> cond;
    std::unique_ptr<Statement>  body;
	std::unique_ptr<Statement>  else_;
};

class LoopStatement : public Statement {
public:
    std::vector<std::unique_ptr<Expression>> init;
    std::unique_ptr<Expression>              cond;
    std::vector<std::unique_ptr<Expression>> incr;
    std::unique_ptr<Statement>               body;
};

class ForEachStatement : public Statement {
public:
	std::unique_ptr<Expression> iterator;
	std::unique_ptr<Expression> container;
    std::unique_ptr<Statement>  body;
};

class BreakStatement : public Statement {};

class ContinueStatement : public Statement {};

class ExpressionStatement : public Statement {
public:
	std::unique_ptr<Expression> expression;
};

class ReturnStatement : public Statement {
public:
	std::unique_ptr<Expression> expression;
};

#endif
