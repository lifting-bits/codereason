if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-fpermissive")
endif()

add_library(PELib BoundImportDirectory.cpp ComHeaderDirectory.cpp DebugDirectory.cpp ExportDirectory.cpp IatDirectory.cpp MzHeader.cpp PeFile.cpp PeHeader.cpp PeLibAux.cpp RelocationsDirectory.cpp ResourceDirectory.cpp buffer/InputBuffer.cpp buffer/OutputBuffer.cpp)
