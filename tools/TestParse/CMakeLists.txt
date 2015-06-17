include_directories(${codeReason_SOURCE_DIR}/libs/ExprParser ${codeReason_SOURCE_DIR}/libs/IR)

add_executable(TestParse TestParse.cpp )
add_dependencies(TestParse ExprParser)
target_link_libraries(TestParse ExprParser IR ${Boost_LIBRARIES})
