#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <memory>
#include <vector>

class Statement;

namespace CodeGenerator {

void generate(const std::vector<std::unique_ptr<Statement>> &statements);
void print_ir();

}

#endif
