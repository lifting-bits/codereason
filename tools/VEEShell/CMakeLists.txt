include_directories(${codeReason_SOURCE_DIR}/libs/IR ${codeReason_SOURCE_DIR}/libs/VEE ${codeReason_SOURCE_DIR}/libs/PELib ${codeReason_SOURCE_DIR}/lua-5.1.4/src)

#link_directories(${codeReason_BINARY_DIR}/IR ${codeReason_BINARY_DIR}/VEE ${codeReason_BINARY_DIR}/PELib)
link_directories(${Boost_LIBRARY_DIRS})

add_executable(VEEShell shell.cpp )
target_link_libraries(VEEShell VEE IR PELib ${VEX_LIB} lua ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
