#ifndef BUILDRESULT_H
#define BUILDRESULT_H

#include <QString>

struct BuildResult {
    bool success = false;
    QString output;
    QString errorOutput;
    int exitCode = -1;
};

#endif // BUILDRESULT_H
