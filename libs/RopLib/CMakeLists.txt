include_directories(${codeReason_SOURCE_DIR}/libs/IR ${codeReason_SOURCE_DIR}/lua-5.3.0/src ${codeReason_SOURCE_DIR}/libs/getExecSections ${codeReason_SOURCE_DIR}/${LIBVEX_BUILD_DIR}/include)

add_library(RopLib 
            RopLib.cpp)
