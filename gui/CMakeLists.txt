cmake_minimum_required(VERSION 3.20)

message(STATUS "CMAKE_PREFIX_PATH in GUI: ${CMAKE_PREFIX_PATH}")

# Try to find Qt in standard locations
if(WIN32 AND NOT Qt6_DIR AND NOT Qt5_DIR)
    # Look for Qt6 in common Windows locations
    if(EXISTS "C:/Qt/Qt6/6.10.1/mingw_64/lib/cmake/Qt6")
        set(Qt6_DIR "C:/Qt/Qt6/6.10.1/mingw_64/lib/cmake/Qt6" CACHE PATH "Qt6 directory")
    endif()
endif()

# Find Qt6 packages
find_package(Qt6 COMPONENTS Core Widgets Gui)
message(STATUS "Qt6_FOUND: ${Qt6_FOUND}")

# If Qt6 not found, try Qt5
if(NOT Qt6_FOUND)
    find_package(Qt5 5.6 COMPONENTS Core Widgets Gui QUIET)
    if(NOT Qt5_FOUND)
        message(FATAL_ERROR "Neither Qt6 nor Qt5 found. Please install Qt or set CMAKE_PREFIX_PATH to your Qt installation.")
    endif()
    set(QT_VERSION_MAJOR 5)
else()
    set(QT_VERSION_MAJOR 6)
endif()

# Enable automatic MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Source files
set(GUI_SOURCES
    main.cpp
    main_window.cpp
    scenario_document.cpp
    hex_editor_widget.cpp
    chunk_tree_widget.cpp
    chunk_properties_widget.cpp
    chunk_metadata.cpp
    high_level_editor_dialog.cpp
    change_overview_widget.cpp
)

# Header files
set(GUI_HEADERS
    main_window.h
    scenario_document.h
    hex_editor_widget.h
    chunk_tree_widget.h
    chunk_properties_widget.h
    chunk_metadata.h
    high_level_editor_dialog.h
    change_overview_widget.h
)

# Create executable
add_executable(scenario_viewer_gui
    ${GUI_SOURCES}
    ${GUI_HEADERS}
)

# Link libraries
if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(scenario_viewer_gui PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        scenario_parser_core
    )
else()
    target_link_libraries(scenario_viewer_gui PRIVATE
        Qt5::Core
        Qt5::Widgets
        Qt5::Gui
        scenario_parser_core
    )
endif()

# Include directories
target_include_directories(scenario_viewer_gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

# Set C++17 standard
target_compile_features(scenario_viewer_gui PRIVATE cxx_std_17)

# Platform-specific settings
if(WIN32)
    # Set Windows subsystem for GUI application (no console window)
    set_target_properties(scenario_viewer_gui PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Install target
install(TARGETS scenario_viewer_gui
    RUNTIME DESTINATION bin
)
