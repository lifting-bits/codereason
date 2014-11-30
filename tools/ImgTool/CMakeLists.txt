include_directories(${codeReason_SOURCE_DIR}/libs/IR ${codeReason_SOURCE_DIR}/libs/getExecSections)

add_executable(ImgTool ImgTool.cpp )
IF(WIN32)
target_link_libraries(ImgTool getExecSections PELib ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
ELSE(WIN32)
target_link_libraries(ImgTool getExecSections MachOLib PELib ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
ENDIF(WIN32)
