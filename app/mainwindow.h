#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QAction>
#include <QSplitter>
#include <memory>

class EditorPane;
class OutputPane;
class SimulatorPane;
class Logger;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onBuildClicked();
    void onRunClicked();
    void onPauseClicked();
    void onStopClicked();
    void onOpenClicked();
    void onSaveClicked();
    void onClearClicked();
    void onThemeToggled();
    void onThemeChanged();

private:
    // UI Setup
    void setupUI();
    void setupToolBar();
    void connectSignals();

    // Core Panes
    std::unique_ptr<EditorPane>    m_editorPane;
    std::unique_ptr<SimulatorPane> m_simulatorPane;
    std::unique_ptr<OutputPane>    m_outputPane;      // Compiler Output
    std::unique_ptr<OutputPane>    m_debugPane;       // Debug Log

    // Toolbar
    QToolBar* m_mainToolBar = nullptr;
    QAction* m_buildAction  = nullptr;
    QAction* m_runAction    = nullptr;
    QAction* m_pauseAction  = nullptr;
    QAction* m_stopAction   = nullptr;
    QAction* m_openAction   = nullptr;
    QAction* m_saveAction   = nullptr;
    QAction* m_clearAction  = nullptr;
    QAction* m_themeAction  = nullptr;

    // Splitters
    QSplitter* m_mainSplitter   = nullptr;   // horizontal: left, center, right
    QSplitter* m_rightSplitter  = nullptr;   // vertical: compiler output, debug log

    // State flags
    bool m_buildSucceeded  = false;
    bool m_executionActive = false;
};

#endif // MAINWINDOW_H
