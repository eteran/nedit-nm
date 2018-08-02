
#include "Context.h"
#include "Input.h"

/**
 * @brief Context::Context
 * @param input
 */
Context::Context(const Input &input) : line_(input.line()), column_(input.column()) {
}
