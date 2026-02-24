cmake_minimum_required(VERSION 3.19)
project(ArduinoSimulatorQt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets)

qt_standard_project_setup()

qt_add_executable(ArduinoSimulatorQt
    WIN32 MACOSX_BUNDLE






    main.cpp
    app/mainwindow.h app/mainwindow.cpp
    ui/editorpane.h ui/editorpane.cpp
    ui/outputpane.h ui/outputpane.cpp
    ui/simulatorpane.h ui/simulatorpane.cpp
    core/logger.h core/logger.cpp
    core/thememanager.h core/thememanager.cpp
    scene/arduinoboarditem.h scene/arduinoboarditem.cpp
    scene/buttonitem.h scene/buttonitem.cpp
    scene/connectionitem.h scene/connectionitem.cpp
    scene/leditem.h scene/leditem.cpp
    scene/resistoritem.h scene/resistoritem.cpp
    scene/PinPositions.cpp
    scene/ThemedScene.h
    scene/PinPositions.h
    complier/buildresult.h
    complier/compilerservice.h complier/compilerservice.cpp
    complier/compilerworker.h complier/compilerworker.cpp
    sim/Action.h
    sim/boardmodel.h sim/boardmodel.cpp
    sim/parser.h sim/parser.cpp
    sim/pin.h sim/pin.cpp
    sim/runtime.h sim/runtime.cpp
    sim/sketchinterpreter.h sim/sketchinterpreter.cpp
    ui/editortextedit.h ui/editortextedit.cpp
    ui/linenumberarea.h ui/linenumberarea.cpp
    ui/arduinohighlighter.h ui/arduinohighlighter.cpp






)

target_link_libraries(ArduinoSimulatorQt
    PRIVATE
        Qt::Core
        Qt6::Gui
        Qt::Widgets
)

qt_add_resources(ArduinoSimulatorQt RCC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/simulator.qrc
)

install(TARGETS ArduinoSimulatorQt
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET ArduinoSimulatorQt
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
