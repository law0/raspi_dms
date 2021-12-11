set(GCC_COMPILER_VERSION "" CACHE STRING "GCC Compiler version")
set(GNU_MACHINE "arm-linux-gnueabi" CACHE STRING "GNU compiler triple")
set(FLOAT_ABI_SUFFIX "hf" CACHE STRING "Float abi")
set(CMAKE_C_COMPILER   "${CMAKE_CURRENT_LIST_DIR}/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc" CACHE PATH "C compiler")
set(CMAKE_CXX_COMPILER "${CMAKE_CURRENT_LIST_DIR}/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++" CACHE PATH "Cpp compiler")
set(CMAKE_LINKER "${CMAKE_CURRENT_LIST_DIR}/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-ld" CACHE PATH "linker")
set(CMAKE_AR "${CMAKE_CURRENT_LIST_DIR}/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-ar" CACHE PATH "ar")
include("${CMAKE_CURRENT_LIST_DIR}/arm.toolchain.cmake")