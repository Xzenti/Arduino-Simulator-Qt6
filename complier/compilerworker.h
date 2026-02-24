#ifndef COMPILERWORKER_H
#define COMPILERWORKER_H
#include "BuildResult.h"

#include <QString>
#include <QObject>
#include <QProcess>
#include <memory>

class CompilerWorker : public QObject {
    Q_OBJECT

public:
    explicit CompilerWorker(QObject *parent = nullptr);
    ~CompilerWorker();
    static BuildResult runArduinoCLICompile(const QString &sketchCode);

public slots:
    void compileAsync(const QString &sketchCode);

signals:
    void compilationStarted();
    void compilationFinished(BuildResult result);
    void compilationProgress(int percentage);

private:
    std::unique_ptr<QProcess> process;
    BuildResult performCompilation(const QString &sketchCode);
};

#endif // COMPILERWORKER_H
