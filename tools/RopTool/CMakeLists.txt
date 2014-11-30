PROTOBUF_GENERATE_CPP(IR_PROTO_SRCS IR_PROTO_HDRS ${codeReason_SOURCE_DIR}/libs/IR/IR.proto)

include_directories(${codeReason_SOURCE_DIR}/libs/IR ${codeReason_SOURCE_DIR}/libs/IR ${codeReason_SOURCE_DIR}/libs/VEE ${codeReason_SOURCE_DIR}/libs/getExecSections ${codeReason_SOURCE_DIR}/lua-5.1.4/src ${codeReason_SOURCE_DIR}/udis86 ${codeReason_SOURCE_DIR}/libs/RopLib ${CMAKE_CURRENT_BINARY_DIR} )

link_directories(${LLVM_LIBDIR_RAW} ${Boost_LIBRARY_DIRS})
add_definitions(-D__STDC_LIMIT_MACROS)

add_executable( RopTool 
                RopTool.cpp
                ${IR_PROTO_SRCS}
                ${IR_PROTO_HDRS})

IF(WIN32)
target_link_libraries(  RopTool 
                        ${PROTOBUF_LIBRARIES} 
                        VEE 
                        RopLib 
                        IR 
                        getExecSections 
                        PELib 
                        lua 
                        udis86 
                        ${VEX_LIB} 
                        ${CMAKE_THREAD_LIBS_INIT} 
                        ${Boost_LIBRARIES} 
                        ${LLVM_LIBS_RAW} )
ELSE(WIN32)
target_link_libraries(  RopTool 
                        ${PROTOBUF_LIBRARIES}
                        VEE 
                        RopLib 
                        IR 
                        getExecSections 
                        MachOLib 
                        PELib 
                        lua 
                        udis86 
                        ${VEX_LIB} 
                        ${CMAKE_THREAD_LIBS_INIT} 
                        ${Boost_LIBRARIES} 
                        ${LLVM_LIBS_RAW} 
                        dl)
ENDIF(WIN32)
