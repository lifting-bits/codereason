include_directories(${codeReason_SOURCE_DIR}/libs/elfio ${codeReason_SOURCE_DIR}/libs/IR ${codeReason_SOURCE_DIR}/libs/pe-parse/parser-library ${codeReason_SOURCE_DIR}/libs/MachOLib )

add_library(getExecSections getExec.cpp)
