#include "ArduinoBoardItem.h"
#include <QPainter>
#include <QPixmap>
#include <QCoreApplication>
#include <QFileInfo>

ArduinoBoardItem::ArduinoBoardItem(QGraphicsItem *parent)
    : QGraphicsItem(parent) {
    setZValue(-1);  // Board behind wires and components so connections are always visible on top
    boardPixmap = QPixmap(":/images/arduino_uno.png");
    if (boardPixmap.isNull()) {
        QString appDir = QCoreApplication::applicationDirPath();
        QStringList fallbackPaths;
        fallbackPaths << (appDir + "/../resources/images/arduino_uno.png");
        fallbackPaths << (appDir + "/../../resources/images/arduino_uno.png");
        fallbackPaths << (appDir + "/../../../resources/images/arduino_uno.png");
        fallbackPaths << (appDir + "/../../../../resources/images/arduino_uno.png");
        fallbackPaths << (appDir + "/../../../../../resources/images/arduino_uno.png");
        fallbackPaths << (appDir + "/resources/images/arduino_uno.png");
        fallbackPaths << "resources/images/arduino_uno.png";
        fallbackPaths << "../resources/images/arduino_uno.png";
        fallbackPaths << "../../resources/images/arduino_uno.png";
        for (const QString &path : fallbackPaths) {
            QString absPath = QFileInfo(path).absoluteFilePath();
            if (QFileInfo(absPath).exists() && boardPixmap.load(absPath))
                break;
        }
    }
    if (boardPixmap.isNull()) {
        boardPixmap = QPixmap(300, 200);
        boardPixmap.fill(Qt::lightGray);
    }
    // Always 560×420 so hardcoded pin positions (PinPositions.h) match exactly
    boardPixmap = boardPixmap.scaled(MaxDisplayWidth, MaxDisplayHeight,
                                     Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

ArduinoBoardItem::~ArduinoBoardItem() = default;

QRectF ArduinoBoardItem::boundingRect() const {
    return QRectF(0, 0, MaxDisplayWidth, MaxDisplayHeight);
}

void ArduinoBoardItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->drawPixmap(0, 0, boardPixmap);
}
