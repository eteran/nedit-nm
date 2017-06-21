
#include "Parser.h"
#include "Error.h"
#include <iostream>
#include <map>

namespace {

std::vector<std::unique_ptr<Statement>> statements;

}

/**
 * @brief main
 * @return
 */
int main(int argc, char *argv[]) {

	if(argc < 2) {
		printf("%s <filename>\n", argv[0]);
		return -1;
	}

#if 0
    try {
#endif
        Parser parser(argv[1]);

        while (true) {
            auto statement = parser.parseStatement();
            if (!statement) {
                break;
            }

            statements.push_back(std::move(statement));
        }
#if 0
    } catch(const SyntaxError &ex) {
        std::cerr << ex.what() << std::endl;
        std::cerr << "On line:   " << ex.line() << std::endl;
        std::cerr << "In Column: " << ex.column() << std::endl;
        std::cerr << "Token:     " << ex.token().value << std::endl;
        return -1;
    } catch(const TokenizationError &ex) {
        std::cerr << ex.what() << std::endl;
        std::cerr << "On line:   " << ex.context().line() << std::endl;
        std::cerr << "In Column: " << ex.context().column() << std::endl;
        return -1;
    }
#endif
}



