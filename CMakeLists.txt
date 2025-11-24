cmake_minimum_required(VERSION 3.20)

project(laser-engraver-toolkit VERSION 0.8.0)

option(TESTS "Build unit tests" ON)
option(BENCHMARKS "Build benchmarks" ON)

if (BENCHMARKS)
    include(FindPkgConfig)
    pkg_check_modules(BENCHMARK REQUIRED IMPORTED_TARGET benchmark)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets SerialPort REQUIRED)
find_package(Threads)


add_library(semi-gcode-lib STATIC
    src/gcode-generator.cpp
    src/gcode-generator.hpp
    src/semi-gcode.cpp
    src/semi-gcode.hpp
)

target_link_libraries(semi-gcode-lib PUBLIC
    Qt6::Core
    Qt6::Widgets
)

set(sources
        src/dialogs/add-engraver-dialog.cpp
        src/dialogs/add-engraver-dialog.h
        src/dialogs/add-engraver-dialog.ui
        src/dialogs/dialogs.cpp
        src/dialogs/dialogs.hpp
        src/dialogs/font-dialog.cpp
        src/dialogs/font-dialog.h
        src/dialogs/font-dialog.ui
        glbr-settings.hpp
        glbr-settings.cpp
        glbr-settings.ui
        src/engraver-connection.cpp
        src/engraver-connection.h
        src/engraver-manager.cpp
        src/engraver-manager.h
        src/engraver-settings.h
        src/engraver/spindle/manager.cpp
        src/engraver/spindle/manager.h
        src/gcode-generator.cpp
        src/gcode-generator.hpp
        src/gui-settings.cpp
        src/gui-settings.cpp
        src/gui-settings.h
        src/gui-settings.h
        src/instructions.hpp
        src/log/model.cpp
        src/log/model.h
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui
        src/models/graphical-object-model.cpp
        src/models/graphical-object-model.h
        src/qt-wrappers.cpp
        src/qt-wrappers.h
        src/select-engraver-dialog.cpp
        src/select-engraver-dialog.h
        src/select-engraver-dialog.ui
        src/sheets.hpp
        src/spindle-position.hpp
        src/upload-strategy.cpp
        src/upload-strategy.hpp
        src/utils.hpp
        src/widgets/engraver-movement-settings-widget.cpp
        src/widgets/engraver-movement-settings-widget.h
        src/widgets/engraver-movement-settings-widget.ui
        src/widgets/graphics-view.cpp
        src/widgets/graphics-view.cpp
        src/widgets/graphics-view.h
        src/workspace.cpp
        src/workspace.h
)

if (TESTS)
    find_package(GTest)

    add_executable(laser-engraver-tests
            src/ut/instruction-tests.cpp
            src/ut/main.cpp
    )

    target_link_libraries(laser-engraver-tests
            GTest::GTest
            semi-gcode-lib
    )
endif()

if (BENCHMARKS)
    add_executable(laser-engraver-benchmark
            src/benchmark/main.cpp
    )

    target_link_libraries(laser-engraver-benchmark
            PkgConfig::BENCHMARK
            semi-gcode-lib
    )
endif()

add_executable(laser-engraver-toolkit ${sources})
target_compile_definitions(laser-engraver-toolkit PRIVATE QT_DEPRECATED_WARNINGS)
target_compile_definitions(laser-engraver-toolkit PRIVATE QT_DISABLE_DEPRECATED_BEFORE=0x060000)

target_link_libraries(laser-engraver-toolkit PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::SerialPort
    Threads::Threads
    semi-gcode-lib
)

install(TARGETS laser-engraver-toolkit RUNTIME DESTINATION bin)

