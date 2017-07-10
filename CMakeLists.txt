cmake_minimum_required (VERSION 3.0)

project(nm CXX)

find_package(Boost 1.35 REQUIRED)

set(nm_SOURCES
	Error.h
	Expression.h
	Context.h
	Context.cpp
	Input.cpp
	Input.h
	main.cpp
	Parser.cpp
	Parser.h
	Statement.h
	Token.h
	Tokenizer.cpp
	Tokenizer.h
)

add_executable(nm ${nm_SOURCES})

target_include_directories(nm PUBLIC
	${Boost_INCLUDE_DIR}
)

target_link_libraries(nm LINK_PUBLIC
	${Boost_LIBRARIES}
)

set_property(TARGET nm PROPERTY CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -W -Wall -Wmissing-field-initializers -Wunused -Wshadow")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb3")
