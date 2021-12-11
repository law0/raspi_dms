cmake_minimum_required(VERSION 3.1)

project (raspidms)

# dlib
add_subdirectory(../dlib "${CMAKE_CURRENT_BINARY_DIR}/dlib" EXCLUDE_FROM_ALL)

# tensorflow lite
add_subdirectory(
  "../tensorflow/tensorflow/lite"
  "${CMAKE_CURRENT_BINARY_DIR}/tensorflow-lite" EXCLUDE_FROM_ALL)


#avoid warning messages of type
#parameter passing has changed in GCC 7.1
set(GCC_NO_WARN_FLAGS "-Wno-psabi")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE SOURCES "*.cpp")
add_executable(raspidms ${SOURCES})

add_definitions(${GCC_NO_WARN_FLAGS})

find_package(PkgConfig REQUIRED)

pkg_search_module(PKG_OPENCV REQUIRED opencv)
include_directories(${PKG_OPENCV_INCLUDE_DIRS})
target_link_libraries(raspidms PRIVATE ${PKG_OPENCV_LDFLAGS}
                               PRIVATE dlib::dlib
                               PRIVATE tensorflow-lite)

install(TARGETS raspidms DESTINATION bin)
install(FILES run.sh DESTINATION bin)
install(FILES run_local.sh DESTINATION bin)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/res/ DESTINATION res)
