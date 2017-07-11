
#ifndef OPTIMIZER_H_
#define OPTIMIZER_H_

#include <vector>
#include <memory>
class Statement;

namespace Optimizer {

void prune_empty_statements(std::vector<std::unique_ptr<Statement>> &statements);
void fold_constant_expressions(std::vector<std::unique_ptr<Statement>> &statements);

}

#endif
