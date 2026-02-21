cmake_minimum_required(VERSION 3.19)
project(ArduinoSimulatorQt LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

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




)

target_link_libraries(ArduinoSimulatorQt
    PRIVATE
        Qt::Core
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
