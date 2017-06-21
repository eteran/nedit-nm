
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
    std::unique_ptr<Expression> expression;
    std::unique_ptr<Expression> index;
};

class CallStatement : public Statement {
public:
    std::string name;
    std::vector<std::unique_ptr<Statement>> statements;
};

class BlockStatement : public Statement {
public:
    std::vector<std::unique_ptr<Statement>> statements;
};

class CondStatement : public Statement {
public:
    std::unique_ptr<Expression> cond_;
    std::unique_ptr<Statement>  body_;
    std::unique_ptr<Statement>  else_;
};

class LoopStatement : public Statement {
public:
    std::unique_ptr<Expression> init_;
    std::unique_ptr<Expression> cond_;
    std::unique_ptr<Expression> incr_;
    std::unique_ptr<Statement>  body_;
};

class ForEachStatement : public Statement {
public:
    std::unique_ptr<Expression> iterator;
    std::unique_ptr<Expression> container;
    std::unique_ptr<Statement>  body_;
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
