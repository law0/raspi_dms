cmake_minimum_required(VERSION 3.1)

project (raspidms)

add_subdirectory(../dlib dlib_build)

#avoid warning messages of type
#parameter passing has changed in GCC 7.1
set(GCC_NO_WARN_FLAGS "-Wno-psabi")

file(GLOB SOURCES "*.cpp")
add_executable(raspidms ${SOURCES})

add_definitions(${GCC_NO_WARN_FLAGS})

find_package(PkgConfig REQUIRED)

pkg_search_module(PKG_OPENCV REQUIRED opencv)
include_directories(${PKG_OPENCV_INCLUDE_DIRS})
target_link_libraries(raspidms PRIVATE ${PKG_OPENCV_LDFLAGS}
                               PRIVATE dlib::dlib)

install(TARGETS raspidms DESTINATION bin)
install(FILES run.sh DESTINATION bin)
install(FILES run_local.sh DESTINATION bin)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/res/ DESTINATION res)
