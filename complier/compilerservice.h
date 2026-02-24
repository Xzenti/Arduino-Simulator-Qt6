#ifndef COMPILERSERVICE_H
#define COMPILERSERVICE_H
#include "CompilerWorker.h"
#include "BuildResult.h"

#include <QString>
#include <QObject>
#include <QThread>
#include <memory>

class CompilerService : public QObject {
    Q_OBJECT

public:
    CompilerService();
    ~CompilerService();

    BuildResult compile(const QString &sketchCode);
    void compileInBackground(const QString &sketchCode);

signals:
    void compilationStarted();
    void compileRequested(const QString &sketchCode);
    void compilationFinished(BuildResult result);
    void compilationProgress(int percentage);
    void compilationOutput(const QString &output);
    void compilationError(const QString &error);

private:
    std::unique_ptr<QThread> workerThread;
    CompilerWorker *worker;

private slots:
    void requestCompilation(const QString &sketchCode);
};

#endif // COMPILERSERVICE_H
