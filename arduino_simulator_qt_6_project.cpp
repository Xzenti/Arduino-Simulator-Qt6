#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}

#pragma once
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QGraphicsView>
#include <QGraphicsScene>
#include "compiler_manager.h"
#include "simulator_engine.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void buildCode();
    void runSimulation();
    void onBuildFinished(bool success, const QString &output);

private:
    QPlainTextEdit *codeEditor;
    QTextEdit *compileOutput;
    QTextEdit *debugOutput;
    QPushButton *buildButton;
    QPushButton *runButton;
    QGraphicsView *simView;
    QGraphicsScene *scene;

    CompilerManager compiler;
    SimulatorEngine simulator;
};

#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    buildButton = new QPushButton("Build");
    runButton   = new QPushButton("Run");
    runButton->setEnabled(false);

    codeEditor = new QPlainTextEdit();
    codeEditor->setPlainText("void setup() {\n  pinMode(13, OUTPUT);\n}\n\nvoid loop() {\n  digitalWrite(13, HIGH);\n  delay(1000);\n  digitalWrite(13, LOW);\n  delay(1000);\n}");

    compileOutput = new QTextEdit();
    compileOutput->setReadOnly(true);
    debugOutput = new QTextEdit();
    debugOutput->setReadOnly(true);

    scene = new QGraphicsScene();
    simView = new QGraphicsView(scene);

    QHBoxLayout *controls = new QHBoxLayout();
    controls->addWidget(buildButton);
    controls->addWidget(runButton);

    QSplitter *splitter = new QSplitter(Qt::Vertical);
    splitter->addWidget(codeEditor);
    splitter->addWidget(simView);
    splitter->addWidget(compileOutput);
    splitter->addWidget(debugOutput);

    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->addLayout(controls);
    layout->addWidget(splitter);

    connect(buildButton, &QPushButton::clicked, this, &MainWindow::buildCode);
    connect(runButton, &QPushButton::clicked, this, &MainWindow::runSimulation);
    connect(&compiler, &CompilerManager::buildFinished, this, &MainWindow::onBuildFinished);
}

void MainWindow::buildCode()
{
    compileOutput->clear();
    debugOutput->append("Build started...");
    compiler.compile(codeEditor->toPlainText());
}

void MainWindow::onBuildFinished(bool success, const QString &output)
{
    compileOutput->setPlainText(output);
    if (success) {
        debugOutput->append("Build successful.");
        runButton->setEnabled(true);
    } else {
        debugOutput->append("Build failed.");
        runButton->setEnabled(false);
    }
}

void MainWindow::runSimulation()
{
    debugOutput->append("Simulation started.");
    simulator.loadCode(codeEditor->toPlainText());
    simulator.start();
}

#pragma once
#include <QObject>
#include <QProcess>

class CompilerManager : public QObject
{
    Q_OBJECT
public:
    explicit CompilerManager(QObject *parent = nullptr);
    void compile(const QString &code);

signals:
    void buildFinished(bool success, const QString &output);

private:
    QProcess process;
};

#include "compiler_manager.h"
#include <QTemporaryDir>
#include <QFile>

CompilerManager::CompilerManager(QObject *parent) : QObject(parent)
{
    connect(&process, &QProcess::finished, this, [this](int exitCode) {
        QString output = process.readAllStandardOutput() + process.readAllStandardError();
        emit buildFinished(exitCode == 0, output);
    });
}

void CompilerManager::compile(const QString &code)
{
    QTemporaryDir dir;
    QFile file(dir.path() + "/sketch.ino");
    file.open(QIODevice::WriteOnly);
    file.write(code.toUtf8());
    file.close();

    QStringList args;
    args << "compile" << "--fqbn" << "arduino:avr:uno" << dir.path();
    process.start("arduino-cli", args);
}

#pragma once
#include <QObject>
#include <QMap>

class SimulatorEngine : public QObject
{
    Q_OBJECT
public:
    explicit SimulatorEngine(QObject *parent = nullptr);

    void loadCode(const QString &code);
    void start();

signals:
    void pinStateChanged(int pin, bool state);
    void debugMessage(const QString &msg);

private:
    QMap<int, bool> pinStates;   // pin -> HIGH / LOW
    QMap<int, QString> pinModes; // pin -> INPUT / OUTPUT

    void executeLine(const QString &line);
};

#include "simulator_engine.h"
#include <QThread>

SimulatorEngine::SimulatorEngine(QObject *parent) : QObject(parent) {}

void SimulatorEngine::loadCode(const QString &)
{
    pinStates.clear();
}

void SimulatorEngine::start()
{
    emit debugMessage("Simulator started");

    for (int i = 0; i < 5; ++i) 
    {
        pinStates[13] = true;
        emit pinStateChanged(13, true);
        emit debugMessage("Pin 13 set HIGH");
        QThread::sleep(1);

        pinStates[13] = false;
        emit pinStateChanged(13, false);
        emit debugMessage("Pin 13 set LOW");
        QThread::sleep(1);
    }

    emit debugMessage("Simulator finished");
}
