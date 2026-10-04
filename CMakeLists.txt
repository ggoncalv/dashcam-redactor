cmake_minimum_required(VERSION 3.20)
project(dashcam_redactor VERSION 0.1.0 LANGUAGES CXX)

find_package(OpenCV 4 REQUIRED COMPONENTS core videoio)
include(CTest)

function(dashcam_target target)
    target_compile_features(${target} PUBLIC cxx_std_20)
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

add_library(dashcam_core src/cli.cpp src/video_io.cpp)
target_include_directories(dashcam_core PUBLIC include)
target_include_directories(dashcam_core SYSTEM PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(dashcam_core PRIVATE ${OpenCV_LIBS})
dashcam_target(dashcam_core)

add_executable(dashcam-redactor src/main.cpp)
target_link_libraries(dashcam-redactor PRIVATE dashcam_core)
dashcam_target(dashcam-redactor)

if(BUILD_TESTING)
    add_executable(dashcam_tests tests/test_main.cpp)
    target_include_directories(dashcam_tests SYSTEM PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(dashcam_tests PRIVATE dashcam_core ${OpenCV_LIBS})
    dashcam_target(dashcam_tests)
    add_test(NAME core_and_video_io COMMAND dashcam_tests)
    set_tests_properties(core_and_video_io PROPERTIES TIMEOUT 60)
    add_test(NAME cli_contract COMMAND ${CMAKE_COMMAND}
        -DPROGRAM=$<TARGET_FILE:dashcam-redactor>
        -DFIXTURE_TOOL=$<TARGET_FILE:dashcam_tests>
        -DTEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/cli-tests
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/TestCli.cmake)
    set_tests_properties(cli_contract PROPERTIES TIMEOUT 60)
endif()
