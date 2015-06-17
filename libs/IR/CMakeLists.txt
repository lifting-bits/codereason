PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS IR.proto)

include_directories(${PROTOBUF_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_library(IR 
            BasicIR.cpp 
            Output.cpp 
            printIR.cpp 
            printOp.cpp 
            Input.cpp 
            decodeToIR.cpp 
            VexIR.cpp 
            Helpers.cpp 
            IRMatcher.cpp 
            Serial.cpp
            ${PROTO_SRCS}
            ${PROTO_HDRS})
