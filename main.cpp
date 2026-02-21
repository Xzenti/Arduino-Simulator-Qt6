#include <QApplication>
#include "app/MainWindow.h"
#include "core/Logger.h"

// Pipeline Edition Build Markers
#define PIPELINE_BUILD_EDITION 1
#define PIPELINE_BUILD_DATE "2026-01-24"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    Logger& logger = Logger::instance();
    logger.log("[PIPELINE] Arduino Simulator - Pipeline Edition Build 2026-01-24");
    logger.log("[PIPELINE] Build system: Ninja + MSVC + Qt6");

    MainWindow window;
    window.setWindowTitle("Arduino-Simulator - Qt 6 + C++ ");
    window.resize(1200, 800);
    window.show();

    return app.exec();
}
