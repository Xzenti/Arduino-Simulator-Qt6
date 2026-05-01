#include "gui\mainwindow.h"

#include <QTextEdit>
#include <QPushButton>

void MainWindow::onBuildClicked()
{
    debugOutput->append("🔧 Build button clicked");
    compileOutput->append("Compilation started...");

    // Temporary success (real compiler added by Member 2)
    runButton->setEnabled(true);

    debugOutput->append("✅ Build successful");
}

void MainWindow::onRunClicked()
{
    debugOutput->append("▶ Run button clicked");
}
