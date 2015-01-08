include_directories(${codeReason_SOURCE_DIR}/libs/IR ${codeReason_SOURCE_DIR}/libs/IR ${codeReason_SOURCE_DIR}/libs/VEE ${codeReason_SOURCE_DIR}/libs/getExecSections ${codeReason_SOURCE_DIR}/lua-5.1.4/src ${codeReason_SOURCE_DIR}/gtest-1.6.0/include )

link_directories(${Boost_LIBRARY_DIRS})
add_definitions(-D__STDC_LIMIT_MACROS)

add_executable(codeReasonTest test.cpp )

IF(WIN32)
target_link_libraries(codeReasonTest VEE IR getExecSections PELib lua ${VEX_LIB} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} )
ELSE(WIN32)
target_link_libraries(codeReasonTest VEE IR getExecSections MachOLib PELib lua ${VEX_LIB} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} )
ENDIF(WIN32)

enable_testing()

target_link_libraries(codeReasonTest gtest gtest_main)

add_test(codeReason-test ${CMAKE_CURRENT_BINARY_DIR}/codeReasonTest)
