#include "mainwindow.h"

#include <QPushButton>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Central widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // ----------- Buttons -----------
    buildButton = new QPushButton("Build");
    runButton   = new QPushButton("Run");
    runButton->setEnabled(false); // disabled until build succeeds

    // ----------- Code Editor -----------
    codeEditor = new QPlainTextEdit();
    codeEditor->setPlainText(
        "void setup() {\n"
        "  pinMode(13, OUTPUT);\n"
        "}\n\n"
        "void loop() {\n"
        "  digitalWrite(13, HIGH);\n"
        "  delay(1000);\n"
        "  digitalWrite(13, LOW);\n"
        "  delay(1000);\n"
        "}"
        );

    // ----------- Output Panels -----------
    debugOutput = new QTextEdit();
    compileOutput = new QTextEdit();

    debugOutput->setReadOnly(true);
    compileOutput->setReadOnly(true);

    debugOutput->setPlaceholderText("Debug messages...");
    compileOutput->setPlaceholderText("Compilation output...");

    // ----------- Layouts -----------

    // Top bar (Build / Run)
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(buildButton);
    buttonLayout->addWidget(runButton);

    // Main vertical layout
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(codeEditor);
    mainLayout->addWidget(debugOutput);
    mainLayout->addWidget(compileOutput);

    // ----------- Signal & Slot -----------

    connect(buildButton, &QPushButton::clicked,
            this, &MainWindow::onBuildClicked);

    connect(runButton, &QPushButton::clicked,
            this, &MainWindow::onRunClicked);
}
