#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QAction>
#include <QSplitter>
#include <memory>
#include "../compiler/BuildResult.h"

class EditorPane;
class OutputPane;
class SketchInterpreter;
class CompilerService;
class SimulatorPane;
class Logger;
class BoardModel;
class SketchInterpreter;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() ;
signals:
    void syncHorizontalSplitters();
private slots:
    void onBuildClicked();
    void onRunClicked();
    void onPauseClicked();
    void onStopClicked();
    void onOpenClicked();
    void onSaveClicked();
    void onPinStateChanged(int pin, int value);
    void onClearClicked();
    void onThemeToggled();
    void onThemeChanged();
    void onCompilationDone(BuildResult result);

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


    std::unique_ptr<BoardModel> m_boardModel;
    std::unique_ptr<CompilerService> m_compilerService;
    std::unique_ptr<SketchInterpreter> m_interpreter;
    QSet<int> m_warnedPins;

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
    QSplitter* m_mainSplitter   = nullptr;
    QSplitter* m_rightSplitter  = nullptr;

    // State flags
    bool m_buildSucceeded  = false;
    bool m_executionActive = false;
    bool m_syncingSplitters = false;
};

#endif // MAINWINDOW_H
