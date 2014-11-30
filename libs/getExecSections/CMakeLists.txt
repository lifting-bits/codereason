include_directories(${codeReason_SOURCE_DIR}/libs/IR ${codeReason_SOURCE_DIR}/libs/PELib ${codeReason_SOURCE_DIR}/libs/MachOLib )

add_library(getExecSections getExec.cpp)
