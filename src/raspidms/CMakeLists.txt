cmake_minimum_required(VERSION 3.1)

project (raspidms)

file(GLOB SOURCES "*.cpp")
add_executable(raspidms ${SOURCES})

find_package(PkgConfig REQUIRED)

pkg_search_module(PKG_OPENCV REQUIRED opencv)
include_directories(${PKG_OPENCV_INCLUDE_DIRS})
target_link_libraries(raspidms PRIVATE ${PKG_OPENCV_LDFLAGS})

install(TARGETS raspidms DESTINATION bin)
install(FILES run.sh DESTINATION bin)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/res/ DESTINATION res)
