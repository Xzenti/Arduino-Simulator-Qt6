#include "MainWindow.h"
#include "../ui/EditorPane.h"
#include "../ui/SimulatorPane.h"
#include "../ui/OutputPane.h"
#include "../core/Logger.h"
#include "../core/ThemeManager.h"

#include <QVBoxLayout>
#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include <QStyle>
#include <QApplication>
#include <QStatusBar>
#include <QFont>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_editorPane(std::make_unique<EditorPane>()),
    //m_simulatorPane(std::make_unique<SimulatorPane>()),
    m_outputPane(std::make_unique<OutputPane>(nullptr, "Compiler Output")),
    m_debugPane(std::make_unique<OutputPane>(nullptr, "Debug Log"))
{

    QFont defaultFont("Segoe UI", 10);
    qApp->setFont(defaultFont);

    setupUI();
    setupToolBar();
    connectSignals();

    statusBar()->showMessage("Ready");

    Logger::instance().log("Application initialized");
}

MainWindow::~MainWindow() = default;

//////////////////////////////////////////////////////////////
// UI SETUP
//////////////////////////////////////////////////////////////

void MainWindow::setupUI()
{
    // ===== GLOBAL DARK THEME =====
    qApp->setStyleSheet(R"(

        QMainWindow {
            background-color: #1e1e2e;
        }

        QToolBar {
            background: #181825;
            border-bottom: 1px solid #313244;
            spacing: 6px;
        }

        QToolButton {
            background-color: #313244;
            color: #cdd6f4;
            border-radius: 6px;
            padding: 6px 12px;
        }

        QToolButton:hover {
            background-color: #45475a;
        }

        QToolButton:pressed {
            background-color: #585b70;
        }

        QSplitter::handle {
            background-color: #45475a;
        }

        QStatusBar {
            background-color: #181825;
            color: #cdd6f4;
        }

    )");

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0,0,0,0);

    // Create splitters
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);   // left, center, right
    m_rightSplitter = new QSplitter(Qt::Vertical, this);    // compiler output + debug log

    // Build the right panel: vertical splitter with two output panes
    m_rightSplitter->addWidget(m_outputPane.get());   // top: compiler output
    m_rightSplitter->addWidget(m_debugPane.get());    // bottom: debug log
    m_rightSplitter->setStretchFactor(0, 1); // equal stretch (adjust as needed)

    // Build the main horizontal splitter: editor | simulator | right panel
    m_mainSplitter->addWidget(m_editorPane.get());     // left
    //m_mainSplitter->addWidget(m_simulatorPane.get());  // center
    m_mainSplitter->addWidget(m_rightSplitter);        // right (vertical splitter)

    // Set stretch factors: simulator gets more space, editor and right panel get equal
    m_mainSplitter->setStretchFactor(0, 1); // editor
    m_mainSplitter->setStretchFactor(1, 2); // simulator (more space)
    m_mainSplitter->setStretchFactor(2, 1); // right panel

    mainLayout->addWidget(m_mainSplitter);

    setCentralWidget(centralWidget);
    setWindowTitle("Arduino Simulator Pro");

    resize(1300, 850);
}

//////////////////////////////////////////////////////////////
// TOOLBAR
//////////////////////////////////////////////////////////////

void MainWindow::setupToolBar()
{
    m_mainToolBar = addToolBar("Main Toolbar");
    m_mainToolBar->setMovable(false);

    m_buildAction = m_mainToolBar->addAction(
        style()->standardIcon(QStyle::SP_FileDialogDetailedView), "Build");

    m_runAction = m_mainToolBar->addAction(
        style()->standardIcon(QStyle::SP_MediaPlay), "Run");

    m_pauseAction = m_mainToolBar->addAction(
        style()->standardIcon(QStyle::SP_MediaPause), "Pause");

    m_stopAction = m_mainToolBar->addAction(
        style()->standardIcon(QStyle::SP_MediaStop), "Stop");

    m_mainToolBar->addSeparator();

    m_openAction = m_mainToolBar->addAction(
        style()->standardIcon(QStyle::SP_DirOpenIcon), "Open");

    m_saveAction = m_mainToolBar->addAction(
        style()->standardIcon(QStyle::SP_DialogSaveButton), "Save");

    m_clearAction = m_mainToolBar->addAction(
        style()->standardIcon(QStyle::SP_DialogResetButton), "Clear");

    m_mainToolBar->addSeparator();

    m_themeAction = m_mainToolBar->addAction(
        style()->standardIcon(QStyle::SP_DesktopIcon), "Switch Theme");

    m_runAction->setEnabled(false);
    m_pauseAction->setEnabled(false);
    m_stopAction->setEnabled(false);
}

//////////////////////////////////////////////////////////////
// CONNECTIONS
//////////////////////////////////////////////////////////////

void MainWindow::connectSignals()
{
    connect(m_buildAction, &QAction::triggered, this, &MainWindow::onBuildClicked);
    connect(m_runAction, &QAction::triggered, this, &MainWindow::onRunClicked);
    connect(m_pauseAction, &QAction::triggered, this, &MainWindow::onPauseClicked);
    connect(m_stopAction, &QAction::triggered, this, &MainWindow::onStopClicked);
    connect(m_openAction, &QAction::triggered, this, &MainWindow::onOpenClicked);
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::onSaveClicked);
    connect(m_clearAction, &QAction::triggered, this, &MainWindow::onClearClicked);
    connect(m_themeAction, &QAction::triggered, this, &MainWindow::onThemeToggled);

    // Connect logger to debug pane (compiler output may need separate connection)
    connect(&Logger::instance(), &Logger::messageLogged,
            m_debugPane.get(), &OutputPane::onLogMessage);

    // If you have a separate signal for compiler output, connect it to m_outputPane here
    // e.g., connect(&Logger::instance(), &Logger::compilerOutput, m_outputPane.get(), &OutputPane::onLogMessage);
}

//////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
//////////////////////////////////////////////////////////////

void MainWindow::onBuildClicked()
{
    statusBar()->showMessage("Building...");

    m_buildAction->setEnabled(false);
    m_runAction->setEnabled(false);

    m_outputPane->appendMessage("[INFO] Compiling sketch...", Logger::Info);

    // Simulate build process
    m_buildSucceeded = true;

    m_buildAction->setEnabled(true);
    m_runAction->setEnabled(true);

    statusBar()->showMessage("Build Succeeded");
}

void MainWindow::onRunClicked()
{
    if (!m_buildSucceeded)
        return;

    statusBar()->showMessage("Running simulation...");

    m_buildAction->setEnabled(false);
    m_runAction->setEnabled(false);
    m_pauseAction->setEnabled(true);
    m_stopAction->setEnabled(true);

    m_executionActive = true;
}

void MainWindow::onPauseClicked()
{
    if (m_executionActive) {
        statusBar()->showMessage("Paused");
        m_pauseAction->setText("Resume");
        m_executionActive = false;
    } else {
        statusBar()->showMessage("Running...");
        m_pauseAction->setText("Pause");
        m_executionActive = true;
    }
}

void MainWindow::onStopClicked()
{
    statusBar()->showMessage("Stopped");

    m_buildAction->setEnabled(true);
    m_runAction->setEnabled(m_buildSucceeded);
    m_pauseAction->setEnabled(false);
    m_pauseAction->setText("Pause");
    m_stopAction->setEnabled(false);

    m_executionActive = false;
}

void MainWindow::onOpenClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this, "Open Arduino Sketch", "",
        "Arduino Sketches (*.ino);;C++ Files (*.cpp);;All Files (*)");

    if (!fileName.isEmpty()) {
        if (m_editorPane->loadFromFile(fileName)) {
            setWindowTitle("Arduino Simulator Pro - " + fileName);
            statusBar()->showMessage("File Loaded");
        }
    }
}

void MainWindow::onSaveClicked()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, "Save Arduino Sketch", "sketch.ino",
        "Arduino Sketches (*.ino);;C++ Files (*.cpp);;All Files (*)");

    if (!fileName.isEmpty()) {
        if (m_editorPane->saveToFile(fileName)) {
            setWindowTitle("Arduino Simulator Pro - " + fileName);
            statusBar()->showMessage("File Saved");
        }
    }
}

void MainWindow::onClearClicked()
{
    m_outputPane->clearLogs();
    m_debugPane->clearLogs();
    statusBar()->showMessage("Logs Cleared");
}

void MainWindow::onThemeToggled()
{
    static bool dark = true;
    dark = !dark;

    m_editorPane->applyTheme(dark);
    m_outputPane->applyTheme();
    m_debugPane->applyTheme();
    //m_simulatorPane->applyTheme();

    statusBar()->showMessage(dark ? "Dark Mode Enabled"
                                  : "Light Mode Enabled");
}

void MainWindow::onThemeChanged()
{
    bool dark = (ThemeManager::instance().currentTheme()
                 == ThemeManager::Dark);

    m_editorPane->applyTheme(dark);
    m_outputPane->applyTheme();
    m_debugPane->applyTheme();
    //m_simulatorPane->applyTheme();
}
