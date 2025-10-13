cmake_minimum_required(VERSION 3.20)
project(ScenarioParser 
    VERSION 1.0.0
    DESCRIPTION "Rise of Nations Scenario File Parser"
    LANGUAGES CXX)

#C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#Default build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

#Options
option(BUILD_CLI "Build CLI version" ON)
option(BUILD_TESTS "Build tests" OFF)

#Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

#prefer Conda env
if(NOT CMAKE_PREFIX_PATH AND DEFINED ENV{CONDA_PREFIX})
    list(APPEND CMAKE_PREFIX_PATH "$ENV{CONDA_PREFIX}")
    message(STATUS "Using Conda environment: $ENV{CONDA_PREFIX}")
endif()

#Find Zlib
find_package(ZLIB QUIET)

#Fallback: Conda environment
if(NOT ZLIB_FOUND AND DEFINED ENV{CONDA_PREFIX})
    message(STATUS "ZLIB not found via config; trying \$ENV{CONDA_PREFIX}")
    find_path(ZLIB_INCLUDE_DIR zlib.h
        PATHS "$ENV{CONDA_PREFIX}/include" "$ENV{CONDA_PREFIX}/Library/include" NO_DEFAULT_PATH)
    
    if(WIN32)
        find_library(ZLIB_LIBRARY NAMES zlib zdll zlibstatic z zlib1
            PATHS "$ENV{CONDA_PREFIX}/lib" "$ENV{CONDA_PREFIX}/Library/lib" NO_DEFAULT_PATH)
    else()
        find_library(ZLIB_LIBRARY NAMES z
            PATHS "$ENV{CONDA_PREFIX}/lib" "$ENV{CONDA_PREFIX}/Library/lib" NO_DEFAULT_PATH)
    endif()

    if(ZLIB_INCLUDE_DIR AND ZLIB_LIBRARY)
        add_library(ZLIB::ZLIB UNKNOWN IMPORTED)
        set_target_properties(ZLIB::ZLIB PROPERTIES
            IMPORTED_LINK_INTERFACE_LANGUAGES "C"
            IMPORTED_LOCATION "${ZLIB_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIR}")
        set(ZLIB_FOUND TRUE)
        message(STATUS "Found ZLIB: ${ZLIB_LIBRARY}")
    endif()
endif()

if(NOT ZLIB_FOUND)
    message(FATAL_ERROR "Zlib not found. Set CMAKE_PREFIX_PATH or install zlib.")
endif()

#Includes
include_directories(include)

#Core library
set(CORE_SOURCES
    chunk_parsers.cpp
    chunk_types.cpp
    compression.cpp
    encryption.cpp
    scenario_parser.cpp
)

add_library(scenario_parser_core STATIC ${CORE_SOURCES})
target_include_directories(scenario_parser_core PUBLIC include)
target_link_libraries(scenario_parser_core PUBLIC ZLIB::ZLIB)

#CLI executable
if(BUILD_CLI)
    add_executable(scenario_parser_cli main.cpp)
    target_link_libraries(scenario_parser_cli PRIVATE scenario_parser_core)
    set_target_properties(scenario_parser_cli PROPERTIES OUTPUT_NAME "scenario_parser")
    # Link libstdc++/libgcc statically (Release only)
    if(NOT MSVC)
        target_link_options(scenario_parser_cli PRIVATE
            $<$<CONFIG:Release>:-static-libstdc++ -static-libgcc>
        )
    endif()
    
    #Install CLI
    install(TARGETS scenario_parser_cli DESTINATION bin)
endif()


#Copy zlib DLL (Windows only)
if(WIN32 AND ZLIB_FOUND)
    function(copy_zlib_dll target_name)
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:ZLIB::ZLIB>"
                "$<TARGET_FILE_DIR:${target_name}>"
            COMMENT "Copying zlib runtime to output directory"
        )
    endfunction()

    if(BUILD_CLI)
        copy_zlib_dll(scenario_parser_cli)
    endif()
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    # add_subdirectory(tests)
endif()

# Summary
message(STATUS "")
message(STATUS "==== ScenarioParser Configuration Summary ====")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "")
message(STATUS "Targets:")
message(STATUS "  CLI: ${BUILD_CLI}")
message(STATUS "  Tests: ${BUILD_TESTS}")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "  ZLIB found: ${ZLIB_FOUND}")
if(ZLIB_FOUND)
    message(STATUS "  ZLIB include: ${ZLIB_INCLUDE_DIR}")
    message(STATUS "  ZLIB library: ${ZLIB_LIBRARY}")
endif()
message(STATUS "===============================================")
