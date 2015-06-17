include_directories(${codeReason_SOURCE_DIR}/libs/IR ${codeReason_SOURCE_DIR}/lua-5.3.0/src ${codeReason_SOURCE_DIR}/libs/getExecSections)

add_library(RopLib 
            RopLib.cpp)
