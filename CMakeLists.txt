project(laser-engraver-toolkit)
cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 7)
set(VERSION_PATCH 1)

find_package(Qt5 COMPONENTS Core Gui Widgets SerialPort REQUIRED)
find_package(Threads)

set(sources
        src/add-engraver-dialog.cpp
        src/add-engraver-dialog.h
        src/add-engraver-dialog.ui
        src/dialogs/font-dialog.cpp
        src/dialogs/font-dialog.h
        src/dialogs/font-dialog.ui
        src/engraver-connection.cpp
        src/engraver-connection.h
        src/engraver-manager.cpp
        src/engraver-manager.h
        src/engraver-settings.h
        src/gcode-generator.cpp
        src/gcode-generator.hpp
        src/gui-settings.cpp
        src/gui-settings.cpp
        src/gui-settings.h
        src/gui-settings.h
        src/instructions.hpp
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui
        src/select-engraver-dialog.cpp
        src/select-engraver-dialog.h
        src/select-engraver-dialog.ui
        src/semi-gcode.cpp
        src/semi-gcode.hpp
        src/sheets.hpp
        src/spindle-position.hpp
        src/widgets/engraver-movement-settings-widget.cpp
        src/widgets/engraver-movement-settings-widget.h
        src/widgets/engraver-movement-settings-widget.ui
        src/widgets/graphics-view.cpp
        src/widgets/graphics-view.cpp
        src/widgets/graphics-view.h
        src/workspace.cpp
        src/workspace.h
)

add_executable(laser-engraver-toolkit ${sources})
install(TARGETS laser-engraver-toolkit DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)
target_compile_definitions(laser-engraver-toolkit PRIVATE QT_DEPRECATED_WARNINGS)
target_compile_definitions(laser-engraver-toolkit PRIVATE QT_DISABLE_DEPRECATED_BEFORE=0x060000)

target_link_libraries(laser-engraver-toolkit PRIVATE
    Qt5::Core
    Qt5::Widgets
    Qt5::SerialPort
    Threads::Threads
)

