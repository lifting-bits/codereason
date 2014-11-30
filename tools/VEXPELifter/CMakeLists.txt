include_directories(${codeReason_SOURCE_DIR}/libs/IR ${codeReason_SOURCE_DIR}/libs/PELib)

#link_directories(${codeReason_BINARY_DIR}/IR)

add_executable(VEXPELifter lifter.cpp getopt.c )
target_link_libraries(VEXPELifter IR PELib ${VEX_LIB} ${CMAKE_THREAD_LIBS_INIT})
