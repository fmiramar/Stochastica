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
    processes/AdvancedEventProcesses.cpp
    processes/ShotNoise.cpp
    distributions/CircularDistributions.cpp
    state/MultiOUProcess.cpp
    processes/MultiHawkesTrig.cpp
    processes/DoubleWellProcess.cpp
    flows/KuramotoNetwork.cpp
    distributions/TDirichletRand.cpp
    state/PolyaUrn.cpp
    processes/MarkovModulatedTrig.cpp
    processes/MaternNoise.cpp
    processes/LevyFlight.cpp
    processes/JumpDiffusion.cpp
    flows/WilsonCowan.cpp
    flows/Lorenz96.cpp
    gendy/GendyUGens.cpp
)

set(sd_sc_files
    Classes/SDRawMaps.sc
    Classes/SDFlows.sc
    Classes/SDProcesses.sc
    Classes/SDDistributions.sc
    Classes/SDStateProcesses.sc
    Classes/SDMapSonifiers.sc
    Classes/SDVisualization.sc
    Classes/SDAdvancedProcesses.sc
    Classes/SDCircularDistributions.sc
    Classes/SDMultiOUProcess.sc
    Classes/SDMultiHawkesTrig.sc
    Classes/SDDoubleWellProcess.sc
    Classes/SDKuramotoNetwork.sc
    Classes/SDTDirichletRand.sc
    Classes/SDPolyaUrn.sc
    Classes/SDMarkovModulatedTrig.sc
    Classes/SDMaternNoise.sc
    Classes/SDLevyFlight.sc
    Classes/SDJumpDiffusion.sc
    Classes/SDWilsonCowan.sc
    Classes/SDLorenz96.sc
    Classes/SDGendy.sc
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

install(FILES HelpSource/Guides/Stochastica.schelp HelpSource/Guides/scdoc_banner.jpg
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
    foreach(kind reference statistical)
        if(kind STREQUAL "reference")
            set(round2_source tests/reference/test_round2_reference.cpp)
        else()
            set(round2_source tests/statistical/test_round2_statistics.cpp)
        endif()
        set(round2_target sd_round2_${kind}_tests)
        add_executable(${round2_target} ${round2_source})
        target_include_directories(${round2_target} PRIVATE include)
        if(STRICT)
            if(MSVC)
                target_compile_options(${round2_target} PRIVATE /W4)
            else()
                target_compile_options(${round2_target} PRIVATE -Wall -Wextra -Wpedantic)
            endif()
        endif()
        if(SD_SANITIZE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
            target_compile_options(${round2_target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
            set_property(TARGET ${round2_target} APPEND_STRING PROPERTY LINK_FLAGS " -fsanitize=address,undefined")
        endif()
        add_test(NAME ${round2_target} COMMAND ${round2_target})
    endforeach()
endif()

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    add_executable(sd_round2_unit_tests tests/server/test_round2_units.cpp
        processes/AdvancedEventProcesses.cpp processes/ShotNoise.cpp
        distributions/CircularDistributions.cpp state/MultiOUProcess.cpp
        processes/MultiHawkesTrig.cpp processes/DoubleWellProcess.cpp flows/KuramotoNetwork.cpp)
    target_include_directories(sd_round2_unit_tests PRIVATE include
        "${SC_PATH}/include/plugin_interface" "${SC_PATH}/include/common" "${SC_PATH}/common")
    if(SD_SANITIZE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(sd_round2_unit_tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        set_property(TARGET sd_round2_unit_tests APPEND_STRING PROPERTY LINK_FLAGS " -fsanitize=address,undefined")
    endif()
    add_test(NAME sd_round2_unit_tests COMMAND sd_round2_unit_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/server/round2-seeded-reference.txt")
endif()

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    add_executable(sd_round3_reference_tests tests/reference/test_round3_reference.cpp)
    add_executable(sd_round3_statistical_tests tests/statistical/test_round3_statistics.cpp)
    set(sd_round3_sources
        distributions/TDirichletRand.cpp state/PolyaUrn.cpp processes/MarkovModulatedTrig.cpp
        processes/MaternNoise.cpp processes/LevyFlight.cpp processes/JumpDiffusion.cpp
        flows/WilsonCowan.cpp flows/Lorenz96.cpp)
    add_executable(sd_round3_unit_tests tests/server/test_round3_units.cpp ${sd_round3_sources})
    add_executable(sd_round3_benchmark EXCLUDE_FROM_ALL
        tests/performance/round3_benchmark.cpp ${sd_round3_sources})
    target_include_directories(sd_round3_benchmark PRIVATE include
        "${SC_PATH}/include/plugin_interface" "${SC_PATH}/include/common" "${SC_PATH}/common")
    add_executable(sd_round3_analysis EXCLUDE_FROM_ALL scripts/export_round3_analysis.cpp)
    target_include_directories(sd_round3_analysis PRIVATE include)
    target_include_directories(sd_round3_unit_tests PRIVATE
        "${SC_PATH}/include/plugin_interface" "${SC_PATH}/include/common" "${SC_PATH}/common")
    foreach(kind reference statistical unit)
        set(round3_target sd_round3_${kind}_tests)
        target_include_directories(${round3_target} PRIVATE include)
        if(STRICT AND NOT kind STREQUAL "unit")
            if(MSVC)
                target_compile_options(${round3_target} PRIVATE /W4)
            else()
                target_compile_options(${round3_target} PRIVATE -Wall -Wextra -Wpedantic)
            endif()
        endif()
        if(SD_SANITIZE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
            target_compile_options(${round3_target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
            set_property(TARGET ${round3_target} APPEND_STRING PROPERTY LINK_FLAGS " -fsanitize=address,undefined")
        endif()
        if(kind STREQUAL "unit")
            add_test(NAME ${round3_target} COMMAND ${round3_target}
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/server/round3-seeded-reference.txt")
        else()
            add_test(NAME ${round3_target} COMMAND ${round3_target})
        endif()
    endforeach()
endif()

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    add_executable(sd_gendy_reference_tests tests/reference/test_gendy_reference.cpp)
    add_executable(sd_gendy_statistical_tests tests/statistical/test_gendy_statistics.cpp)
    add_executable(sd_gendy_unit_tests tests/server/test_gendy_units.cpp gendy/GendyUGens.cpp)
    target_include_directories(sd_gendy_unit_tests PRIVATE
        "${SC_PATH}/include/plugin_interface" "${SC_PATH}/include/common" "${SC_PATH}/common")
    foreach(kind reference statistical unit)
        set(gendy_target sd_gendy_${kind}_tests)
        target_include_directories(${gendy_target} PRIVATE include)
        if(STRICT AND NOT MSVC)
            target_compile_options(${gendy_target} PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter)
        endif()
        if(SD_SANITIZE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
            target_compile_options(${gendy_target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
            set_property(TARGET ${gendy_target} APPEND_STRING PROPERTY LINK_FLAGS " -fsanitize=address,undefined")
        endif()
        add_test(NAME ${gendy_target} COMMAND ${gendy_target})
    endforeach()
endif()

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    add_executable(sd_gendy_analysis EXCLUDE_FROM_ALL scripts/export_gendy_analysis.cpp)
    target_include_directories(sd_gendy_analysis PRIVATE include)
    add_executable(sd_gendy_benchmark EXCLUDE_FROM_ALL tests/performance/gendy_benchmark.cpp gendy/GendyUGens.cpp)
    target_include_directories(sd_gendy_benchmark PRIVATE include
        "${SC_PATH}/include/plugin_interface" "${SC_PATH}/include/common" "${SC_PATH}/common")
endif()
