include_directories(${codeReason_SOURCE_DIR}/libs/IR)

#link_directories(${codeREason_BINARY_DIR}/libs/IR)

add_executable(VEXBlobEater blobEater.cpp getopt.c)
target_link_libraries(VEXBlobEater IR ${VEX_LIB} ${CMAKE_THREAD_LIBS_INIT})
#set_target_properties(VEXBlobEater PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
