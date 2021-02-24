
#include "CodeGenerator.h"
#include "Expression.h"
#include "Statement.h"
#include <boost/variant.hpp>
#include <climits>
#include <list>
#include <stack>

namespace {

struct Node {
	int64_t location;
	std::string instr;
};

struct BranchNode {
	int64_t location;
	std::string instr;
	int64_t target = LONG_LONG_MAX; // default to blatently invalid
};

struct AssignNode {
	int64_t location;
	std::string instr;
	std::string symbol;
};

struct PushSymbolNode {
	int64_t location;
	std::string instr;
	std::string symbol;
};

struct PushStringNode {
	int64_t location;
	std::string instr;
	std::string string;
};

struct PushArraySymbolNode {
	int64_t location;
	std::string instr;
	std::string symbol;
	std::string suffix;
};

struct ArrayOpNode {
	int64_t location;
	std::string instr;
	size_t dimensions;
};

struct CallNode {
	int64_t location;
	std::string instr;
	std::string target;
	size_t args;
};

using node_type = boost::variant<Node, BranchNode, AssignNode, PushSymbolNode, PushStringNode, PushArraySymbolNode, ArrayOpNode, CallNode>;

struct Visitor {
	void operator()(const Node &node) const {
		printf("%-16ld %s\n", node.location, node.instr.c_str());
	}

	void operator()(const BranchNode &node) const {
		printf("%-16ld %s to=(%+ld)\n", node.location, node.instr.c_str(), node.target);
	}

	void operator()(const AssignNode &node) const {
		printf("%-16ld %s %s\n", node.location, node.instr.c_str(), node.symbol.c_str());
	}

	void operator()(const PushSymbolNode &node) const {
		printf("%-16ld %s %s\n", node.location, node.instr.c_str(), node.symbol.c_str());
	}

	void operator()(const PushArraySymbolNode &node) const {
		printf("%-16ld %s %s %s\n", node.location, node.instr.c_str(), node.symbol.c_str(), node.suffix.c_str());
	}

	void operator()(const ArrayOpNode &node) const {
		printf("%-16ld %s nDim=%lu\n", node.location, node.instr.c_str(), node.dimensions);
	}

	void operator()(const CallNode &node) const {
		printf("%-16ld %s %s (%lu arg)\n", node.location, node.instr.c_str(), node.target.c_str(), node.args);
	}

	void operator()(const PushStringNode &node) const {
		if (node.string.size() > 20) {
			printf("%-16ld %s <%lu> \"%s\"\n", node.location, node.instr.c_str(), node.string.size(), escape_string(node.string.substr(0, 20)).c_str());
		} else {
			printf("%-16ld %s <%lu> \"%s\"...\n", node.location, node.instr.c_str(), node.string.size(), escape_string(node.string.substr(0, 20)).c_str());
		}
	}

private:
	static std::string escape_string(const std::string &s) {

		std::string r;

		for (char ch : s) {
			switch (ch) {
			case '\n':
				r.append("\\n");
				break;
			case '\t':
				r.append("\\t");
				break;
			case '\"':
				r.append("\\\"");
				break;
			default:
				r.push_back(ch);
				break;
			}
		}

		return r;
	}
};

struct LoopContext {
	const LoopStatement *loop;
	std::vector<BranchNode *> continues;
	std::vector<BranchNode *> breaks;
};

// NOTE(eteran): important to use std::list so addresses are stable
std::list<node_type> nodes;
std::stack<LoopContext> loopStack;

int in_binary_expression = 0;

void generate_ir(const std::unique_ptr<Expression> &expression);
void generate_ir(const std::unique_ptr<Statement> &statement);
void generate_ir(const Expression *expression);
void generate_ir(const Statement *statement);
void generate_ir(const ExpressionStatement *statement);
void generate_ir(const std::vector<std::unique_ptr<Statement>> &statements);

/**
 * @brief current_location
 * @return
 */
int64_t current_location() {
	return static_cast<int64_t>(nodes.size());
}

/**
 * @brief to_string
 * @param statement
 * @return
 */
std::string to_string(const std::unique_ptr<Expression> &statement) {
	if (auto atom_expression = dynamic_cast<AtomExpression *>(statement.get())) {
		return atom_expression->value;
	}

	printf("(to_string) EXPRESSION - UNHANDLED\n");
	abort();
}

/**
 * @brief emit_node
 * @param args
 * @return
 */
template <class T, class... Args>
T *emit_node(Args... args) {
	int64_t index = current_location();
	nodes.emplace_back(T{index, std::forward<Args>(args)...});
	return &boost::get<T>(nodes.back());
}

/**
 * @brief c_emit_node
 * @param enabled
 * @param args
 * @return
 */
template <class T, class... Args>
T *c_emit_node(bool enabled, Args... args) {
	if (enabled) {
		return emit_node<T>(std::forward<Args>(args)...);
	}

	return nullptr;
}

/**
 * @brief generate_ir
 * @param statement
 */
void generate_ir(const ExpressionStatement *statement) {
	generate_ir(statement->expression);
}

/**
 * @brief generate_ir
 * @param statement
 */
void generate_ir(const Expression *statement) {
	if (auto binary_expression = dynamic_cast<const BinaryExpression *>(statement)) {

		++in_binary_expression;

		switch (binary_expression->op) {
		case Token::Assign:
			if (auto array_index = dynamic_cast<const ArrayIndexExpression *>(binary_expression->lhs.get())) {

				emit_node<PushArraySymbolNode>("PUSH_ARRAY_SYM", to_string(array_index->array), "createAndRef");

				for (const std::unique_ptr<Expression> &index_expr : array_index->index) {
					generate_ir(index_expr);
				}
				generate_ir(binary_expression->rhs);

				emit_node<ArrayOpNode>("ARRAY_ASSIGN", array_index->index.size());

			} else {
				generate_ir(binary_expression->rhs);
				emit_node<AssignNode>("ASSIGN", to_string(binary_expression->lhs));
			}
			break;
		case Token::Add:
			generate_ir(binary_expression->lhs);
			generate_ir(binary_expression->rhs);
			emit_node<Node>("ADD");
			break;
		case Token::Sub:
			generate_ir(binary_expression->lhs);
			generate_ir(binary_expression->rhs);
			emit_node<Node>("SUB");
			break;
		case Token::Mul:
			generate_ir(binary_expression->lhs);
			generate_ir(binary_expression->rhs);
			emit_node<Node>("MUL");
			break;
		case Token::Div:
			generate_ir(binary_expression->lhs);
			generate_ir(binary_expression->rhs);
			emit_node<Node>("DIV");
			break;
		case Token::Mod:
			generate_ir(binary_expression->lhs);
			generate_ir(binary_expression->rhs);
			emit_node<Node>("MOD");
			break;
		case Token::Equal:
			generate_ir(binary_expression->lhs);
			generate_ir(binary_expression->rhs);
			emit_node<Node>("EQ");
			break;
		case Token::NotEqual:
			generate_ir(binary_expression->lhs);
			generate_ir(binary_expression->rhs);
			emit_node<Node>("NE");
			break;
		case Token::LessThan:
			generate_ir(binary_expression->lhs);
			generate_ir(binary_expression->rhs);
			emit_node<Node>("LT");
			break;
		case Token::GreaterThan:
			generate_ir(binary_expression->lhs);
			generate_ir(binary_expression->rhs);
			emit_node<Node>("GT");
			break;
		case Token::GreaterThanOrEqual:
			generate_ir(binary_expression->lhs);
			generate_ir(binary_expression->rhs);
			emit_node<Node>("GE");
			break;
		case Token::LessThanOrEqual:
			generate_ir(binary_expression->lhs);
			generate_ir(binary_expression->rhs);
			emit_node<Node>("LE");
			break;
		case Token::Concatenate: {
			generate_ir(binary_expression->lhs);

			Expression *ptr = binary_expression->rhs.get();

			while (auto binary_rhs = dynamic_cast<BinaryExpression *>(ptr)) {
				if (binary_rhs->op == Token::Concatenate) {
					generate_ir(binary_rhs->lhs);
					emit_node<Node>("CONCAT");
					ptr = binary_rhs->rhs.get();
				} else {
					break;
				}
			}

			generate_ir(ptr);
			emit_node<Node>("CONCAT");
			break;
		}
		case Token::LogicalAnd: {

			generate_ir(binary_expression->lhs);
			emit_node<Node>("DUP");

			BranchNode *br  = emit_node<BranchNode>("BRANCH_FALSE");
			Expression *ptr = binary_expression->rhs.get();

			while (auto binary_rhs = dynamic_cast<BinaryExpression *>(ptr)) {
				if (binary_rhs->op != Token::LogicalAnd) {
					break;
				}

				generate_ir(binary_rhs->lhs);
				emit_node<Node>("AND");
				br->target = current_location() - br->location;
				emit_node<Node>("DUP");
				br  = emit_node<BranchNode>("BRANCH_FALSE");
				ptr = binary_rhs->rhs.get();
			}

			generate_ir(ptr);
			emit_node<Node>("AND");
			br->target = current_location() - br->location;
			break;
		}
		case Token::LogicalOr: {
			generate_ir(binary_expression->lhs);
			emit_node<Node>("DUP");

			BranchNode *br  = emit_node<BranchNode>("BRANCH_TRUE");
			Expression *ptr = binary_expression->rhs.get();

			while (auto binary_rhs = dynamic_cast<BinaryExpression *>(ptr)) {
				if (binary_rhs->op != Token::LogicalOr) {
					break;
				}

				generate_ir(binary_rhs->lhs);
				emit_node<Node>("OR");
				br->target = current_location() - br->location;
				emit_node<Node>("DUP");
				br  = emit_node<BranchNode>("BRANCH_TRUE");
				ptr = binary_rhs->rhs.get();
			}

			generate_ir(ptr);
			emit_node<Node>("OR");
			br->target = current_location() - br->location;
			break;
		}
		default:
			printf("BINARY EXPRESSION - UNHANDLED [%d]\n", binary_expression->op);
			abort();
		}

		--in_binary_expression;

	} else if (auto unary_expression = dynamic_cast<const UnaryExpression *>(statement)) {
		switch (unary_expression->op) {
		case Token::Sub:
			generate_ir(unary_expression->operand);
			emit_node<Node>("NEGATE");
			break;
		case Token::Increment:
			generate_ir(unary_expression->operand);
			if (unary_expression->prefix) {
				c_emit_node<Node>(in_binary_expression, "DUP");
				emit_node<Node>("INCR");
			} else {
				emit_node<Node>("INCR");
				c_emit_node<Node>(in_binary_expression, "DUP");
			}

			// TODO(eteran): support arr[x]++ and ++arr[x]
			emit_node<AssignNode>("ASSIGN", to_string(unary_expression->operand));
			break;
		case Token::Decrement:
			generate_ir(unary_expression->operand);
			if (unary_expression->prefix) {
				c_emit_node<Node>(in_binary_expression, "DUP");
				emit_node<Node>("DECR");
			} else {
				emit_node<Node>("DECR");
				c_emit_node<Node>(in_binary_expression, "DUP");
			}
			// TODO(eteran): support arr[x]-- and --arr[x]
			emit_node<AssignNode>("ASSIGN", to_string(unary_expression->operand));
			break;
		default:
			printf("UNARY EXPRESSION - UNHANDLED [%d]\n", unary_expression->op);
			abort();
		}
	} else if (auto atom_expression = dynamic_cast<const AtomExpression *>(statement)) {
		switch (atom_expression->type) {
		case Token::Integer:
			emit_node<PushSymbolNode>("PUSH_SYM const", atom_expression->value);
			break;
		case Token::String:
			emit_node<PushStringNode>("PUSH_SYM string", atom_expression->value);
			break;
		case Token::Identifier:
			emit_node<PushSymbolNode>("PUSH_SYM", atom_expression->value);
			break;
		case Token::ArrayIdentifier:
			emit_node<PushArraySymbolNode>("PUSH_ARRAY_SYM", atom_expression->value, "refOnly");
			break;
		default:
			printf("ATOM EXPRESSION - UNHANDLED (%d)\n", atom_expression->type);
			abort();
		}
	} else if (auto call_expression = dynamic_cast<const CallExpression *>(statement)) {

		for (auto &parameter : call_expression->parameters) {
			generate_ir(parameter);
		}

		emit_node<CallNode>("SUBR_CALL", to_string(call_expression->function), call_expression->parameters.size());

		c_emit_node<Node>(in_binary_expression, "FETCH_RET_VAL");

	} else if (auto index_expression = dynamic_cast<const ArrayIndexExpression *>(statement)) {

		generate_ir(index_expression->array);
		for (const std::unique_ptr<Expression> &index_expr : index_expression->index) {
			generate_ir(index_expr);
		}

		emit_node<ArrayOpNode>("ARRAY_REF", index_expression->index.size());
	}
}

/**
 * @brief generate_ir
 * @param statement
 */
void generate_ir(const Statement *statement) {
	if (auto delete_statement = dynamic_cast<const DeleteStatement *>(statement)) {

		generate_ir(delete_statement->expression);
		for (const std::unique_ptr<Expression> &index_expr : delete_statement->index) {
			generate_ir(index_expr);
		}
		emit_node<ArrayOpNode>("ARRAY_DELETE", delete_statement->index.size());

	} else if (auto function_statement = dynamic_cast<const FunctionStatement *>(statement)) {
		(void)function_statement;
		printf("FUNCTION - UNHANDLED\n");
		// NOTE(eteran): NEdit handles functions wierd, they are plucked out and treated like independently compiled programs...
#if 1
		abort();
#endif
	} else if (auto block_statement = dynamic_cast<const BlockStatement *>(statement)) {
		generate_ir(block_statement->statements);

	} else if (auto cond_statement = dynamic_cast<const CondStatement *>(statement)) {

		generate_ir(cond_statement->cond);

		BranchNode *br = emit_node<BranchNode>("BRANCH_FALSE");

		generate_ir(cond_statement->body);

		if (cond_statement->else_) {
			BranchNode *br2 = emit_node<BranchNode>("BRANCH");
			br->target      = current_location() - br->location;
			generate_ir(cond_statement->else_);
			br = br2;
		}

		br->target = current_location() - br->location;

	} else if (auto loop_statement = dynamic_cast<const LoopStatement *>(statement)) {

		BranchNode *cond_br;

		loopStack.push({loop_statement, {}, {}});

		for (auto &&init_expr : loop_statement->init) {
			generate_ir(init_expr);
		}

		auto loop_start = current_location();

		if (!loop_statement->cond) {
			cond_br = emit_node<BranchNode>("BRANCH_NEVER");
		} else {
			generate_ir(loop_statement->cond);
			cond_br = emit_node<BranchNode>("BRANCH_FALSE");
		}

		generate_ir(loop_statement->body);

		auto loop_incr = current_location();

		for (auto &&incr_expr : loop_statement->incr) {
			generate_ir(incr_expr);
		}

		auto loop_end = current_location();

		BranchNode *br = emit_node<BranchNode>("BRANCH");
		br->target     = loop_start - loop_end;

		cond_br->target = loop_end - cond_br->location + 1;

		std::vector<BranchNode *> continues = loopStack.top().continues;
		std::vector<BranchNode *> breaks    = loopStack.top().breaks;

		for (BranchNode *break_br : breaks) {
			break_br->target = loop_end + 1 - break_br->location;
		}

		for (BranchNode *cont_br : continues) {
			cont_br->target = loop_incr - cont_br->location;
		}

		loopStack.pop();
	} else if (auto foreach_statement = dynamic_cast<const ForEachStatement *>(statement)) {
		(void)foreach_statement;
		printf("FOREACH - UNHANDLED\n");
		abort();
	} else if (auto break_statement = dynamic_cast<const BreakStatement *>(statement)) {

		(void)break_statement;

		if (loopStack.empty()) {
			printf("ERROR! break statement not within loop or switch\n");
			abort();
		}

		BranchNode *br = emit_node<BranchNode>("BRANCH");
		loopStack.top().breaks.push_back(br);
	} else if (auto continue_statement = dynamic_cast<const ContinueStatement *>(statement)) {

		(void)continue_statement;

		if (loopStack.empty()) {
			printf("ERROR! continue statement not within loop\n");
			abort();
		}

		BranchNode *br = emit_node<BranchNode>("BRANCH");
		loopStack.top().continues.push_back(br);

	} else if (auto expression_statement = dynamic_cast<const ExpressionStatement *>(statement)) {
		generate_ir(expression_statement);
	} else if (auto return_statement = dynamic_cast<const ReturnStatement *>(statement)) {

		if (return_statement->expression) {
			generate_ir(return_statement->expression);
			emit_node<Node>("RETURN");
		} else {
			emit_node<Node>("RETURN_NO_VAL");
		}
	}
}

/**
 * @brief generate_ir
 * @param statement
 */
void generate_ir(const std::unique_ptr<Statement> &statement) {
	generate_ir(statement.get());
}

/**
 * @brief generate_ir
 * @param expression
 */
void generate_ir(const std::unique_ptr<Expression> &expression) {
	generate_ir(expression.get());
}

/**
 * @brief generate_ir
 * @param statements
 */
void generate_ir(const std::vector<std::unique_ptr<Statement>> &statements) {
	for (auto it = statements.begin(); it != statements.end(); ++it) {
		generate_ir(*it);
	}
}

}

/**
 * @brief CodeGenerator::generate
 * @param statements
 */
void CodeGenerator::generate(const std::vector<std::unique_ptr<Statement>> &statements) {
	generate_ir(statements);
	emit_node<Node>("RETURN_NO_VAL");
}

/**
 * @brief CodeGenerator::print_ir
 */
void CodeGenerator::print_ir() {
	for (const node_type &node : nodes) {
		boost::apply_visitor(Visitor{}, node);
	}
}
