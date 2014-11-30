include_directories(${codeReason_SOURCE_DIR}/libs/IR ${codeReason_SOURCE_DIR}/lua-5.1.4/src)

add_library(VEE 
            VEE.cpp 
            VEEOps.cpp 
            VEEState.cpp 
            VEECC.cpp 
            VEEHelpers.cpp
            Conditions.cpp
            VEElua.cpp)
