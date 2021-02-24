
#include "Optimizer.h"
#include "Expression.h"
#include "Statement.h"
#include <algorithm>
#include <cmath>

namespace Optimizer {
namespace {

void fold(std::unique_ptr<Expression> &expression);

void fold_string_expression(AtomExpression *left, AtomExpression *right, Token::Type op, std::unique_ptr<Expression> &expression) {
	switch (op) {
	case Token::Type::Concatenate: {
		std::string v = left->value + right->value;

		auto atom   = std::make_unique<AtomExpression>();
		atom->value = v;
		atom->type  = Token::Type::String;
		expression  = std::move(atom);
	} break;
	default:
		break;
	}
}

void fold_numeric_expression(AtomExpression *left, AtomExpression *right, Token::Type op, std::unique_ptr<Expression> &expression) {
	switch (op) {
	case Token::Type::Add: {
		int32_t l = std::stoi(left->value);
		int32_t r = std::stoi(right->value);
		int32_t v = l + r;

		auto atom   = std::make_unique<AtomExpression>();
		atom->value = std::to_string(v);
		atom->type  = Token::Type::Integer;
		expression  = std::move(atom);
	} break;
	case Token::Type::Sub: {
		int32_t l = std::stoi(left->value);
		int32_t r = std::stoi(right->value);
		int32_t v = l - r;

		auto atom   = std::make_unique<AtomExpression>();
		atom->value = std::to_string(v);
		atom->type  = Token::Type::Integer;
		expression  = std::move(atom);
	} break;
	case Token::Type::Mul: {
		int32_t l = std::stoi(left->value);
		int32_t r = std::stoi(right->value);
		int32_t v = l * r;

		auto atom   = std::make_unique<AtomExpression>();
		atom->value = std::to_string(v);
		atom->type  = Token::Type::Integer;
		expression  = std::move(atom);
	} break;
	case Token::Type::Div: {
		int32_t l = std::stoi(left->value);
		int32_t r = std::stoi(right->value);

		// NOTE(eteran): we don't HAVE to throw an error (but we could)
		// we can just let it fail at runtime
		if (r == 0) {
			break;
		}

		int32_t v = l / r;

		auto atom   = std::make_unique<AtomExpression>();
		atom->value = std::to_string(v);
		atom->type  = Token::Type::Integer;
		expression  = std::move(atom);
	} break;
	case Token::Type::Mod: {
		int32_t l = std::stoi(left->value);
		int32_t r = std::stoi(right->value);

		// NOTE(eteran): we don't HAVE to throw an error (but we could)
		// we can just let it fail at runtime
		if (r == 0) {
			break;
		}

		int32_t v = l % r;

		auto atom   = std::make_unique<AtomExpression>();
		atom->value = std::to_string(v);
		atom->type  = Token::Type::Integer;
		expression  = std::move(atom);
	} break;
	case Token::Type::Exponent: {
		int32_t l = std::stoi(left->value);
		int32_t r = std::stoi(right->value);
		int32_t v = static_cast<int32_t>(std::pow(static_cast<double>(l), static_cast<double>(r)));

		auto atom   = std::make_unique<AtomExpression>();
		atom->value = std::to_string(v);
		atom->type  = Token::Type::Integer;
		expression  = std::move(atom);
	} break;
	default:
		break;
	}
}

void fold_binary_expression(BinaryExpression *bin, std::unique_ptr<Expression> &expression) {
	fold(bin->lhs);
	fold(bin->rhs);

	if (auto left = dynamic_cast<AtomExpression *>(bin->lhs.get())) {
		if (auto right = dynamic_cast<AtomExpression *>(bin->rhs.get())) {
			if (left->type == Token::Integer && right->type == Token::Integer) {
				fold_numeric_expression(left, right, bin->op, expression);
			} else if (left->type == Token::String && right->type == Token::String) {
				fold_string_expression(left, right, bin->op, expression);
			} else if (left->type == Token::String && right->type == Token::Integer) {
				fold_string_expression(left, right, bin->op, expression);
			} else if (left->type == Token::Integer && right->type == Token::String) {
				fold_string_expression(left, right, bin->op, expression);
			}
		}
	}
}

/**
 * @brief fold
 * @param expression
 */
void fold(std::unique_ptr<Expression> &expression) {

	if (auto bin = dynamic_cast<BinaryExpression *>(expression.get())) {
		fold_binary_expression(bin, expression);
	} else if (auto call = dynamic_cast<CallExpression *>(expression.get())) {
		for (auto &param : call->parameters) {
			fold(param);
		}
	} else if (auto arr = dynamic_cast<ArrayIndexExpression *>(expression.get())) {
		for (auto &idx : arr->index) {
			fold(idx);
		}
	}
}

/**
 * @brief fold
 * @param statement
 */
void fold(std::unique_ptr<Statement> &statement) {

	// NOTE(eteran): CondStatement, LoopStatement, ForEachStatement

	Statement *p = statement.get();
	if (auto block = dynamic_cast<BlockStatement *>(p)) {
		fold_constant_expressions(block->statements);
	} else if (auto expr = dynamic_cast<ExpressionStatement *>(p)) {
		fold(expr->expression);
	} else if (auto ret = dynamic_cast<ReturnStatement *>(p)) {
		fold(ret->expression);
	}
}

}

/**
 * @brief fold_constant_expressions
 * @param statements
 */
void fold_constant_expressions(std::vector<std::unique_ptr<Statement>> &statements) {

	for (std::unique_ptr<Statement> &statement : statements) {
		fold(statement);
	}
}

/**
 * @brief prune_empty_statements
 * @param statements
 */
void prune_empty_statements(std::vector<std::unique_ptr<Statement>> &statements) {
	auto it = std::remove_if(statements.begin(), statements.end(), [](const std::unique_ptr<Statement> &stmt) {
		if (auto expr = dynamic_cast<ExpressionStatement *>(stmt.get())) {
			if (!expr->expression) {
				return true;
			}
		}

		return false;
	});

	statements.erase(it, statements.end());
}

}
