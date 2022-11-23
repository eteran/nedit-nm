
#include "CodeGenerator.h"
#include "Error.h"
#include "Optimizer.h"
#include "Parser.h"
#include <iostream>
#include <list>
#include <stack>
#include <variant>
#include <vector>

/**
 * @brief main
 * @return
 */
int main(int argc, char *argv[]) {

	if (argc < 2) {
		printf("%s <filename>\n", argv[0]);
		return -1;
	}

	try {
		std::vector<std::unique_ptr<Statement>> statements;

		Parser parser(argv[1]);

		while (true) {
			auto statement = parser.parseStatement();
			if (!statement) {
				break;
			}

			statements.emplace_back(std::move(statement));
		}

		Optimizer::prune_empty_statements(statements);
#if 0
        Optimizer::fold_constant_expressions(statements);
#endif

		CodeGenerator::generate(statements);
		CodeGenerator::print_ir();

	} catch (const SyntaxError &ex) {
		std::cerr << ex.what() << std::endl;
		std::cerr << "At Index:  " << ex.index() << std::endl;
		std::cerr << "Token:     " << ex.token().value << std::endl;
		return -1;
	} catch (const TokenizationError &ex) {
		std::cerr << ex.what() << std::endl;
		std::cerr << "At Index:  " << ex.index() << std::endl;
		return -1;
	} catch (const FileNotFound &ex) {
		std::cerr << ex.what() << std::endl;
		std::cerr << "Filename:   " << ex.filename() << std::endl;
		return -1;
	}
}
