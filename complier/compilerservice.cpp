#include "CompilerService.h"

CompilerService::CompilerService()
    : worker(new CompilerWorker()) {
    workerThread = std::make_unique<QThread>();
    worker->moveToThread(workerThread.get());

    connect(this, &CompilerService::compileRequested, worker, &CompilerWorker::compileAsync, Qt::QueuedConnection);
    connect(worker, &CompilerWorker::compilationStarted, this, &CompilerService::compilationStarted);
    connect(worker, &CompilerWorker::compilationFinished, this, &CompilerService::compilationFinished);
    connect(worker, &CompilerWorker::compilationProgress, this, &CompilerService::compilationProgress);
    workerThread->start();
}

CompilerService::~CompilerService() {
    if (workerThread && workerThread->isRunning()) {
        workerThread->quit();
        workerThread->wait(5000);
    }
    delete worker;
    worker = nullptr;
}

BuildResult CompilerService::compile(const QString &sketchCode) {
    //BuildResult result;

    // Stub implementation for now
    /*esult.success = true;
    result.output = "Compilation successful (stub)";
    result.exitCode = 0;
    */
    return CompilerWorker::runArduinoCLICompile(sketchCode);
}

void CompilerService::compileInBackground(const QString &sketchCode) {
    emit compilationStarted();
    // Stub implementation for now
}

void CompilerService::requestCompilation(const QString &sketchCode) {
    // Stub implementation for now
    emit compilationOutput("Requesting compilation...");
    emit compileRequested(sketchCode);

}
