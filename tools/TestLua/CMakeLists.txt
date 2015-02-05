include_directories(${codeReason_SOURCE_DIR}/lua-5.3.0/src)

add_executable(TestLua TestLua.cpp )
target_link_libraries(TestLua lua)
