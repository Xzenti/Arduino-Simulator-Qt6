#include "CompilerWorker.h"
#include "BuildResult.h"

#include <QThread>
#include <QDebug>
#include <QProcess>
#include <QDir>
#include <QTemporaryDir>
#include <QFile>
#include <QStandardPaths>
namespace {
const char *FQBN = "arduino:avr:uno";
const int COMPILE_TIMEOUT_MS = 60000;
const char *SKETCH_DIR_NAME = "sketch";
const char *SKETCH_INO_NAME = "sketch.ino";

QString findArduinoCLIPath() {
    const QString home = QDir::homePath();
    QStringList candidates;
#if defined(Q_OS_WIN)
    candidates << QDir::rootPath() + QStringLiteral("Program Files/Arduino CLI/arduino-cli.exe")
               << QDir::rootPath() + QStringLiteral("Program Files (x86)/Arduino CLI/arduino-cli.exe");
    const QString localAppData = qEnvironmentVariable("LOCALAPPDATA");
    if (!localAppData.isEmpty())
        candidates << localAppData + QStringLiteral("/Programs/Arduino CLI/arduino-cli.exe");
    if (!home.isEmpty()) {
        candidates << home + QStringLiteral("/arduino-cli.exe")
        << home + QStringLiteral("/bin/arduino-cli.exe");
    }
    candidates << QStandardPaths::findExecutable(QStringLiteral("arduino-cli"));
#else
    candidates << QStringLiteral("/usr/local/bin/arduino-cli")
               << QStringLiteral("/opt/homebrew/bin/arduino-cli")
               << QStringLiteral("/usr/bin/arduino-cli");
    if (!home.isEmpty()) {
        candidates << home + QStringLiteral("/bin/arduino-cli")
        << home + QStringLiteral("/.local/bin/arduino-cli")
        << home + QStringLiteral("/arduino-cli");
    }
    candidates << QStandardPaths::findExecutable(QStringLiteral("arduino-cli"));
#endif
    for (const QString &path : candidates) {
        if (path.isEmpty())
            continue;
        if (QFile::exists(path)) {
            // Optional: verify it's actually the CLI by running --version (short timeout).
            QProcess probe;
            probe.setProgram(path);
            probe.setArguments({QStringLiteral("--version")});
            probe.setProcessChannelMode(QProcess::MergedChannels);
            probe.start();
            if (probe.waitForFinished(3000) && probe.exitStatus() == QProcess::NormalExit && probe.exitCode() == 0)
                return path;
            if (!probe.waitForFinished(0))
                probe.kill();
            return path; // Exists; use it even if --version failed (e.g. missing deps).
        }
    }
    return QString();
}
}
CompilerWorker::CompilerWorker(QObject *parent)
    : QObject(parent),
    process(std::make_unique<QProcess>()) {
}

CompilerWorker::~CompilerWorker() = default;

void CompilerWorker::compileAsync(const QString &sketchCode) {
    emit compilationStarted();
    qDebug() << "[Worker Thread] Starting async compilation...";

    // Simulate compilation progress (VS Code style)
    emit compilationProgress(25);
    QThread::msleep(100);

    emit compilationProgress(50);
    BuildResult result = performCompilation(sketchCode);

    emit compilationProgress(75);
    QThread::msleep(50);

    emit compilationProgress(100);
    qDebug() << "[Worker Thread] Compilation complete!";
    emit compilationFinished(result);
}

BuildResult CompilerWorker::runArduinoCLICompile(const QString &sketchCode) {
    BuildResult result;
    result.success = false;
    result.exitCode = -1;

    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        result.output = "Error: could not create temporary directory for sketch.";
        return result;
    }

    QDir buildRoot(tempDir.path());
    if (!buildRoot.mkdir(SKETCH_DIR_NAME)) {
        result.output = "Error: could not create sketch directory.";
        return result;
    }

    QString sketchPath = buildRoot.absoluteFilePath(SKETCH_DIR_NAME);
    QString inoPath = QDir(sketchPath).absoluteFilePath(SKETCH_INO_NAME);

    QFile inoFile(inoPath);
    if (!inoFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        result.output = "Error: could not write sketch file: " + inoPath;
        return result;
    }
    if (inoFile.write(sketchCode.toUtf8()) < 0) {
        result.output = "Error: could not write sketch content.";
        return result;
    }
    inoFile.close();

    const QString cliPath = findArduinoCLIPath();
    if (cliPath.isEmpty()) {
        result.output = "Arduino CLI not found. Install it and ensure it is in one of: /usr/local/bin,  or in PATH.";

#if defined(Q_OS_WIN)
        result.output = "Arduino CLI not found. Install it (e.g. from Arduino CLI installer) or add it to PATH.";
#endif
        return result;
    }

    QProcess proc;
    proc.setProgram(cliPath);
    proc.setArguments({QStringLiteral("compile"), QStringLiteral("--fqbn"), QString::fromUtf8(FQBN), sketchPath});
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.start();
    if (!proc.waitForFinished(COMPILE_TIMEOUT_MS)) {
        result.output = "Compilation timed out after " + QString::number(COMPILE_TIMEOUT_MS / 1000) + " seconds.";
        if (proc.state() != QProcess::NotRunning)
            proc.kill();
        return result;
    }

    result.exitCode = proc.exitCode();
    result.output = QString::fromUtf8(proc.readAllStandardOutput());
    if (result.output.isEmpty())
        result.output = QString::fromUtf8(proc.readAllStandardError());
    result.success = (proc.exitStatus() == QProcess::NormalExit && result.exitCode == 0);

    return result;
}

BuildResult CompilerWorker::performCompilation(const QString &sketchCode) {
    // BuildResult result;

    // // Stub implementation for now - runs in background thread
    // result.success = true;
    // result.output = "Compilation successful (async worker thread)";
    // result.exitCode = 0;

    // return result;
    return runArduinoCLICompile(sketchCode);

}
