PROTOBUF_GENERATE_CPP(IR_PROTO_SRCS IR_PROTO_HDRS ${codeReason_SOURCE_DIR}/libs/IR/IR.proto)

include_directories(${codeReason_SOURCE_DIR}/libs/IR ${codeReason_SOURCE_DIR}/libs/IR ${codeReason_SOURCE_DIR}/libs/VEE ${codeReason_SOURCE_DIR}/libs/getExecSections ${codeReason_SOURCE_DIR}/lua-5.3.0/src ${codeReason_SOURCE_DIR}/libs/RopLib ${CMAKE_CURRENT_BINARY_DIR} ${codeReason_SOURCE_DIR}/libs/ExprParser )

link_directories(${LLVM_LIBDIR_RAW} ${Boost_LIBRARY_DIRS})
add_definitions(-D__STDC_LIMIT_MACROS)

add_executable( RopScore 
                RopScore.cpp
                ${IR_PROTO_SRCS}
                ${IR_PROTO_HDRS})

IF(WIN32)
target_link_libraries(  RopScore 
                        ${PROTOBUF_LIBRARIES} 
                        ExprParser
                        VEE 
                        RopLib 
                        IR 
                        getExecSections 
                        PELib 
                        lua 
                        ${VEX_LIB} 
                        ${CMAKE_THREAD_LIBS_INIT} 
                        ${Boost_LIBRARIES} 
                        ${LLVM_LIBS_RAW} )
ELSE(WIN32)
target_link_libraries(  RopScore 
                        ${PROTOBUF_LIBRARIES}
                        ExprParser
                        VEE 
                        RopLib 
                        IR 
                        getExecSections 
                        MachOLib 
                        PELib 
                        lua 
                        ${VEX_LIB} 
                        ${CMAKE_THREAD_LIBS_INIT} 
                        ${Boost_LIBRARIES} 
                        ${LLVM_LIBS_RAW} 
                        dl)
ENDIF(WIN32)
