#include "ResistorItem.h"
#include "PinPositions.h"
#include <QPainter>
#include <QCoreApplication>
#include <QFileInfo>

ResistorItem::ResistorItem(QGraphicsItem *parent)
    : QGraphicsItem(parent) {
    loadPixmap();
    setFlag(ItemIsSelectable);
    setFlag(ItemIsMovable);
}

ResistorItem::~ResistorItem() = default;

void ResistorItem::loadPixmap() {
    pixmap = QPixmap(":/images/resistor.png");
    if (pixmap.isNull()) {
        QString appDir = QCoreApplication::applicationDirPath();
        QStringList paths;
        paths << (appDir + "/../resources/images/resistor.png");
        paths << (appDir + "/../../resources/images/resistor.png");
        paths << "resources/images/resistor.png";
        for (const QString &path : paths) {
            QString absPath = QFileInfo(path).absoluteFilePath();
            if (QFileInfo(absPath).exists() && pixmap.load(absPath))
                break;
        }
    }
    if (pixmap.isNull()) {
        pixmap = QPixmap(MaxWidth, MaxHeight);
        pixmap.fill(Qt::darkYellow);
    }
    pixmap = pixmap.scaled(PinPositions::RESISTOR_WIDTH, PinPositions::RESISTOR_HEIGHT,
                           Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

QRectF ResistorItem::boundingRect() const {
    return QRectF(0, 0, PinPositions::RESISTOR_WIDTH, PinPositions::RESISTOR_HEIGHT);
}

QPointF ResistorItem::leg1Pos() const {
    return QPointF(PinPositions::RESISTOR_LEG1_X, PinPositions::RESISTOR_LEG1_Y);
}

QPointF ResistorItem::leg2Pos() const {
    return QPointF(PinPositions::RESISTOR_LEG2_X, PinPositions::RESISTOR_LEG2_Y);
}

void ResistorItem::setLeg1PinId(const QString &id) {
    if (leg1PinId_ != id) {
        leg1PinId_ = id;
        update();
    }
}

void ResistorItem::setLeg2PinId(const QString &id) {
    if (leg2PinId_ != id) {
        leg2PinId_ = id;
        update();
    }
}

void ResistorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->drawPixmap(0, 0, pixmap);
    if (isSelected()) {
        painter->setPen(QPen(Qt::blue, 2, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
    }
}
