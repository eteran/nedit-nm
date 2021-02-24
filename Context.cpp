
#include "Context.h"
#include "Reader.h"

/**
 * @brief Context::Context
 * @param input
 */
Context::Context(const Reader &reader)
	: line_(reader.line()), column_(reader.column()) {
}
