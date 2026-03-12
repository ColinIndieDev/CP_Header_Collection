cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(cpstd VERSION 1.0.0 LANGUAGES C)

add_library(cpstd INTERFACE)

target_include_directories(cpstd INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Not recommended but I am too lazy to write it out
file(GLOB CPSTD_HEADERS "*.h")
install(FILES ${CPSTD_HEADERS} DESTINATION include/cpstd)

install(TARGETS cpstd
    EXPORT cpstdTargets
)

install(EXPORT cpstdTargets
    FILE cpstdTargets.cmake
    NAMESPACE cpstd::
    DESTINATION lib/cmake/cpstd
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cpstdConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cpstdConfig.cmake"
    INSTALL_DESTINATION lib/cmake/cpstd
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cpstdConfig.cmake"
    DESTINATION lib/cmake/cpstd
)

configure_file(cpstd.pc.in cpstd.pc @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cpstd.pc"
    DESTINATION lib/pkgconfig
)
