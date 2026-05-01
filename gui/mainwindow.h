#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QPushButton;
class QPlainTextEdit;
class QTextEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onBuildClicked();
    void onRunClicked();

private:
    // Control buttons
    QPushButton *buildButton;
    QPushButton *runButton;

    // Text editor for Arduino code
    QPlainTextEdit *codeEditor;

    // Output panels
    QTextEdit *debugOutput;
    QTextEdit *compileOutput;
};

#endif // MAINWINDOW_H
