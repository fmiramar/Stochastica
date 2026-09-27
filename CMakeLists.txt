cmake_minimum_required(VERSION 3.12)

project(Stochastica VERSION 0.1.0 LANGUAGES CXX)

option(SCSYNTH "Build the scsynth plugin target" ON)
option(SUPERNOVA "Build the supernova plugin target" OFF)
option(STRICT "Enable strict compiler warnings" OFF)
option(NATIVE "Enable -march=native where supported" OFF)
option(SD_SANITIZE "Enable AddressSanitizer and UndefinedBehaviorSanitizer for tests" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/stage" CACHE PATH "Install prefix" FORCE)
endif()

if(NOT SC_PATH)
    message(FATAL_ERROR "SC_PATH must point to a matching SuperCollider source tree")
endif()

get_filename_component(SC_PATH_FOR_MODULES "${SC_PATH}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${SC_PATH_FOR_MODULES}/tools/cmake_gen")

include(SuperColliderServerPlugin)
sc_do_initial_compiler_config()
sc_check_sc_path("${SC_PATH}")

set(sd_cpp_files
    Stochastica.cpp
    maps/Maps.cpp
    flows/Flows.cpp
    processes/Processes.cpp
    distributions/TriggeredDistributions.cpp
    state/StateProcesses.cpp
    sonifiers/MapSonifiers.cpp
)

set(sd_sc_files
    Classes/SDRawMaps.sc
    Classes/SDFlows.sc
    Classes/SDProcesses.sc
    Classes/SDDistributions.sc
    Classes/SDStateProcesses.sc
    Classes/SDMapSonifiers.sc
    Classes/SDVisualization.sc
)

file(GLOB sd_schelp_files CONFIGURE_DEPENDS HelpSource/Classes/*.schelp)
file(GLOB sd_schelp_images CONFIGURE_DEPENDS HelpSource/Classes/*.png)

sc_add_server_plugin(
    "Stochastica"
    "Stochastica"
    "${sd_cpp_files}"
    "${sd_sc_files}"
    "${sd_schelp_files}"
)

foreach(target Stochastica_scsynth Stochastica_supernova)
    if(TARGET ${target})
        target_include_directories(${target} PRIVATE include)
        if(STRICT)
            if(MSVC)
                target_compile_options(${target} PRIVATE /W4)
            else()
                target_compile_options(${target} PRIVATE
                    -Wall -Wextra -Wpedantic
                    -Wno-unused-parameter
                )
            endif()
        endif()
    endif()
endforeach()

if(TARGET Stochastica_scsynth)
    set_target_properties(Stochastica_scsynth PROPERTIES OUTPUT_NAME "Stochastica")
    if(MINGW)
        target_link_options(Stochastica_scsynth PRIVATE -static-libgcc -static-libstdc++)
    endif()
endif()

if(TARGET Stochastica_supernova)
    set_target_properties(Stochastica_supernova PROPERTIES OUTPUT_NAME "Stochastica_supernova")
endif()

install(FILES HelpSource/Guides/Stochastica.schelp
    DESTINATION Stochastica/HelpSource/Guides)
if(sd_schelp_images)
    install(FILES ${sd_schelp_images}
        DESTINATION Stochastica/HelpSource/Classes)
endif()
install(FILES README.md CHANGELOG.md DESTINATION Stochastica)
install(DIRECTORY docs/ DESTINATION Stochastica/docs)

include(CTest)
if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    add_executable(sd_reference_tests tests/reference/test_sd_reference.cpp)
    target_include_directories(sd_reference_tests PRIVATE include)
    add_executable(sd_statistical_tests tests/statistical/test_sd_statistics.cpp)
    target_include_directories(sd_statistical_tests PRIVATE include)
    if(STRICT)
        if(MSVC)
            target_compile_options(sd_reference_tests PRIVATE /W4)
            target_compile_options(sd_statistical_tests PRIVATE /W4)
        else()
            target_compile_options(sd_reference_tests PRIVATE -Wall -Wextra -Wpedantic)
            target_compile_options(sd_statistical_tests PRIVATE -Wall -Wextra -Wpedantic)
        endif()
    endif()
    if(SD_SANITIZE)
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
            foreach(test_target sd_reference_tests sd_statistical_tests)
                target_compile_options(${test_target} PRIVATE
                    -fsanitize=address,undefined
                    -fno-omit-frame-pointer)
                set_property(TARGET ${test_target} APPEND_STRING PROPERTY
                    LINK_FLAGS " -fsanitize=address,undefined")
            endforeach()
        else()
            message(FATAL_ERROR "SD_SANITIZE requires Clang or GCC")
        endif()
    endif()
    add_test(NAME sd_reference_tests COMMAND sd_reference_tests)
    add_test(NAME sd_statistical_tests COMMAND sd_statistical_tests)
endif()
